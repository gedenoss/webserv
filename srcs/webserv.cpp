#include "../includes/webserv.hpp"
#include "../includes/response.hpp"
#include "../includes/request.hpp"

Webserv::Webserv()
{
}

void Webserv::webserv(char *argv)
{
    (void)argv;
    std::cout << "Hello Webserv! " << argv <<  "\n" << std::endl;
    Request request;
    Response response(request);
    response.sendResponse(request);
}

Webserv::~Webserv()
{
}