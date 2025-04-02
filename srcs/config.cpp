#include "../includes/config.hpp"
#include "../includes/utils.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <map>
#include <set>
// a faire
// parametre tout le bloc server
// mettre ip de base si pas d ip
//gerer les ;
//-----------------------------------------GETTER-------------------------------------------------------//

int ServerConfig::getPort()
{
	return port;
}

std::vector<LocationConfig> ServerConfig::getLocations() const
{
	return locations;
}

std::vector<ServerConfig> Config::getServers() const
{
	return servers;
}

//-----------------------------------------CLEAN FUNCTIONS-------------------------------------------------------//

std::string	cleanValue(std::string value)
{
	while (!value.empty() && (value[value.size() - 1] == ';' || isspace(value[value.size() - 1]))) 
		value.erase(value.size() - 1);
	return value;
}

std::string cleanForLoc(std::string line){
	if(line[line.size() - 1] == '{'){
		line.erase(line.size() - 1);
		return line;
	}
	return NULL;
}

//-----------------------------------------------------SERVER HANDLER-----------------------------------------------------------------//

void ServerConfig::handleListen(std::istringstream& iss, std::string line)
{
	std::string listenValue;
	iss >> listenValue;
	if (countWords(line) != 2){
		exit(1);
	}
	(void)line;	
	listenValue = cleanValue(listenValue);
	if (listenValue.empty())
	{
		std::cerr << "Error: Missing value for 'listen' directive." << std::endl;
		exit(1);
	}
	size_t colonPos = listenValue.find(':');
	if (colonPos != std::string::npos)
	{
		host = listenValue.substr(0, colonPos);
		std::string portStr = listenValue.substr(colonPos + 1);
		if (!isNumber(portStr))
		{
			std::cerr << "Error: Invalid port number: " << portStr << std::endl;
			exit(1);
		}
		port = atoi(portStr.c_str());
		if (!isValidIP(host))
		{
			std::cerr << "Error: Invalid host address: " << host << std::endl;
			exit(1);
		}
	}
	else
	{
		if (!isNumber(listenValue))
		{
			std::cerr << "Error: Invalid port number: " << listenValue << std::endl;
			exit(1);
		}
		port = atoi(listenValue.c_str());
		host = "";
	}

	if (port < 1 || port > 65535)
	{
		std::cerr << "Error: Port number out of range: " << port << std::endl;
		exit(1);
	}
}

void ServerConfig::handleHost(std::istringstream& iss, std::string line)
{
	iss >> host;
	host = cleanValue(host);
	if (countWords(line) != 2)
		exit(1);
	if (host.empty() || !isValidIP(host))
	{
		std::cerr << "Error: Invalid host address: " << host << std::endl;
		exit(1);
	}
}

void ServerConfig::handleServerName(std::istringstream& iss, std::string line)
{
	if (countWords(line) != 2)
		exit(1);
	iss >> server_name;
	server_name = cleanValue(server_name);
}

void ServerConfig::handleIndex(std::istringstream& iss)
{
	iss >> index;
	index = cleanValue(index);
	std::string fullPath = root + "/" + index;
	if (!isFileValid(fullPath))
	{
		std::cerr << "Warning: Default index file does not exist: " << fullPath << std::endl;
	}
}

void ServerConfig::handleRoot(std::istringstream& iss)
{
	iss >> root;
	root = cleanValue(root);
	if (!isPathValid(root))
	{
		std::cerr << "Error: Invalid root path: " << root << std::endl;
		exit(1);
	}
}

void ServerConfig::handleClientMaxBodySize(std::istringstream& iss, std::string line)
{
	std::string size_str;
	iss >> size_str;
	size_str = cleanValue(size_str);
	
	if (countWords(line) != 2)
		exit(1);
	if (size_str.empty())
	{
		std::cerr << "Error: client_max_body_size is empty" << std::endl;
		exit(1);
	}

	char unit = size_str[size_str.size() - 1];
	std::string numberPart = size_str.substr(0, size_str.size() - 1);

	int multiplier = 1;
	if (unit == 'K' || unit == 'k') multiplier = 1024;
	else if (unit == 'M' || unit == 'm') multiplier = 1024 * 1024;
	else if (unit == 'G' || unit == 'g') multiplier = 1024 * 1024 * 1024;
	else if (isdigit(unit)) numberPart += unit;
	else
	{
		std::cerr << "Error: Invalid client_max_body_size unit: " << unit << std::endl;
		exit(1);
	}

	if (!isNumber(numberPart))
	{
		std::cerr << "Error: Invalid client_max_body_size: " << size_str << std::endl;
		exit(1);
	}

	client_max_body_size = atoi(numberPart.c_str()) * multiplier;
}

void ServerConfig::handleErrorPage(std::istringstream& iss)
{
	std::string code_str, page;
	iss >> code_str >> page;
	code_str = cleanValue(code_str);
	page = cleanValue(page);

	if (!isNumber(code_str) || !isHttpErrorCodeValid(atoi(code_str.c_str())))
	{
		std::cerr << "Error: Invalid error code: " << code_str << std::endl;
		exit(1);
	}

	error_pages[atoi(code_str.c_str())] = page;
}

