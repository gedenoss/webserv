#include "../includes/webserv.hpp"
#include "../includes/config.hpp"

int main (int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Missing argument" << std::endl;
        return 1;
    }

    Config config = parseConfig(argv[1]);

    
    if (config.servers.empty()) {
        std::cerr << "Error: No valid servers found in config file." << std::endl;
        return 1;
    }
    else
    {
        
        Webserv webserv;
        webserv.webserv(config);

        std::cout << "Server started with config: " << argv[1] << std::endl;
    }

    return 0;
}
