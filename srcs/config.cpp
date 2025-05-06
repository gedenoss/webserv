#include "../includes/config.hpp"
#include "../includes/utils.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <map>
#include <set>
#include "../includes/utils.hpp"

void printError(const std::string& context, const std::string& msg) {
    std::cerr << "\033[1;31m[ERROR] " << context << ": " << msg << "\033[0m" << std::endl;
}

//------------------------------------------------CONSTRUCTORS-----------------------------------------------------//


ServerConfig::ServerConfig()
{
    server_name = "localhost";
    client_max_body_size = 1024 * 1024; // Default to 1MB
}

LocationConfig::LocationConfig()
{
    autoindex = false;
    allow_methods.push_back("GET");
    hasReturn = false;
    returnPath = "";
}

//-----------------------------------------------------GETTERS-------------------------------------------------------------//

std::vector<LocationConfig> ServerConfig::getLocations() const {
    return locations;
}

std::vector<ServerConfig> Config::getServers() const {
    return servers;
}

//-------------------------------------------------CLEAN FUNCTIONS----------------------------------------------------------//

std::string cleanValue(std::string value) {
    while (!value.empty() && (value[value.size() - 1] == ';' || isspace(value[value.size() - 1]))) 
        value.erase(value.size() - 1);
    return value;
}

std::string cleanForLoc(std::string line) {
    if (!line.empty() && line[line.size() - 1] == '{') {
        line.erase(line.size() - 1);
        return line;
    }
    return "";
}

//-----------------------------------------------------SERVER HANDLERS------------------------------------------------------//

void ServerConfig::handleListen(std::istringstream& iss, std::string line) {
    std::string listenValue;
    iss >> listenValue;
    if (countWords(line) != 2) {
        printError("handleListen", "Wrong number of arguments for 'listen'. Expected 1.");
        exit(1);
    }
    listenValue = cleanValue(listenValue);
    if (listenValue.empty()) {
        printError("handleListen", "Missing value for 'listen'.");
        exit(1);
    }
    size_t colonPos = listenValue.find(':');
    if (colonPos != std::string::npos) {
        host = listenValue.substr(0, colonPos);
        std::string portStr = listenValue.substr(colonPos + 1);
        if (!isNumber(portStr)) {
            printError("handleListen", "Port must be a number: " + portStr);
            exit(1);
        }
        port = atoi(portStr.c_str());
        if (!isValidIP(host)) {
            printError("handleListen", "Invalid IP address: " + host);
            exit(1);
        }
    } else {
        if (!isNumber(listenValue)) {
            printError("handleListen", "Port must be numeric: " + listenValue);
            exit(1);
        }
        port = atoi(listenValue.c_str());
        host = "";
    }
    if (port < 1 || port > 65535) {
        printError("handleListen", "Port out of range: " + toString(port));
        exit(1);
    }
}


void ServerConfig::handleServerName(std::istringstream& iss, std::string line) {
    if (countWords(line) != 2) {
        printError("handleServerName", "Expected one argument.");
        exit(1);
    }
    iss >> server_name;
    server_name = cleanValue(server_name);
    std::cout << "\033[1;32m[INFO] Server name: " << server_name << "\033[0m" << std::endl;
}



void ServerConfig::handleClientMaxBodySize(std::istringstream& iss, std::string line) {
    std::string size_str;
    iss >> size_str;
    size_str = cleanValue(size_str);
    if (countWords(line) != 2) {
        printError("handleClientMaxBodySize", "Expected one argument.");
        exit(1);
    }
    if (size_str.empty()) {
        printError("handleClientMaxBodySize", "Value cannot be empty.");
        exit(1);
    }
    char unit = size_str[size_str.size() - 1];
    std::string numberPart = size_str.substr(0, size_str.size() - 1);
    int multiplier = 1;
    if (unit == 'K' || unit == 'k') multiplier = 1024;
    else if (unit == 'M' || unit == 'm') multiplier = 1024 * 1024;
    else if (unit == 'G' || unit == 'g') multiplier = 1024 * 1024 * 1024;
    else if (isdigit(unit)) numberPart += unit;
    else {
        printError("handleClientMaxBodySize", "Invalid unit: " + std::string(1, unit));
        exit(1);
    }
    if (!isNumber(numberPart)) {
        printError("handleClientMaxBodySize", "Non-numeric size: " + numberPart);
        exit(1);
    }
    client_max_body_size = atoi(numberPart.c_str()) * multiplier;
}