void ServerConfig::handleLocation(std::istringstream& iss, std::ifstream& configFile, std::string line)
{
	LocationConfig location;
	iss >> location.path;
	location.path = cleanValue(location.path);
	line = cleanForLoc(line);
	if (countWords(line) != 2)
		exit(1);
	location.parseLocation(configFile, location);

	locations.push_back(location);
}

//-------------------------------LOCATION HANDLER----------------------------------------//

void LocationConfig::handleLocRoot(std::istringstream &iss, LocationConfig& location, std::string line){

	iss	>> location.root;
	location.root =	cleanValue(location.root);
	if (countWords(line) != 2)
		exit(1);
	if (!isPathValid(location.root))
	{
		std::cerr << "Error: Invalid root path:	" << location.root << std::endl;
		exit(1);
	}
}

void LocationConfig::handleLocIndex(std::istringstream &iss, LocationConfig& location, std::string line){
	iss	>> location.index;
	if (countWords(line) != 2)
		exit (1);
	location.index = cleanValue(location.index);
	std::string	fullPath = location.root + "/" + location.index;
	if (!isFileValid(fullPath))
	{
		std::cerr << "Warning: Default index file does not exist: "	<< fullPath	<< std::endl;
	}
}

void LocationConfig::handleLocAllMethods(std::istringstream &iss, LocationConfig& location){
	std::string	method;
	while (iss >> method){
		method = cleanValue(method);
		if (method != "GET" && method != "POST" && method != "DELETE")
			exit(1);
		location.allow_methods.push_back(cleanValue(method));
	}
	(void)iss, (void)location;
}

void LocationConfig::handleAutoIndex(std::istringstream &iss, LocationConfig& location, std::string line){
	std::string	value;
	iss	>> value;
	value =	cleanValue(value);
	if (countWords(line) != 2)
		exit (1);
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

void LocationConfig::handleCGI(std::istringstream &iss, LocationConfig& location){
	(void)iss;(void)location;
}

// bool isNotUrl(std::string value)
// {

// }

void LocationConfig::handleReturn(std::istringstream &iss, LocationConfig& location, std::string line){
	std::string value;
	iss >> value;
	if (countWords(line) != 2)
		exit(1);
	// else if (isNotUrl(value))
	// 	exit(1);
	(void)location;
}
//-------------------------------PARSE FUNCTIONS---------------------------------------//


void LocationConfig::parseLocation(std::ifstream& configFile, LocationConfig& location)
{
	std::set<std::string> usedKeys;
	std::string	line;
	int i = 0;
	while (std::getline(configFile,	line))
	{
		i++;
		std::istringstream iss(line);
		checkPV(line);
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
			handleLocRoot(iss, location, line);
		else if (key == "index")
			handleLocIndex(iss, location, line);
		else if (key == "allow_methods")
			handleLocAllMethods(iss, location);
		else if (key == "autoindex")
			handleAutoIndex(iss, location, line);
		else if (key == "CGI")
			handleCGI(iss, location);
		else if (key == "return")
			handleReturn(iss, location, line);
		else
		{
			std::cerr << "Error: Unknown directive in location block: "	<< key << std::endl;
			exit(1);
		}
	}
}

void ServerConfig::parseServer(std::ifstream& configFile)
{
	std::string line;
	std::set<std::string> usedKeys;

	while (std::getline(configFile, line))
	{
		std::istringstream iss(line);
		checkPV(line);
		std::string key;
		iss >> key;
		if (key.empty()) continue;
		if (key == "}") break;

		if (key != "location" && usedKeys.find(key) != usedKeys.end())
		{
			std::cerr << "Error: Duplicate directive in server block: " << key << std::endl;
			exit(1);
		}
		usedKeys.insert(key);
		if (key == "listen")
			handleListen(iss, line);
		else if (key == "host")
			handleHost(iss, line);
		else if (key == "server_name")
			handleServerName(iss, line);
		else if (key == "index")
			handleIndex(iss);
		else if (key == "root")
			handleRoot(iss);
		else if (key == "client_max_body_size")
			handleClientMaxBodySize(iss, line);
		else if (key == "error_page")
			handleErrorPage(iss);
		else if (key == "location")
			handleLocation(iss, configFile, line);
		else
		{
			std::cerr << "Error: Unknown directive in server block: " << key << std::endl;
			exit(1);
		}
	}
}


void Config::parseConfig(const std::string& filename)
{
	std::ifstream configFile(filename.c_str());
	if (!configFile.is_open())
	{
		std::cerr << "Error: Could not open	config file: " << filename << std::endl;
		exit(1);
	}
	std::string	line;
	while (std::getline(configFile,	line))
	{
		std::istringstream iss(line);
		std::string	key;
		iss	>> key;
		if (key.empty()) 
			continue;
		if (key	== "server")
		{
			ServerConfig server;
			server.parseServer(configFile);
			servers.push_back(server);
		}
		else
		{
			std::cerr << "Error: Unknown directive outside any block: "	<< key << std::endl;
			exit(1);
		}
	}
}
