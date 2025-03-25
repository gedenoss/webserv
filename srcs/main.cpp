#include "../includes/config.hpp"
#include "../includes/webserv.hpp"
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
	std::cout << server.size() << std::endl;
	if (server.empty())
	{
		std::cerr << "Error: No valid getServers() found	in config file." << std::endl;
		return 1;
	}
	else	
	{
		launchServer(config, server[0]);
	}
// 		std::string	rawRequest =  "POST /admin HTTP/1.1\r\n"
// 								 "Host: localhost\r\n"
// 								 "User-Agent: Mozilla/5.0\r\n"
// 								 "Accept: text/html\r\n"
// 								 "Content-Length: 13\r\n"
// 								 "\r\n"
// 								 "Hello, world!";
		
// 		HttpRequestParser parser(1024, 1024);
// 		Request	request	= parser.parse(rawRequest, config);
//     std::map<std::string, std::string>::const_iterator contentLengthIt = request.getHeaders().find("Content-Length");

// if (contentLengthIt != request.getHeaders().end()) {
//     std::cout << "Content-Length header found: " << contentLengthIt->second << std::endl;
// } else {
//     std::cout << "Content-Length header not found!" << std::endl;
// }

// 		if (errorCode != 200)
// 		{
// 			std::cerr << "Error	parsing	request. HTTP Error	Code: "	<< errorCode << std::endl;
// 			return 1;
// // 		}
// 		request.printRequest();
		return 0;
	}




