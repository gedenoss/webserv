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

#include <set>   
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

class Server {
    public:

        void launchServer();
        void clean();
        void handleNewConnection();
        void handleClientData();

        Server(const Config &config);
        ~Server();
    
    
    private:

        int _fd;
        int _epoll_fd;
        Config config;
        std::vector<ServerConfig> _servers;
        std::vector<int> _server_fds;
        struct sockaddr_in _addr;
        std::map<int, std::string>        _buffers;
        std::map<int, int>                _contentLengths;
        std::set<int>                     _headersDone;
        
        void setUp();
        
        struct sockaddr_in _client_addr;
    };

#endif