#include "../includes/server.hpp"
#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/utils.hpp"

int launchServer(Config config) {
    Server server(config);
    server.run();
    return 0;
}

Server::Server(const Config &cfg) : config(cfg) {
    servers = config.getServers();
    epoll_fd = epoll_create(servers.size());
    if (epoll_fd < 0) {
        perror("ERROR: epoll_create failed");
        exit(EXIT_FAILURE);
    }
    initSockets();
}

Server::~Server() {
    cleanup();
}

void Server::initSockets() {
    for (size_t i = 0; i < servers.size(); i++) {
        ServerConfig &server = servers[i];
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("ERROR: socket failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(server.getPort());
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
            listen(server_fd, 10) < 0) {
            perror("ERROR: bind/listen failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL, 0) | O_NONBLOCK);
        server_fds.push_back(server_fd);
        addToEpoll(server_fd);
    }
}

void Server::addToEpoll(int fd) {
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("ERROR: epoll_ctl ADD failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void Server::run() {
    const int MAX_EVENTS = 100;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("ERROR: epoll_wait failed");
            break;
        }
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (std::find(server_fds.begin(), server_fds.end(), fd) != server_fds.end()) {
                handleNewConnection(fd);
            } else {
                handleClient(fd);
            }
        }
    }
}

std::string intToString(int num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}


void Server::handleNewConnection(int server_fd) {
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &len);
    if (client_fd >= 0) {
        fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);
        addToEpoll(client_fd);
        std::cout << "New client connected." << std::endl;
    } else {
        perror("ERROR: accept failed");
    }
}

int whichServerToChoose(std::vector<ServerConfig>& servers, const std::string& host) {
    
    size_t pos = host.find(':');
    if (pos == std::string::npos) {
        std::cerr << "ERROR: Invalid host format (missing ':')" << std::endl;
        return -1;
    }
    std::string port = host.substr(pos + 1);
    for (size_t i = 0; i < servers.size(); ++i) {
        if (intToString(servers[i].getPort()) == port)
            return i;
    }

    return -1;
}


void Server::handleClient(int client_fd) {
    char buffer[4096];
    int contentLenght;
    std::string rawRequest(buffer);
    Request request(1024, 1024);
    request.parse(rawRequest, config);
    Response tmpResponse(request, servers[0]);
    contentLenght = tmpResponse.getContentLength();
    std::cout << "laaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : " << contentLenght << std::endl;
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read > 4096) {
    std::cerr << "ERROR: Request too large." << std::endl;
    send(client_fd, "HTTP/1.1 413 Payload Too Large\r\n\r\n", 35, 0);
    close(client_fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    return;
    }
    std::string need;
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';

        need = request.getIp();
        size_t i = whichServerToChoose(servers, need);
        if (i == static_cast<size_t>(-1) || i >= servers.size()) {
            std::cerr << "ERROR: No matching server found for host: " << need << std::endl;
            close(client_fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
            return;
        }
        std::cout << "\033[1;31m" << i << "\033[0m" << std::endl;
        Response response(request, servers[i]);
        std::string resp = response.sendResponse();
        send(client_fd, resp.c_str(), resp.size(), 0);
    } else {
        std::cout << "Client disconnected." << std::endl;
        close(client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    }
}

void Server::cleanup() {
    for (size_t i = 0; i < server_fds.size(); ++i)
        close(server_fds[i]);
    close(epoll_fd);
}
