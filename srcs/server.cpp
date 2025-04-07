// #include "../includes/server.hpp"
// #include "../includes/request.hpp"
// #include "../includes/response.hpp"
// #include "../includes/utils.hpp"

// std::string intToString(int num) {
//     std::stringstream ss;
//     ss << num;
//     return ss.str();
// }

// std::string getFileExtension(const std::string &path) {
//     size_t dotPos = path.find_last_of(".");
//     if (dotPos != std::string::npos)
//         return path.substr(dotPos);
//     return "";
// }

// std::string getContentType(const std::string &ext) {
//     if (ext == ".html") return "text/html";
//     if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
//     if (ext == ".png") return "image/png";
//     if (ext == ".css") return "text/css";
//     if (ext == ".js") return "application/javascript";
//     return "application/octet-stream";
// }

// int launchServer(Config config) {
    
//     std::vector <ServerConfig> servers = config.getServers();
//     ServerConfig server = servers[0];
//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_fd == -1) {
//         perror("ERROR: Error while creating the server");
//         exit(EXIT_FAILURE);
//     }
    
//     int opt = 1;
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//         perror("ERROR: Error while activating SO_REUSEADDR");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
    
//     struct sockaddr_in server_address;
//     memset(&server_address, 0, sizeof(server_address));
//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(server.getPort());
//     if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
//         perror("ERROR: IP conversion");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
    
//     if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
//         perror("ERROR: Error while binding the socket");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
    
//     if (listen(server_fd, 10) < 0) {
//         perror("ERROR: Error while listening");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
    
//     std::vector<pollfd> fds;
//     pollfd server_pollfd;
//     server_pollfd.fd = server_fd;
//     server_pollfd.events = POLLIN;
//     server_pollfd.revents = 0;
//     fds.push_back(server_pollfd);
    
//     while (true) {
//         int pol_ret = poll(fds.data(), fds.size(), -1);
//         if (pol_ret < 0) {
//             perror("ERROR: poll Error");
//             break;
//         }
        
//         for (size_t i = 0; i < fds.size(); ++i) {
//             if (fds[i].revents & POLLIN) {
//                 if (fds[i].fd == server_fd) {
//                     sockaddr_in client_addr;
//                     socklen_t client_len = sizeof(client_addr);
//                     int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
//                     if (client_fd >= 0) {
//                         std::cout << "New connection accepted" << std::endl;
//                         pollfd client_pollfd;
//                         client_pollfd.fd = client_fd;
//                         client_pollfd.events = POLLIN;
//                         client_pollfd.revents = 0;
//                         fds.push_back(client_pollfd);
//                     } else {
//                         perror("ERROR: accept failed");
//                     }
//                 } else {
//                     char buffer[4096];
//                     int bytes_read = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
//                     if (bytes_read > 4096) {
//                         std::cerr << "ERROR: Request too large! Closing connection." << std::endl;
//                         send(fds[i].fd, "HTTP/1.1 413 Payload Too Large\r\n\r\n", 35, 0);
//                         close(fds[i].fd);
//                         fds.erase(fds.begin() + i);
//                         --i;
//                         continue;
//                     }
//                     if (bytes_read > 0) {
//                         buffer[bytes_read] = '\0';
//                         std::string rawRequest(buffer);
//                         // std::cout << rawRequest << std::endl;
//                         Request request(1024,1024);
//                         // request.addHeader("Range", "bytes=1-10");
//                         request.parse(rawRequest, config);
//                         request.printRequest();
//                         Response response(request, server);
//                         std::string sendResponse = response.sendResponse();
                        
//                         std::string fileContent = readFile("index.html");
//                         std::string fileExt = getFileExtension("index.html");
//                         std::string contentType = getContentType(fileExt);
//                         if (!fileContent.empty()) {
//                             send(fds[i].fd, sendResponse.c_str(), sendResponse.size(), 0);
//                             if (response.getStatusCode() >= 400) { 
//                                 std::string errorPage = "./web/errors/" + intToString(response.getStatusCode()) + ".html";
//                                 std::string fileContent = readFile(errorPage);
//                                 if (!fileContent.empty()) {
//                                     send(fds[i].fd, fileContent.c_str(), fileContent.size(), 0);
//                                 }
//                             }
//                         }
//                     } else {
//                         std::cout << "Disconnected client" << std::endl;
//                         close(fds[i].fd);
//                         fds.erase(fds.begin() + i);
//                         --i;
//                     }
//                 }
//             }
//         }
//     }
//     close(server_fd);
//     return 0;
// }

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

#include "../includes/server.hpp"
#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/utils.hpp"

