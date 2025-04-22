#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

#include <poll.h>

#include "config.hpp"

int launchServer(Config config);


class Server {
    public:
        Server(const Config &config);
        ~Server();
    
        void run();
    
    private:
        int epoll_fd;
        Config config;
        std::vector<ServerConfig> servers;
        std::vector<int> server_fds;
    
        void initSockets();
        void addToEpoll(int fd);
        void handleNewConnection(int server_fd);
        void handleClient(int client_fd);
        void cleanup();
    };

#endif