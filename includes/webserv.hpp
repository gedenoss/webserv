#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include "config.hpp"

class Webserv {
    public : 
        Webserv();
        ~Webserv();

        void webserv(const Config& config);
};

#endif