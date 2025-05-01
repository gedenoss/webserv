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

size_t whichServerToChoose(std::vector<ServerConfig>& servers, const std::string& need) {
    for (size_t i = 0; i < servers.size(); ++i) {
        if (static_cast<size_t>(servers[i].getPort()) == stringToSizeT(need))
            return i;
    }
    return 0;
}

bool Server::_serverIsRunning = true;

void Server::_sigIntCatcher(int signal) {
    std::cout << RED << BOLD << "\nSIGINT received. Shutting down server..." << RESET << std::endl;
    _serverIsRunning = false;
    (void)signal;
}

Server::Server(const Config &conf)
    : _epoll_fd(-1), config(conf), _addr() {
    setUp();
}

Server::~Server() {
    for (std::vector<int>::size_type i = 0; i < _server_fds.size(); ++i)
        close(_server_fds[i]);
    close(_epoll_fd);
}

void Server::clean() {
    close(_fd);
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _fd, NULL);
    _buffers.erase(_fd);
    _contentLengths.erase(_fd);
    _headersDone.erase(_fd);
    _pendingResponses.erase(_fd);
}

void Server::setUp() {
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
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

        std::memset(&_addr, 0, sizeof(_addr));
        _addr.sin_family = AF_INET;
        _addr.sin_port   = htons(server.getPort());
        inet_pton(AF_INET, "127.0.0.1", &_addr.sin_addr);

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
        epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
    }
}

void Server::modifyEpollEvent(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void Server::handleNewConnection() {
    socklen_t client_len = sizeof(_client_addr);
    int client_fd = accept(_fd, (struct sockaddr*)&_client_addr, &client_len);
    if (client_fd >= 0) {
        int fl = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, fl | O_NONBLOCK);
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = client_fd;
        epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
    } else {
        perror("ERROR: accept failed");
    }
}

std::string getHostRawRequest(std::string rq) {
    size_t hostPos = rq.find("Host:") + 6;
    std::string hostname = rq.substr(hostPos);
    size_t colonPos = hostname.find(':');
    if (colonPos != std::string::npos) {
        hostname = hostname.substr(colonPos + 1);
    }
    return hostname;
}

void Server::handleClientData() {
    char buf[4096];
    while (1) {
        int r = recv(_fd, buf, sizeof(buf) - 1, 0);
        if (r <= 0) break;
        _buffers[_fd].append(buf, r);
        if (_buffers[_fd].size() > 4069 * 1000 * 100) {
            std::cerr << "ERROR: Request too large\n";
            _pendingResponses[_fd] = "HTTP/1.1 413 Payload Too Large\r\n\r\n";
            modifyEpollEvent(_fd, EPOLLIN | EPOLLOUT);
            return;
        }
    }

    if (!_headersDone.count(_fd)) {
        std::string &rq = _buffers[_fd];
        size_t end_hdr = rq.find("\r\n\r\n");
        if (end_hdr != std::string::npos) {
            _headersDone.insert(_fd);
            size_t pos = rq.find("Content-Length:");
            if (pos != std::string::npos) {
                size_t line_end = rq.find("\r\n", pos);
                std::string cl = rq.substr(pos + 15, line_end - (pos + 15));
                _contentLengths[_fd] = std::atoi(cl.c_str());
            } else {
                _contentLengths[_fd] = 0;
            }
        }
    }

    if (_headersDone.count(_fd)) {
        std::string &rq = _buffers[_fd];
        size_t end_hdr = rq.find("\r\n\r\n");
        int expected = _contentLengths[_fd];
        size_t have_body = rq.size() - (end_hdr + 4);
        if ((int)have_body < expected)
            return;

        std::string need = getHostRawRequest(rq);
        size_t i = whichServerToChoose(_servers, need);
        std::cout << BLUE << BOLD << "Port choosen : " << _servers[i].getPort() << RESET << std::endl;
        size_t maxBodySize = _servers[i].getClientMaxBodySize();          
        Request  request(maxBodySize,1024);
        request.parse(rq, config);
        Response response(request, _servers[i]);
        std::string reply = response.sendResponse();

        _pendingResponses[_fd] = reply;
        modifyEpollEvent(_fd, EPOLLIN | EPOLLOUT);
    }
}

void Server::handleClientWrite() {
    std::string &data = _pendingResponses[_fd];
    ssize_t sent = send(_fd, data.c_str(), data.size(), 0);
    if (sent <= 0) {
        clean();
        return;
    }
    data.erase(0, sent);
    if (data.empty()) {
        _pendingResponses.erase(_fd);
        modifyEpollEvent(_fd, EPOLLIN);
        clean(); // connection keep-alive à implémenter si besoin
    }
}

void Server::launchServer() {
    const int MAX_EVENTS = 100;
    struct epoll_event events[MAX_EVENTS];
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, _sigIntCatcher);

    while (_serverIsRunning) {
        int n = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            if (!_serverIsRunning)
                break;
            perror("ERROR: epoll_wait failed");
            break;
        }
        for (int i = 0; i < n; ++i) {
            _fd = events[i].data.fd;
            if (std::find(_server_fds.begin(), _server_fds.end(), _fd)
                != _server_fds.end()) {
                handleNewConnection();
            } else {
                if (events[i].events & EPOLLIN)
                    handleClientData();
                if (events[i].events & EPOLLOUT)
                    handleClientWrite();
            }
        }
    }
}