void ServerConfig::handleErrorPage(std::istringstream& iss) {
    std::string code_str, page;
    iss >> code_str >> page;
    code_str = cleanValue(code_str);
    page = cleanValue(page);
    if (!isNumber(code_str) || !isHttpErrorCodeValid(atoi(code_str.c_str()))) {
        printError("handleErrorPage", "Invalid error code: " + code_str);
        exit(1);
    }
    error_pages[atoi(code_str.c_str())] = page;
}

void ServerConfig::handleLocation(std::istringstream& iss, std::ifstream& configFile, std::string line) {
    LocationConfig location;
    iss >> location.path;
    location.path = cleanValue(location.path);
    line = cleanForLoc(line);
    if (countWords(line) != 2) {
        printError("handleLocation", "Expected: location <path> {");
        exit(1);
    }
    location.parseLocation(configFile, location);
    locations.push_back(location);
}

//--------------------------------------------------LOCATION HANDLERS-------------------------------------------------------//

void LocationConfig::handleLocRoot(std::istringstream &iss, LocationConfig& location, std::string line) {
    iss >> location.root;
    location.root = cleanValue(location.root);
    if (countWords(line) != 2) {
        printError("handleLocRoot", "Expected one argument.");
        exit(1);
    }
    if (!isPathValid(location.root)) {
        printError("handleLocRoot", "Invalid path: " + location.root);
        exit(1);
    }
}

void LocationConfig::handleLocIndex(std::istringstream &iss, LocationConfig& location, std::string line) {
    iss >> location.index;
    location.index = cleanValue(location.index);
    if (countWords(line) != 2) {
        printError("handleLocIndex", "Expected one argument.");
        exit(1);
    }
    if (!location.root.empty()) {
        std::string fullPath = location.root + (location.root[location.root.size() - 1] == '/' ? "" : "/") + location.index;
        if (!isFileValid(fullPath)) {
            std::cerr << "\033[1;33m[WARNING] Index file not found at: " << fullPath << "\033[0m" << std::endl;
        }
    }
}

void LocationConfig::handleLocAllMethods(std::istringstream &iss, LocationConfig& location) {
    std::string method;
    while (iss >> method) {
        method = cleanValue(method);
        if (method != "GET" && method != "POST" && method != "DELETE") {
            if (isOnlyWhiteSpace(method) == 1)
                break;
            printError("handleLocAllMethods", "Unsupported method: " + method);
            exit(1);
        }
        location.allow_methods.push_back(method);
    }
}

void LocationConfig::handleAutoIndex(std::istringstream &iss, LocationConfig& location, std::string line) {
    std::string value;
    iss >> value;
    value = cleanValue(value);
    if (countWords(line) != 2) {
        printError("handleAutoIndex", "Expected one argument.");
        exit(1);
    }
    if (value == "on")
        location.autoindex = true;
    else if (value == "off")
        location.autoindex = false;
    else {
        printError("handleAutoIndex", "Expected 'on' or 'off'. Got: " + value);
        exit(1);
    }
}

void LocationConfig::handleCGI(std::istringstream &iss, LocationConfig& location, std::string line) {
    std::string ext, path;
    iss >> ext >> path;
    ext = cleanValue(ext);
    path = cleanValue(path);
    if (countWords(line) != 3) {
        printError("handleCGI", "Expected two arguments: <ext> <path>.");
        exit(1);
    }
    if (!isPathValid(path)) {
        printError("handleCGI", "Invalid CGI path: " + path);
        exit(1);
    }
    location.cgi[ext] = path;
}

