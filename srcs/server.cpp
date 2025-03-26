#include "../includes/server.hpp"
#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/utils.hpp"

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

int launchServer(Config config, ServerConfig server) {
    
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
    server_address.sin_port = htons(server.getPort());
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
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
    
    std::vector<pollfd> fds;
    pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;
    fds.push_back(server_pollfd);
    
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
                        pollfd client_pollfd;
                        client_pollfd.fd = client_fd;
                        client_pollfd.events = POLLIN;
                        client_pollfd.revents = 0;
                        fds.push_back(client_pollfd);
                    } else {
                        perror("ERROR: accept failed");
                    }
                } else {
                    char buffer[4096];
                    int bytes_read = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes_read > 0) {
                        buffer[bytes_read] = '\0';
                        std::string rawRequest(buffer);
                        //std::cout << rawRequest << std::endl;

                        Request request(1024,1024);
                        request.parse(rawRequest, config);
                        request.printRequest();
                        Response response(request);
                        std::string sendResponse = response.sendResponse(request);
                        
                        std::string fileContent = readFile("index.html");
                        std::string fileExt = getFileExtension("index.html");
                        std::string contentType = getContentType(fileExt);
                        if (!fileContent.empty()) {
                            send(fds[i].fd, sendResponse.c_str(), sendResponse.size(), 0);
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
