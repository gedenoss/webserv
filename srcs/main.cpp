#include "../includes/config.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Error: Missing configuration file argument." << std::endl;
        return 1;
    }
    Config config = parseConfig(argv[1]);
    if (config.servers.empty())
    {
        std::cerr << "Error: No valid servers found in config file." << std::endl;
        return 1;
    }
    std::cout << "Configuration successfully loaded." << std::endl;
    return 0;
}

