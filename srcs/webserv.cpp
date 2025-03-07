#include "../includes/webserv.hpp"
#include "../includes/response.hpp"

Webserv::Webserv()
{
}

void Webserv::webserv(char *argv)
{
    (void)argv;
    std::cout << "Hello Webserv! " << argv <<  std::endl;
    Response response = Response();
    response.set_time();
}

Webserv::~Webserv()
{
}