std::string intToString(int num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string getFileExtension(const std::string &path) {
    size_t dotPos = path.find_last_of(".");
    if (dotPos != std::string::npos)
        return path.substr(dotPos);
    return "";
}

std::string getContentType(const std::string &ext) {
    if (ext == ".html")
        return "text/html";
    if (ext == ".jpg" || ext == ".jpeg")
        return "image/jpeg";
    if (ext == ".png")
        return "image/png";
    if (ext == ".css")
        return "text/css";
    if (ext == ".js")
        return "application/javascript";
    return "application/octet-stream";
}

int launchServer(Config config) {
    std::vector<ServerConfig> servers = config.getServers();
    std::vector<int> server_fds;

    // epoll est une interface qui permet de surveiller plusieurs fd sans parcouris la liste complete des fds stocker comme poll
    //et n a pas de limite de fds comme select
    int epoll_fd = epoll_create(servers.size());
    if (epoll_fd < 0) {
        perror("ERROR: epoll_create failed");
        exit(EXIT_FAILURE);
    }

    // creation et configuartion des sockets, interface de communication entre deux process
    // AF_INET permet d etre sur une connection ipv4
    //SOCK_STREAM permet d avoir une connexion fiable en TCP
    for (size_t i = 0; i < servers.size(); i++) {
        ServerConfig server = servers[i];
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("ERROR: Error while creating the server socket");
            exit(EXIT_FAILURE);
        }
        int opt = 1;
        //setsockopt permet d activer des options sur sont socket pour ajouter des fontcionnalite comme resuaddr qui permet de pas cree de bug 
        //comme dans notre cas pouvoir relancer sur la meme ip sans avoir de pb
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("ERROR: setsockopt failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // cette structure nous permet de stocker tout les information pour pouvoir communiquer grace au socket
        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server.getPort());
        //permet de convertir une adress ip du format str en binaire
        if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
            perror("ERROR: IP conversion failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // bind permet de lie un socket a une adress ip ou un port
        if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            perror("ERROR: Error while binding the socket");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // listen permet une fois qu on a bind d ecouter sur l ip ou le port lie
        if (listen(server_fd, 10) < 0) {
            perror("ERROR: Error while listening");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // rendre la socket non bloquante
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

        server_fds.push_back(server_fd);

        // Ajout de la socket serveur à epoll grace a epoll_Ctl
        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
            perror("ERROR: epoll_ctl ADD server_fd failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
    }

    const int MAX_EVENTS = 100;
    epoll_event events[MAX_EVENTS];

    // boucle principale d attente et de traitement des evenements
    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("ERROR: epoll_wait failed");
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            // verification si l evenement provient d une socket serveur
            if (std::find(server_fds.begin(), server_fds.end(), fd) != server_fds.end()) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd >= 0) {
                    std::cout << "New connection accepted" << std::endl;
                    // rendre la socket client non bloquante
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
                    // ajout de la socket client a epoll
                    epoll_event client_event;
                    client_event.events = EPOLLIN;
                    client_event.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) < 0) {
                        perror("ERROR: epoll_ctl ADD client_fd failed");
                        close(client_fd);
                    }
                } else {
                    perror("ERROR: accept failed");
                }
            } else {
                // gestion de l evenement sur une socket client
                char buffer[4096];
                int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_read > 4096) {
                    std::cerr << "ERROR: Request too large! Closing connection." << std::endl;
                    send(fd, "HTTP/1.1 413 Payload Too Large\r\n\r\n", 35, 0);
                    close(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    continue;
                }
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    std::string rawRequest(buffer);
                    Request request(1024, 1024);
                    request.parse(rawRequest, config);
                    request.printRequest();

                    // choix du serveur à utiliser ici par defaut 0
                    Response response(request, servers[0]);
                    std::string sendResponse = response.sendResponse();

                    std::string fileContent = readFile("index.html");
                    std::string fileExt = getFileExtension("index.html");
                    std::string contentType = getContentType(fileExt);
                    if (!fileContent.empty()) {
                        send(fd, sendResponse.c_str(), sendResponse.size(), 0);
                        if (response.getStatusCode() >= 400) {
                            std::string errorPage = "./web/errors/" + intToString(response.getStatusCode()) + ".html";
                            std::string errorContent = readFile(errorPage);
                            if (!errorContent.empty()) {
                                send(fd, errorContent.c_str(), errorContent.size(), 0);
                            }
                        }
                    }
                } else {
                    std::cout << "Disconnected client" << std::endl;
                    close(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                }
            }
        }
    }

    for (size_t i = 0; i < server_fds.size(); i++) {
        close(server_fds[i]);
    }
    close(epoll_fd);
    return 0;
}

