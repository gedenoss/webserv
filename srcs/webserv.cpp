#include "../includes/webserv.hpp"
#include "../includes/response.hpp"
#include "../includes/request.hpp"

Webserv::Webserv()
{
}

void Webserv::webserv(char *argv)
{
    (void)argv;
    std::cout << "Hello Webserv! " << argv <<  std::endl;
    Request request;
    Response response;
    response.send_response(request);
}

Webserv::~Webserv()
{
}