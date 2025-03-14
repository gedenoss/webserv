
#include "../includes/webserv.hpp"
#include "../includes/config.hpp"
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/stat.h>

// Fonction pour vérifier si un chemin existe et est accessible
bool isPathValid(const std::string& path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

// Fonction pour vérifier si un fichier existe
bool isFileValid(const std::string& filePath)
{
    struct stat info;
    return (stat(filePath.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

// Fonction pour vérifier si un code d'erreur HTTP est valide
bool isHttpErrorCodeValid(int code)
{
    return (code >= 400 && code <= 599);
}

// Fonction pour parser un bloc "location"
void parseLocation(std::ifstream& configFile, LocationConfig& location)
{
    std::string line;
    while (std::getline(configFile, line))
  {
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key == "}")
            break;
        else if (key == "allow_methods")
        {
            std::string method;
            while (iss >> method)
            {
                if (!method.empty() && method[method.size() - 1] == ';')
                    method.erase(method.size() - 1);
                location.allow_methods.push_back(method);
            }
        }
        else if (key == "root")
        {
            std::string root;
            iss >> root;
            if (!root.empty() && root[root.size() - 1] == ';')
                root.erase(root.size() - 1);
            if (!isPathValid(root))
            {
                std::cerr << "Error: Invalid root path: " << root << std::endl;
                exit(1);
            }
            location.root = root;
        }
        else if (key == "index")
        {
            std::string index;
            iss >> index;
            if (!index.empty() && index[index.size() - 1] == ';')
                index.erase(index.size() - 1);
            location.index = index;
        }
        else if (key == "autoindex")
        {
            std::string value;
            iss >> value;
            if (!value.empty() && value[value.size() - 1] == ';')
                value.erase(value.size() - 1);
            location.autoindex = (value == "on");
        }
        else if (key == "upload_dir")
        {
            std::string upload_dir;
            iss >> upload_dir;
            if (!upload_dir.empty() && upload_dir[upload_dir.size() - 1] == ';')
                upload_dir.erase(upload_dir.size() - 1);
            if (!isPathValid(upload_dir))
            {
                std::cerr << "Error: Invalid upload directory: " << upload_dir << std::endl;
                exit(1);
            }
            location.upload_dir = upload_dir;
        }
        else if (key == "cgi_extension")
        {
            std::string cgi_extension;
            iss >> cgi_extension;
            if (!cgi_extension.empty() && cgi_extension[cgi_extension.size() - 1] == ';')
                cgi_extension.erase(cgi_extension.size() - 1);
            location.cgi_extension = cgi_extension;
        }
        else if (key == "cgi_path")
        {
            std::string cgi_path;
            iss >> cgi_path;
            if (!cgi_path.empty() && cgi_path[cgi_path.size() - 1] == ';')
                cgi_path.erase(cgi_path.size() - 1);
            if (!isFileValid(cgi_path))
            {
                std::cerr << "Error: Invalid CGI path: " << cgi_path << std::endl;
                exit(1);
            }
            location.cgi_path = cgi_path;
        }
        else 
        {
            std::cerr << "Error: Unknown directive '" << key << "' in location block." << std::endl;
            exit(1);
        }
    }
}

// Fonction pour parser un bloc "server"
void parseServer(std::ifstream& configFile, ServerConfig& server) {
    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key == "}") {
            break;
        } else if (key == "listen") {
            std::string port_str;
            iss >> port_str;
            if (!port_str.empty() && port_str[port_str.size() - 1] == ';') {
                port_str.erase(port_str.size() - 1);
            }
            char* endptr;
            long port = std::strtol(port_str.c_str(), &endptr, 10);
            if (*endptr != '\0' || port < 1 || port > 65535) {
                std::cerr << "Error: Invalid port number: " << port_str << std::endl;
                exit(1);
            }
            server.port = static_cast<int>(port);
        } else if (key == "server_name") {
            std::string server_name;
            iss >> server_name;
            if (!server_name.empty() && server_name[server_name.size() - 1] == ';') {
                server_name.erase(server_name.size() - 1);
            }
            server.server_name = server_name;
        } else if (key == "root") {
            std::string root;
            iss >> root;
            if (!root.empty() && root[root.size() - 1] == ';') {
                root.erase(root.size() - 1);
            }
            if (!isPathValid(root)) {
                std::cerr << "Error: Invalid root path: " << root << std::endl;
                exit(1);
            }
            server.root = root;
        } else if (key == "client_max_body_size") {
            std::string size_str;
            iss >> size_str;
            if (!size_str.empty() && size_str[size_str.size() - 1] == ';') {
                size_str.erase(size_str.size() - 1);
            }
            char* endptr;
            long size = std::strtol(size_str.c_str(), &endptr, 10);
            if (*endptr != '\0' || size < 0) {
                std::cerr << "Error: Invalid client_max_body_size: " << size_str << std::endl;
                exit(1);
            }
            server.client_max_body_size = static_cast<size_t>(size);
        } else if (key == "error_page") {
            std::string code_str, page;
            iss >> code_str >> page;
            int code = std::atoi(code_str.c_str());
            if (!isHttpErrorCodeValid(code)) {
                std::cerr << "Error: Invalid HTTP error code: " << code << std::endl;
                exit(1);
            }
            server.error_pages[code] = page;
        } else if (key == "location") {
            LocationConfig location;
            iss >> location.path;
            parseLocation(configFile, location);
            server.locations.push_back(location);
        } else {
            std::cerr << "Error: Unknown directive '" << key << "' in server block." << std::endl;
            exit(1);
        }
    }
}

// Fonction pour parser le fichier de configuration
Config parseConfig(const std::string& filename) {
    Config config;
    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open config file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key == "server") {
            ServerConfig server;
            parseServer(configFile, server);
            config.servers.push_back(server);
        } else {
            std::cerr << "Error: Unknown directive '" << key << "' outside of any server block." << std::endl;
            exit(1);
        }
    }

    return config;
}

