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

Server::Server(const Config &conf)
    : _epoll_fd(-1), config(conf), _addr()
{
    setUp();
}

Server::~Server(){
    for (std::vector<int>::size_type i = 0; i < _server_fds.size(); ++i)
        close(_server_fds[i]);
    close(_epoll_fd);
}

void Server::clean(){
    close(_fd);
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _fd, NULL);
    _buffers.erase(_fd);
    _contentLengths.erase(_fd);
    _headersDone.erase(_fd);
}

void Server::setUp(){
    
    _servers = config.getServers();

    _epoll_fd = epoll_create(_servers.size());
    if (_epoll_fd < 0) {
        perror("ERROR: epoll_create failed");
        exit(EXIT_FAILURE);
    }

    for (std::vector<ServerConfig>::size_type i = 0; i < _servers.size(); ++i) {
        ServerConfig server = _servers[i];
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("ERROR: socket failed");
            exit(EXIT_FAILURE);
        }
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("ERROR: setsockopt failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        std::memset(&_addr, 0, sizeof(_addr));
        _addr.sin_family = AF_INET;
        _addr.sin_port   = htons(server.getPort());
        if (inet_pton(AF_INET, "127.0.0.1", &_addr.sin_addr) <= 0) {
            perror("ERROR: inet_pton failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        if (bind(server_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
            perror("ERROR: bind failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 10) < 0) {
            perror("ERROR: listen failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

        _server_fds.push_back(server_fd);

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

void Server::handleNewConnection(){
    socklen_t client_len = sizeof(_client_addr);
    int client_fd = accept(_fd, (struct sockaddr*)&_client_addr, &client_len);
    if (client_fd >= 0) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &_client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        uint16_t client_port = ntohs(_client_addr.sin_port);
        std::cout << "New connection accepted from client: " 
        << client_ip << ":" << client_port << std::endl;
        int fl = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, fl | O_NONBLOCK);
        struct epoll_event ev;
        ev.events   = EPOLLIN;
        ev.data.fd  = client_fd;
        epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
    } else {
        perror("ERROR: accept failed");
    }
}

void Server::handleClientData(){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    getpeername(_fd, (struct sockaddr*)&client_addr, &client_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    char buf[4096];
    while (1) {
        int r = recv(_fd, buf, sizeof(buf)-1, 0);
        if (r <= 0) break;  // 0=fermeture, -1=EAGAIN/EWOULDBLOCK
        _buffers[_fd].append(buf, r);

        // Affichage des informations du client avec la requête
        // std::cout << "Received data from client " 
        //           << client_ip << ":" << client_port << std::endl;
        //std::cout << "Data: " << _buffers[fd] << std::endl;

        if (_buffers[_fd].size() > 4069 * 1000 * 100) {
            std::cerr << "ERROR: Request too large\n";
            send(_fd, "HTTP/1.1 413 Payload Too Large\r\n\r\n", 35, 0);
            clean();
        }
    }
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
    if (_headersDone.count(_fd)) {
        std::string &rq = _buffers[_fd];
        std::string::size_type end_hdr = rq.find("\r\n\r\n");
        int expected = _contentLengths[_fd];
        std::size_t have_body = rq.size() - (end_hdr + 4);
        if ((int)have_body < expected)
            return;
        {
        std::cout << "Full request received " << rq << " \n";
        Request  request(4096 * 4096, 1024);
        request.parse(rq, config);
        Response response(request, _servers[0]);
        std::string reply = response.sendResponse();
        send(_fd, reply.c_str(), reply.size(), 0);
        }
    }
    clean();
}

void Server::launchServer() {

    const int MAX_EVENTS = 100;
    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int n = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("ERROR: epoll_wait failed");
            break;
        }
        for (int i = 0; i < n; ++i) {
            _fd = events[i].data.fd;
            if (std::find(_server_fds.begin(), _server_fds.end(), _fd)
                != _server_fds.end())
                handleNewConnection();
            else
                handleClientData();
        }
    }
    return ;
}
