#include "webserv.hpp"

int main (int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Missing argument" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Hello " << argv[1] << "!" << std::endl;
    }
    return 0;
}