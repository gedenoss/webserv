#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

#include "../includes/server.hpp"
#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/utils.hpp"

// permet de choisir le bon server sur le quel se connecter 
size_t whichServerToChoose(std::vector<ServerConfig>& servers, const std::string& need) {
    for (size_t i = 0; i < servers.size(); ++i) {
        if (static_cast<size_t> (servers[i].getPort()) == stringToSizeT(need))
            return i;
    }
    return 0;
}

bool Server::_serverIsRunning = true;

//permet de gerer le ctrl C
void Server::_sigIntCatcher(int signal) {
    std::cout << RED << BOLD << "\nSIGINT received. Shutting down server..." << RESET << std::endl;
    _serverIsRunning = false;
    (void)signal;
}

// constructeur par defaut 
Server::Server(const Config &conf)
    : _epoll_fd(-1), config(conf), _addr()
{
    setUp();
}

// destructeur et close les fd necessaire 
Server::~Server(){
    for (std::vector<int>::size_type i = 0; i < _server_fds.size(); ++i)
        close(_server_fds[i]);
    close(_epoll_fd);
}

//fonction si quelque chose echoue dans la boucle qui handle le client
void Server::clean(){
    close(_fd);
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _fd, NULL);
    _buffers.erase(_fd);
    _contentLengths.erase(_fd);
    _headersDone.erase(_fd);
}

// creation des socket pour chaque server + ajout a la strucure epoll
void Server::setUp(){
    
    _servers = config.getServers();

    // creation du descripteur epoll
    _epoll_fd = epoll_create(_servers.size());
    if (_epoll_fd < 0) {
        perror("ERROR: epoll_create failed");
        exit(EXIT_FAILURE);
    }

    // creation des socket pour chaque serveur 
    for (std::vector<ServerConfig>::size_type i = 0; i < _servers.size(); ++i) {
        ServerConfig server = _servers[i];
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("ERROR: socket failed");
            exit(EXIT_FAILURE);
        }
        // permet la reutilisation de l adresse en cas de redemarage rapide
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("ERROR: setsockopt failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // configuration de l adresse ip et port du socket seveur
        std::memset(&_addr, 0, sizeof(_addr));
        _addr.sin_family = AF_INET;
        _addr.sin_port   = htons(server.getPort());
        if (inet_pton(AF_INET, "127.0.0.1", &_addr.sin_addr) <= 0) {
            perror("ERROR: inet_pton failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // bind permet la liaison entre la socket et l adresse
        if (bind(server_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
            perror("ERROR: bind failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // ecoute sur le port
        if (listen(server_fd, 10) < 0) {
            perror("ERROR: listen failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // configuration du socket non bloquant
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

        _server_fds.push_back(server_fd);

        //enregistrement du socket server dans epoll (epoll gestionaire de fds)
        struct epoll_event ev;
        ev.events   = EPOLLIN;
        ev.data.fd  = server_fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
            perror("ERROR: epoll_ctl ADD server_fd failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
    }
}

// fonction appelee lorsque une nouvelle connexion client est detectee
void Server::handleNewConnection(){
    socklen_t client_len = sizeof(_client_addr);
    int client_fd = accept(_fd, (struct sockaddr*)&_client_addr, &client_len);
    if (client_fd >= 0) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &_client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        // uint16_t client_port = ntohs(_client_addr.sin_port);
        // std::cout << "New connection accepted from client: " 
        // << client_ip << ":" << client_port << std::endl;
        // rend le socket non bloquant
        int fl = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, fl | O_NONBLOCK);
        //ajout le socket client a epoll
        struct epoll_event ev;
        ev.events   = EPOLLIN;
        ev.data.fd  = client_fd;
        epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
    } else {
        perror("ERROR: accept failed");
    }
}

//recupere la valeur brut pour recupere le port
std::string getHostRawRequest(std::string rq)
{
    size_t hostPos = rq.find("Host:") + 6;
    std::string hostname = rq.substr(hostPos);
    size_t colonPos = hostname.find(':');
    if (colonPos != std::string::npos) {
        hostname = hostname.substr(colonPos + 1);
    }
    return hostname;
}

// fonction qui lit les donnees envoyee par un client connecte
void Server::handleClientData(){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    getpeername(_fd, (struct sockaddr*)&client_addr, &client_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
   // lecture des donnees dans un buffer tmp
    char buf[4096];
    while (1) {
        int r = recv(_fd, buf, sizeof(buf)-1, 0);
        if (r <= 0) break;  // 0=fermeture, -1=EAGAIN/EWOULDBLOCK
        _buffers[_fd].append(buf, r);

        // Affichage des informations du client avec la requête
        // std::cout << "Received data from client " 
        //           << client_ip << ":" << client_port << std::endl;
        //std::cout << "Data: " << _buffers[fd] << std::endl;
        // protection contre les requetes trop grande
        if (_buffers[_fd].size() > 4069 * 1000 * 100) {
            std::cerr << "ERROR: Request too large\n";
            send(_fd, "HTTP/1.1 413 Payload Too Large\r\n\r\n", 35, 0);
            clean();
        }
    }

    // si les headers ne sont pas encore entierement recus
    if (! _headersDone.count(_fd)) {
        std::string &rq = _buffers[_fd];
        std::string::size_type end_hdr = rq.find("\r\n\r\n");
        if (end_hdr != std::string::npos) {
            _headersDone.insert(_fd);
            std::string::size_type pos = rq.find("Content-Length:");
            if (pos != std::string::npos) {
                std::string::size_type line_end = rq.find("\r\n", pos);
                std::string cl = rq.substr(pos + 15, line_end - (pos+15));
                _contentLengths[_fd] = std::atoi(cl.c_str());
                } else {
                    _contentLengths[_fd] = 0;
                }
        }
    }
    // si les headers sont cpmplet on verifie si tout le corps est recue 
    if (_headersDone.count(_fd)) {
        std::string &rq = _buffers[_fd];
        std::string::size_type end_hdr = rq.find("\r\n\r\n");
        int expected = _contentLengths[_fd];
        std::size_t have_body = rq.size() - (end_hdr + 4);
        if ((int)have_body < expected)
            return;
        {
            // si on a tout recu on traite la requete
        std::string need = getHostRawRequest(rq);
        size_t i = whichServerToChoose(_servers, need);
        size_t maxBodySize = _servers[i].getClientMaxBodySize();          
        Request  request(maxBodySize,1024);
        request.parse(rq, config);
        request.printRequest();
        Response response(request, _servers[i]);
        std::string reply = response.sendResponse();
        send(_fd, reply.c_str(), reply.size(), 0);
        }
    }
    //nettoyage du client
    clean();
}

void Server::launchServer() {

    const int MAX_EVENTS = 100;
    struct epoll_event events[MAX_EVENTS];
    // ignorer les signaux de fermeture de socket
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, _sigIntCatcher);
    while (_serverIsRunning) {
        int n = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            if (!_serverIsRunning)
                break; // Serveur stoppé par SIGINT, on sort sans afficher d'erreur
            perror("ERROR: epoll_wait failed");
            break;
        }
        //boucle sur tout les evenement recus
        for (int i = 0; i < n; ++i) {
            _fd = events[i].data.fd;
            //si le fd correspond a un socket sever alors c est une nouvelle connexion
            if (std::find(_server_fds.begin(), _server_fds.end(), _fd)
                != _server_fds.end())
                handleNewConnection();
            else
                handleClientData(); // sinon donnees client a traiter
        }
    }
    return ;
}
