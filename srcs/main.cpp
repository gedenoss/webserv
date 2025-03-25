#include "../includes/config.hpp"
#include "../includes/webserv.hpp"
#include "../includes/request.hpp"
#include <fstream>
#include <sstream>

int	main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Error: Missing configuration file	argument." << std::endl;
		return 1;
	}
	Config config =	parseConfig(argv[1]);
	if (config.servers.empty())
	{
		std::cerr << "Error: No valid servers found	in config file." << std::endl;
		return 1;
	}
	else	
	{
		std::cout << "Config ok" << std::endl;
    //test
    std::cout << "Loaded server configuration:\n";
    for (size_t i = 0; i < config.servers.size(); ++i) {
    std::cout << " Server: " << config.servers[i].host << " on port " << config.servers[i].port << "\n";
    for (size_t j = 0; j < config.servers[i].locations.size(); ++j) {
        std::cout << "  - Location: " << config.servers[i].locations[j].path << "\n";
        std::cout << "    Allowed methods: ";
        for (size_t k = 0; k < config.servers[i].locations[j].allow_methods.size(); ++k) {
            std::cout << config.servers[i].locations[j].allow_methods[k] << " ";
        }
        std::cout << "\n";
    }
}
		std::string	rawRequest =  "POST /admin HTTP/1.1\r\n"
								 "Host: localhost\r\n"
								 "User-Agent: Mozilla/5.0\r\n"
								 "Accept: text/html\r\n"
								 "Content-Length: 13\r\n"
								 "\r\n"
								 "Hello, world!";
		
		HttpRequestParser parser(1024, 1024);
		int	errorCode =	0;
		Request	request	= parser.parse(rawRequest, errorCode, config);
    std::map<std::string, std::string>::const_iterator contentLengthIt = request.getHeaders().find("Content-Length");

if (contentLengthIt != request.getHeaders().end()) {
    std::cout << "Content-Length header found: " << contentLengthIt->second << std::endl;
} else {
    std::cout << "Content-Length header not found!" << std::endl;
}

		if (errorCode != 200)
		{
			std::cerr << "Error	parsing	request. HTTP Error	Code: "	<< errorCode << std::endl;
			return 1;
		}
		request.printRequest();
		Webserv	webserv;
		webserv.webserv(config);
	}

	return 0;
}


