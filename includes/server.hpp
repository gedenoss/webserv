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
    public:

        size_t verifyLoc(std::string method, std::string path, const std::string& need);
        size_t whichServerToChoose(std::vector<ServerConfig>& servers, const std::string& need, std::string &adem);
        void launchServer();
        void clean();
        void handleNewConnection();
        void handleClientData();

        void handleClientWrite();
        void modifyEpollEvent(int fd, uint32_t events);
        
        static void _sigIntCatcher(int signal);

        Server(const Config &config);
        ~Server();
    
    
    private:

        std::map<int, std::string> _pendingResponses;
        int _fd;
        int _epoll_fd;
        Config config;
        std::vector<ServerConfig> _servers;
        std::vector<int> _server_fds;
        struct sockaddr_in _addr;
        static bool _serverIsRunning;
        std::map<int, std::string>        _buffers;
        std::map<int, int>                _contentLengths;
        std::set<int>                     _headersDone;
        
        void setUp();
        
        struct sockaddr_in _client_addr;
    };
    

#endif