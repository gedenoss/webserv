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
#include "../includes/upload.hpp"

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

    // epoll est une interface qui permet de surveiller plusieurs fd sans parcourir la liste complète des fds, contrairement à poll, et n'a pas de limite de fds comme select
    int epoll_fd = epoll_create(servers.size());
    if (epoll_fd < 0) {
        perror("ERROR: epoll_create failed");
        exit(EXIT_FAILURE);
    }

    // Création et configuration des sockets, interface de communication entre deux processus
    for (size_t i = 0; i < servers.size(); i++) {
        ServerConfig server = servers[i];
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("ERROR: Error while creating the server socket");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        // setsockopt permet d'activer des options sur le socket, comme SO_REUSEADDR
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("ERROR: setsockopt failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Configuration de l'adresse du serveur
        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server.getPort());
        if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
            perror("ERROR: IP conversion failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Lier le socket à l'adresse et au port
        if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            perror("ERROR: Error while binding the socket");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Écouter les connexions entrantes
        if (listen(server_fd, 10) < 0) {
            perror("ERROR: Error while listening");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Rendre la socket non bloquante
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

        server_fds.push_back(server_fd);

        // Ajouter la socket serveur à epoll
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

    // Boucle principale d'attente et de traitement des événements
    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("ERROR: epoll_wait failed");
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            // Vérification si l'événement provient d'une socket serveur
            if (std::find(server_fds.begin(), server_fds.end(), fd) != server_fds.end()) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd >= 0) {
                    std::cout << "New connection accepted" << std::endl;
                    // Rendre la socket client non bloquante
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
                    // Ajouter la socket client à epoll
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
                // Gestion de l'événement sur une socket client
                std::string fullRequest = "";
                char buffer[4096];
                ssize_t bytesRead;
                bool headersParsed = false;
                size_t contentLength = 0;

                while (true) {
                    bytesRead = recv(fd, buffer, sizeof(buffer), 0);
                    if (bytesRead <= 0) {
                        std::cerr << "Client disconnected or error" << std::endl;
                        close(fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        break;  // Quitte la boucle et ne traite pas plus de données
                    }

                    fullRequest.append(buffer, bytesRead);

                    // Cherche la fin des headers
                    size_t headerEnd = fullRequest.find("\r\n\r\n");
                    if (headerEnd != std::string::npos && !headersParsed) {
                        headersParsed = true;
                        std::string headerPart = fullRequest.substr(0, headerEnd + 4);

                        // Extrait Content-Length si présent
                        size_t pos = headerPart.find("Content-Length:");
                        if (pos != std::string::npos) {
                            size_t endLine = headerPart.find("\r\n", pos);
                            std::string value = headerPart.substr(pos + 15, endLine - pos - 15);
                            contentLength = std::strtoul(value.c_str(), NULL, 10);
                        }
                    }

                    if (headersParsed) {
                        size_t totalExpected = fullRequest.find("\r\n\r\n") + 4 + contentLength;
                        if (fullRequest.size() >= totalExpected) {
                            break;  // Requête complète reçue
                        }
                    }

                    if (fullRequest.size() > 1024 * 1024) { // Met un max pour éviter les abus
                        std::cerr << "ERROR: Request too large! Closing connection." << std::endl;
                        send(fd, "HTTP/1.1 413 Payload Too Large\r\n\r\n", 35, 0);
                        close(fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        break;  // Quitte la boucle
                    }
                }

                // Traite la requête après lecture complète
                Request request(1024, 1024);
                
                request.parse(fullRequest, config);
                request.printRequest();

                // Gérer /upload POST
                if (request.getMethod() == "POST" && request.getUrl() == "/upload") {
                    try {
                        handleUpload(request, "/home/gbouguer/42/webserv/upload");
                        std::string response = "HTTP/1.1 201 Created\r\n\r\nFile uploaded successfully.";
                        send(fd, response.c_str(), response.size(), 0);
                    } catch (const std::exception &e) {
                        std::string response = "HTTP/1.1 500 Internal Server Error\r\n\r\n" + std::string(e.what());
                        send(fd, response.c_str(), response.size(), 0);
                    }
                } else {
                    // Choix du serveur par défaut
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
                }
            }
        }
    }

    // Fermeture des descripteurs de fichiers
    for (size_t i = 0; i < server_fds.size(); i++) {
        close(server_fds[i]);
    }
    close(epoll_fd);
    return 0;
}
