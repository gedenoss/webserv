#include "../includes/server.hpp"

// ServerConfig parseConfig(const std::string &filename){
//     ServerConfig config;
//     std::ifstream file(filename);
//     if (!file.is_open()){
//         std::cerr << "ERROR: Error while oppening the configuration file" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     std::string line;
//     while (std::getline(file, line)){
//         if (line.empty() || line[0] == '#')
//             continue;
//         std::istringstream iss(line);
//         std::string key, value;
//         if (std::getline(iss, key, '=') && std::getline(iss, value)){
//             if (key == "IP")
//                 config.ip = value;
//             else if (key == "PORT")
//                 config.port = std::stoi(value);
//         }
//     }
//     return config;
// }

// std::string readHtmlFile(const std::string &filename){
//     std::ifstream file(filename);
//     if (!file.is_open()){
//         return "";
//     }
//     std::stringstream buffer;
//     buffer << file.rdbuf();
//     return buffer.str();
// }

// int main()
// {
//     ServerConfig config = parseConfig("server.conf");
//     std::cout << "IP: " << config.ip << ", PORT: " << config.port << std::endl;
    
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
//     server_address.sin_port = htons(config.port);
//     if (inet_pton(AF_INET, config.ip.c_str(), &server_address.sin_addr) <= 0) {
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
    
//     std::cout << "Server launched and listening on " << config.ip 
//               << ":" << config.port << std::endl;
    
//     std::vector<pollfd> fds;
//     fds.push_back({server_fd, POLLIN, 0});
    
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
//                         std::cout << "New connexion accepted" << std::endl;
//                         fds.push_back({client_fd, POLLIN, 0});
//                     } else {
//                         perror("ERROR: accept failed");
//                     }
//                 } else {
//                     char buffer[1024];
//                     int bytes_read = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
//                     if (bytes_read > 0) {
//                         buffer[bytes_read] = '\0';
//                         std::cout << "Message received: " << buffer << std::endl;
//                         std::string htmlContent = readHtmlFile("cube.html");
//                         if (htmlContent.empty()){
//                             htmlContent = "<h1>Erreur 404: Page non trouvée</h1>";
//                         }
//                         std::string response = "HTTP/1.1 200 OK\r\n"
//                         "Content-Type: text/html\r\n"
//                         "Content-Length: " + std::to_string(htmlContent.size()) + "\r\n"
//                         "\r\n" +
//                         htmlContent;
//                     send(fds[i].fd, response.c_str(), response.size(), 0);
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


ServerConfig parseConfig(const std::string &filename) {
    ServerConfig config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Error while opening the configuration file" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "IP")
                config.ip = value;
            else if (key == "PORT")
                config.port = std::stoi(value);
        }
    }
    return config;
}

std::string readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getFileExtension(const std::string &path) {
    size_t dotPos = path.find_last_of(".");
    if (dotPos != std::string::npos)
        return path.substr(dotPos);
    return "";
}

std::string getContentType(const std::string &ext) {
    if (ext == ".html") return "text/html";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    return "application/octet-stream";
}

int main() {
    ServerConfig config = parseConfig("server.conf");
    std::cout << "IP: " << config.ip << ", PORT: " << config.port << std::endl;
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("ERROR: Error while creating the server");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("ERROR: Error while activating SO_REUSEADDR");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(config.port);
    if (inet_pton(AF_INET, config.ip.c_str(), &server_address.sin_addr) <= 0) {
        perror("ERROR: IP conversion");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("ERROR: Error while binding the socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("ERROR: Error while listening");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server launched and listening on " << config.ip 
              << ":" << config.port << std::endl;
    
    std::vector<pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});
    
    while (true) {
        int pol_ret = poll(fds.data(), fds.size(), -1);
        if (pol_ret < 0) {
            perror("ERROR: poll Error");
            break;
        }
        
        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                    if (client_fd >= 0) {
                        std::cout << "New connection accepted" << std::endl;
                        fds.push_back({client_fd, POLLIN, 0});
                    } else {
                        perror("ERROR: accept failed");
                    }
                } else {
                    char buffer[1024];
                    int bytes_read = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes_read > 0) {
                        buffer[bytes_read] = '\0';
                        std::string request(buffer);
                        std::string filename = "index.html";
                        size_t start = request.find("GET ") + 4;
                        size_t end = request.find(" ", start);
                        if (start != std::string::npos && end != std::string::npos) {
                            std::string path = request.substr(start, end - start);
                            if (path == "/") {
                                filename = "index.html";
                            } else {
                                filename = path.substr(1);
                            }
                        }
                        std::string fileContent = readFile(filename);
                        std::string fileExt = getFileExtension(filename);
                        std::string contentType = getContentType(fileExt);
                        if (!fileContent.empty()) {
                            std::string response = "HTTP/1.1 200 OK\r\n"
                                                   "Content-Type: " + contentType + "\r\n"
                                                   "Content-Length: " + std::to_string(fileContent.size()) + "\r\n"
                                                   "\r\n" +
                                                   fileContent;
                            send(fds[i].fd, response.c_str(), response.size(), 0);
                        } else {
                            std::string notFound = "HTTP/1.1 404 Not Found\r\n"
                                                   "Content-Type: text/html\r\n"
                                                   "Content-Length: 46\r\n"
                                                   "\r\n"
                                                   "<h1>Erreur 404: Page ou fichier non trouvé</h1>";
                            send(fds[i].fd, notFound.c_str(), notFound.size(), 0);
                        }
                    } else {
                        std::cout << "Disconnected client" << std::endl;
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                    }
                }
            }
        }
    }
    close(server_fd);
    return 0;
}
