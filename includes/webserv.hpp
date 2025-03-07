#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <string>
#include <vector>
#include <ctime>

class Webserv {
    public : 
        Webserv();
        ~Webserv();

        void webserv(char *argv);
};

#endif