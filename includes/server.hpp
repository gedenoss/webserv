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

#include <poll.h>

#include "config.hpp"

int launchServer(Config config);

#endif