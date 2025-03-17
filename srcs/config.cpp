#include "../includes/config.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <map>
#include <set>

bool isValidIP(const std::string& ip)
{
	int	numDots	= 0;
	int	numDigits =	0;
	int	currentNumber =	0;

	for	(size_t	i =	0; i < ip.length();	++i)
	{
		char c = ip[i];

		if (c == '.')
		{
			if (currentNumber <	0 || currentNumber > 255)
				return false;
			numDots++;
			currentNumber =	0;
			numDigits =	0;
		}
		else if (isdigit(c))
		{
			currentNumber =	currentNumber *	10 + (c - '0');
			numDigits++;
			if (numDigits >	3)
				return false;
		}
		else
			return false;
	}
	if (numDots	!= 3 || currentNumber <	0 || currentNumber > 255) 
		return false;
	return true;
}

bool isPathValid(const std::string&	path)
{
	struct stat	info;
	return (stat(path.c_str(), &info) == 0);
}

bool isFileValid(const std::string&	filePath)
{
	struct stat	info;
	return (stat(filePath.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

bool isHttpErrorCodeValid(int code)
{
	return (code >= 400	&& code	<= 599);
}

bool isNumber(const	std::string& str)
{
	for	(size_t	i =	0; i < str.length(); ++i)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

std::string	cleanValue(std::string value)
{
	while (!value.empty() && (value[value.size() - 1] == ';' || isspace(value[value.size() - 1]))) 
		value.erase(value.size() - 1);
	return value;
}

void parseLocation(std::ifstream& configFile, LocationConfig& location)
{
	std::set<std::string> usedKeys;
	std::string	line;
	while (std::getline(configFile,	line))
	{
		std::istringstream iss(line);
		std::string	key;
		iss	>> key;
		if (key.empty()) continue;
		if (key	== "}")	break;
		
		if (usedKeys.find(key) != usedKeys.end())
		{
			std::cerr << "Error: Duplicate directive in location block:	" << key << std::endl;
			exit(1);
		}
		usedKeys.insert(key);
		if (key	== "root")
		{
			iss	>> location.root;
			location.root =	cleanValue(location.root);
			if (!isPathValid(location.root))
			{
				std::cerr << "Error: Invalid root path:	" << location.root << std::endl;
				exit(1);
			}
		}
		else if (key == "index")
		{
			iss	>> location.index;
			location.index = cleanValue(location.index);
			std::string	fullPath = location.root + "/" + location.index;
			if (!isFileValid(fullPath))
			{
				std::cerr << "Warning: Default index file does not exist: "	<< fullPath	<< std::endl;
			}
		}
		else if (key == "allow_methods")
		{
			std::string	method;
			while (iss >> method)
				location.allow_methods.push_back(cleanValue(method));
		}
		else if (key == "autoindex")
		{
			std::string	value;
			iss	>> value;
			value =	cleanValue(value);
			if (value == "on")
				location.autoindex = true;
			else if (value == "off")
				location.autoindex = false;
			else
			{
				std::cerr << "Error: Invalid value for autoindex: "	<< value << std::endl;
				exit(1);
			}
		}		
		else
		{
			std::cerr << "Error: Unknown directive in location block: "	<< key << std::endl;
			exit(1);
		}
	}
}

void parseServer(std::ifstream&	configFile,	ServerConfig& server)
{
	std::string	line;
	std::set<std::string> usedKeys;
	while (std::getline(configFile,	line))
	{
		std::istringstream iss(line);
		std::string	key;
		iss	>> key;
		if (key.empty()) continue;
		if (key	== "}")	break;

		if (key	!= "location" && usedKeys.find(key)	!= usedKeys.end())
		{
			std::cerr << "Error: Duplicate directive in server block: "	<< key << std::endl;
			exit(1);
		}
		usedKeys.insert(key);			
		if (key	== "listen")
		{
			std::string	listenValue;
			iss	>> listenValue;
			listenValue	= cleanValue(listenValue);
		
			
			size_t colonPos	= listenValue.find(':');
			if (colonPos != std::string::npos)
			{
				
				server.host	= listenValue.substr(0,	colonPos);
				std::string	portStr	= listenValue.substr(colonPos +	1);
				if (!isNumber(portStr))
				{
					std::cerr << "Error: Invalid port number: "	<< portStr << std::endl;
					exit(1);
				}
				server.port	= atoi(portStr.c_str());
				if (!isValidIP(server.host))
				{
					std::cerr << "Error: Invalid host address: " << server.host	<< std::endl;
					exit(1);
				}
			}
			else
			{
				if (!isNumber(listenValue))
				{
					std::cerr << "Error: Invalid port number: "	<< listenValue << std::endl;
					exit(1);
				}
				server.port	= atoi(listenValue.c_str());
				server.host	= "";  
			}
			if (server.port	< 1	|| server.port > 65535)
			{
				std::cerr << "Error: Port number out of range: " << server.port	<< std::endl;
				exit(1);
			}
		}
		else if (key == "host")
		{
			iss	>> server.host;
			server.host	= cleanValue(server.host);
			if (server.host.empty()	|| !isValidIP(server.host))
			{
				std::cerr << "Error: Invalid host address "	<< std::endl;
				exit(1);
			}
		}
		else if (key == "server_name")
		{
			iss	>> server.server_name;
			server.server_name = cleanValue(server.server_name);
		}
		else if (key == "index")
		{
			iss	>> server.index;
			server.index = cleanValue(server.index);
			std::string	fullPath = server.root + "/" + server.index;
			if (!isFileValid(fullPath))
			{
				std::cerr << "Warning: Default index file does not exist: "	<< fullPath	<< std::endl;
			}
		}
		else if (key == "root")
		{
			iss	>> server.root;
			server.root	= cleanValue(server.root);
			if (!isPathValid(server.root))
			{
				std::cerr << "Error: Invalid root path:	" << server.root << std::endl;
				exit(1);
			}
		}
		else if (key == "client_max_body_size")
		{
			std::string	size_str;
			iss	>> size_str;
			size_str = cleanValue(size_str);
			
			if (size_str.empty())
			{
				std::cerr << "Error: client_max_body_size is empty"	<< std::endl;
				exit(1);
			}
			
			char unit =	size_str[size_str.size() - 1];
			std::string	numberPart = size_str.substr(0,	size_str.size()	- 1);
			
			int	multiplier = 1;
			if (unit == 'K'	|| unit	== 'k')	multiplier = 1024;
			else if (unit == 'M' || unit == 'm') multiplier	= 1024 * 1024;
			else if (unit == 'G' || unit == 'g') multiplier	= 1024 * 1024 *	1024;
			else if (isdigit(unit))	numberPart += unit;
			else
			{
				std::cerr << "Error: Invalid client_max_body_size unit:	" << unit << std::endl;
				exit(1);
			}
			
			if (!isNumber(numberPart))
			{
				std::cerr << "Error: Invalid client_max_body_size: " << size_str << std::endl;
				exit(1);
			}
			
			server.client_max_body_size	= atoi(numberPart.c_str()) * multiplier;
		}
		else if (key == "error_page")
		{
			std::string	code_str, page;
			iss	>> code_str	>> page;
			code_str = cleanValue(code_str);
			page = cleanValue(page);
			if (!isNumber(code_str)	|| !isHttpErrorCodeValid(atoi(code_str.c_str())))
			{
				std::cerr << "Error: Invalid error code: " << code_str << std::endl;
				exit(1);
			}
			server.error_pages[atoi(code_str.c_str())] = page;
		}
		else if (key == "location")
		{
			LocationConfig location;
			iss	>> location.path;
			location.path =	cleanValue(location.path);
			std::set<std::string> newLocationKeys;
			parseLocation(configFile, location);

			server.locations.push_back(location);
		}
		else
		{
			std::cerr << "Error: Unknown directive in server block:	" << key << std::endl;
			exit(1);
		}
	}
}

Config parseConfig(const std::string& filename)
{
	Config config;
	std::ifstream configFile(filename.c_str());
	if (!configFile.is_open())
	{
		std::cerr << "Error: Could not open	config file: " << filename << std::endl;
		exit(1);
	}
	std::string	line;
	std::set<std::string> globalKeys;
	while (std::getline(configFile,	line))
	{
		std::istringstream iss(line);
		std::string	key;
		iss	>> key;
		if (key.empty()) continue;
		
		if (key	== "server")
		{
			ServerConfig server;
			parseServer(configFile,	server);
			config.servers.push_back(server);
		}
		else
		{
			std::cerr << "Error: Unknown directive outside any block: "	<< key << std::endl;
			exit(1);
		}
	}

	return config;
}