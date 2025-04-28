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

int launchServer(Config config) {
    std::vector<ServerConfig> servers = config.getServers();
    std::vector<int> server_fds;

    int epoll_fd = epoll_create(servers.size());
    if (epoll_fd < 0) {
        perror("ERROR: epoll_create failed");
        exit(EXIT_FAILURE);
    }

    // ————————————————
    // 1) Mise en place des serveurs
    // ————————————————
    for (std::vector<ServerConfig>::size_type i = 0; i < servers.size(); ++i) {
        ServerConfig server = servers[i];
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

        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(server.getPort());
        if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
            perror("ERROR: inet_pton failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
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

        server_fds.push_back(server_fd);

        struct epoll_event ev;
        ev.events   = EPOLLIN;
        ev.data.fd  = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
            perror("ERROR: epoll_ctl ADD server_fd failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
    }

    // —————————————————————
    // 2) Structures d’état par client
    // —————————————————————
    std::map<int, std::string>        buffers;
    std::map<int, int>                contentLengths;
    std::set<int>                     headersDone;

    const int MAX_EVENTS = 100;
    struct epoll_event events[MAX_EVENTS];

    // ——————————————————————————
    // 3) Boucle principale epoll
    // ——————————————————————————
    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("ERROR: epoll_wait failed");
            break;
        }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            // —————————————————————————————
            // a) Nouvelle connexion entrante
            // —————————————————————————————
            if (std::find(server_fds.begin(), server_fds.end(), fd)
                != server_fds.end())
            {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(fd,
                                       (struct sockaddr*)&client_addr,
                                       &client_len);
                if (client_fd >= 0) {
                    std::cout << "New connection accepted\n";
                    int fl = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, fl | O_NONBLOCK);
                    struct epoll_event ev;
                    ev.events   = EPOLLIN;
                    ev.data.fd  = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                } else {
                    perror("ERROR: accept failed");
                }
            }
            // ——————————————————————————————
            // b) Données prêtes sur socket client
            // ——————————————————————————————
            else {
                // 1) Lire tout ce qui est dispo sans fermer
                char buf[4096];
                while (1) {
                    int r = recv(fd, buf, sizeof(buf)-1, 0);
                    if (r <= 0) break;  // 0=fermeture, -1=EAGAIN/EWOULDBLOCK
                    buffers[fd].append(buf, r);
                    // protection anti-DOS
                    if (buffers[fd].size() > 4069 * 1000 * 100) {
                        std::cerr << "ERROR: Request too large\n";
                        send(fd,
                             "HTTP/1.1 413 Payloaddd To Large\r\n\r\n",
                             35, 0);
                        goto cleanup_fd;
                    }
                }

                // 2) Vérifier si on a déjà parsé les headers
                if (! headersDone.count(fd)) {
                    std::string &rq = buffers[fd];
                    std::string::size_type end_hdr = rq.find("\r\n\r\n");
                    if (end_hdr != std::string::npos) {
                        headersDone.insert(fd);
                        // extraire Content-Length s’il existe
                        std::string::size_type pos = rq.find("Content-Length:");
                        if (pos != std::string::npos) {
                            std::string::size_type line_end = rq.find("\r\n", pos);
                            std::string cl = rq.substr(pos + 15,
                                                       line_end - (pos+15));
                            contentLengths[fd] = std::atoi(cl.c_str());
                        } else {
                            contentLengths[fd] = 0;
                        }
                    }
                }

                // 3) Si headers parsés, vérifier si le body est complet
                if (headersDone.count(fd)) {
                    std::string &rq = buffers[fd];
                    std::string::size_type end_hdr = rq.find("\r\n\r\n");
                    int expected = contentLengths[fd];
                    std::size_t have_body =
                        rq.size() - (end_hdr + 4);

                    // tant que non complet, on attend la prochaine EPOLLIN
                    if ((int)have_body < expected)
                        continue;

                    // body complet ou pas attendu → on traite
                    {
                        // std::cout << "Full request received "
                        //           << rq << " \n";
                        size_t maxBodySize = servers[0].getClientMaxBodySize();          
                        Request  request(maxBodySize,1024);    //define by the client_max_body_size in the .conf
                        request.parse(rq, config);
                        Response response(request, servers[0]);
                        std::string reply = response.sendResponse();
                        send(fd, reply.c_str(), reply.size(), 0);
                    }
                }

            cleanup_fd:
                // on ferme et on nettoie l’état
                close(fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                buffers.erase(fd);
                contentLengths.erase(fd);
                headersDone.erase(fd);
            }
        }
    }

    // ——————————————————————
    // 4) Cleanup final
    // ——————————————————————
    for (std::vector<int>::size_type i = 0; i < server_fds.size(); ++i)
        close(server_fds[i]);
    close(epoll_fd);
    return 0;
}