void LocationConfig::handleReturn(std::istringstream &iss, LocationConfig& location, std::string line) {
    int words = countWords(line);

    if (words != 2) {
        printError("handleReturn", "Expected `return <url>`");
        exit(1);
    }
    // On stocke dans la config
    location.hasReturn = true;
    std::string url;
    iss >> url;
    url = cleanValue(url);
    location.returnPath = url;
}

//--------------------------------------------------PARSERS-----------------------------------------------------------------//

void validateLocationDirectives(const std::set<std::string>& usedKeys) {
    static std::set<std::string> requiredKeys;
    requiredKeys.insert("root");

    for (std::set<std::string>::const_iterator it = requiredKeys.begin(); it != requiredKeys.end(); ++it) {
        if (usedKeys.find(*it) == usedKeys.end()) {
            std::cerr << RED << "[ERROR] Missing required directive: " << *it << RESET <<std::endl;
            exit(1);
        }
    }
}

void LocationConfig::parseLocation(std::ifstream& configFile, LocationConfig& location) {
    std::set<std::string> usedKeys;
    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        checkPV(line);
        std::string key;
        iss >> key;
        if (key.empty() || key[0] == '#') continue;
        if (key == "}") break;
        if (usedKeys.find(key) != usedKeys.end()) {
			if (key != "cgi")
			{
				printError("parseLocation", "Duplicate directive: " + key);
            	exit(1);
			}
        }
        usedKeys.insert(key);
        if (key == "root") handleLocRoot(iss, location, line);
        else if (key == "index") handleLocIndex(iss, location, line);
        else if (key == "allow_methods") handleLocAllMethods(iss, location);
        else if (key == "autoindex") handleAutoIndex(iss, location, line);
        else if (key == "cgi") handleCGI(iss, location, line);
        else if (key == "return") handleReturn(iss, location, line);
        else {
            printError("parseLocation", "Unknown directive: " + key);
            exit(1);
        }
    }
    validateLocationDirectives(usedKeys);
}

void validateServerDirectives(const std::set<std::string>& usedKeys) {
    static std::set<std::string> requiredKeys;
    requiredKeys.insert("listen");

    for (std::set<std::string>::const_iterator it = requiredKeys.begin(); it != requiredKeys.end(); ++it) {
        if (usedKeys.find(*it) == usedKeys.end()) {
            std::cerr << RED << "[ERROR] Missing required directive: " << *it << RESET <<std::endl;
            exit(1);
        }
    }
}


void ServerConfig::parseServer(std::ifstream& configFile) {
    std::string line;
    std::set<std::string> usedKeys;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        checkPV(line);
        std::string key;
        iss >> key;
        if (key.empty() || key[0] == '#') continue;
        if (key == "}") break;
        if (key != "location" && usedKeys.find(key) != usedKeys.end()) {
            if (key != "error_page")
            {
                printError("parseServer", "Duplicate directive: " + key);
                exit(1);
            }
        }
        usedKeys.insert(key);
        if (key == "listen") handleListen(iss, line);
        else if (key == "server_name") handleServerName(iss, line);
        else if (key == "client_max_body_size") handleClientMaxBodySize(iss, line);
        else if (key == "error_page") handleErrorPage(iss);
        else if (key == "location") handleLocation(iss, configFile, line);
        else {
            printError("parseServer", "Unknown directive: " + key);
            exit(1);
        }
    }
    validateServerDirectives(usedKeys);
}


void Config::parseConfig(const std::string& filename) {
	
	if (filename.substr(filename.find_last_of(".") + 1) != "conf") {
        printError("parseConfig", "Invalid file extension. Only .conf files are allowed: " + filename);
        exit(1);
    }

    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open()) {
        printError("parseConfig", "Unable to open file: " + filename);
        exit(1);
    }
    std::string line;
    while (std::getline(configFile, line)) {
        if (line.empty() || line[0] == '#') 
        {
        } 
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key.empty() || key[0] == '#') continue;
        if (key == "server") {
            ServerConfig server;
            server.parseServer(configFile);
            servers.push_back(server);
        }
        else {
            printError("parseConfig", "Unknown directive outside server block: " + key);
            exit(1);
        }
    }
}
