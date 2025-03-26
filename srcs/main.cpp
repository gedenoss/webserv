#include "../includes/config.hpp"
#include "../includes/server.hpp"
#include "../includes/request.hpp"
#include "../includes/server.hpp"
#include <fstream>
#include <sstream>

int	main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Error: Missing configuration file	argument." << std::endl;
		return 1;
	}
	Config config;
	config.parseConfig(argv[1]);
	std::vector<ServerConfig> server = config.getServers();
	if (server.empty())
	{
		std::cerr << "Error: No valid getServers() found	in config file." << std::endl;
		return 1;
	}
	else	
	{
		launchServer(config, server[0]);
	}
		return 0;
	}




