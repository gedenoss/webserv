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
#include <csignal>
#include <poll.h>

#include "config.hpp"

#define RED        "\033[0;31m"
#define GREEN    "\033[0;32m"
#define YELLOW    "\033[0;33m"
#define ORANGE "\033[38;5;208m"
#define BLUE    "\033[0;34m"
#define MAGENTA    "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE    "\033[0;37m"
#define BOLD       "\033[1m"
#define UNDERLINE  "\033[4m"
#define ITALIC     "\033[3m"
#define RESET      "\033[0m"

class Server {
    private:
        Config config;
        std::vector<ServerConfig> servers;
        std::vector<int> server_fds;
        static bool _serverIsRunning;
    
    public:
        Server(const Config &config) {
            this->config = config;
            this->servers = config.getServers();
            this->server_fds = std::vector<int>(servers.size());
            // _serverIsRunning = true; // <-- enlever le `this->`
        };
    
        ~Server() {}
    
        static void _sigIntCatcher(int signal);
        int launchServer(Config config);
    };
    

#endif