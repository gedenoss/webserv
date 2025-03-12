#include "../includes/webserv.hpp"

int main (int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Missing argument" << std::endl;
        return 1;
    }
    else
    {
        Webserv webserv;
        webserv.webserv(argv[1]);
    }
    return 0;
}