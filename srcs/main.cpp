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
		
		std::string	rawRequest = "GET /index.html HTTP/1.1\r\n"
								 "Host:	localhost\r\n"
								 "User-Agent: Mozilla/5.0\r\n"
								 "Accept: text/html\r\n"
								 "Content-Length: 13\r\n"
								 "\r\n"
								 "Hello, world!";
		
		HttpRequestParser parser(1024, 1024);
		int	errorCode =	0;
		Request	request	= parser.parse(rawRequest, errorCode, config);
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


