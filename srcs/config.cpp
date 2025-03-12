#include "../includes/webserv.hpp"
#include <cstdlib> 
#include <sys/stat.h> 


bool isPathValid(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}


bool isFileValid(const std::string& filePath) {
    struct stat info;
    return (stat(filePath.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}


bool isHttpErrorCodeValid(int code) {
    return (code >= 400 && code <= 599); 
}


void parseLocation(std::ifstream& configFile, LocationConfig& location) {
    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key == "}") {
            break; 
        } else if (key == "allow_methods") {
            std::string method;
            while (iss >> method) {
                if (!method.empty() && method.back() == ';') {
                    method.pop_back(); 
                }
                location.allow_methods.push_back(method);
            }
        } else if (key == "root") {
            std::string root;
            iss >> root;
            if (!root.empty() && root.back() == ';') {
                root.pop_back(); 
            }
            if (!isPathValid(root)) {
                std::cerr << "Error: Invalid root path: " << root << std::endl;
                exit(1);
            }
            location.root = root;
        } else if (key == "index") {
            std::string index;
            iss >> index;
            if (!index.empty() && index.back() == ';') {
                index.pop_back(); 
            }
            location.index = index;
        } else if (key == "autoindex") {
            std::string value;
            iss >> value;
            if (!value.empty() && value.back() == ';') {
                value.pop_back(); 
            }
            location.autoindex = (value == "on");
        } else if (key == "upload_dir") {
            std::string upload_dir;
            iss >> upload_dir;
            if (!upload_dir.empty() && upload_dir.back() == ';') {
                upload_dir.pop_back(); 
            }
            if (!isPathValid(upload_dir)) {
                std::cerr << "Error: Invalid upload directory: " << upload_dir << std::endl;
                exit(1);
            }
            location.upload_dir = upload_dir;
        } else if (key == "cgi_extension") {
            std::string cgi_extension;
            iss >> cgi_extension;
            if (!cgi_extension.empty() && cgi_extension.back() == ';') {
                cgi_extension.pop_back(); 
            }
            location.cgi_extension = cgi_extension;
        } else if (key == "cgi_path") {
            std::string cgi_path;
            iss >> cgi_path;
            if (!cgi_path.empty() && cgi_path.back() == ';') {
                cgi_path.pop_back(); 
            }
            if (!isFileValid(cgi_path)) {
                std::cerr << "Error: Invalid CGI path: " << cgi_path << std::endl;
                exit(1);
            }
            location.cgi_path = cgi_path;
        }
    }
}


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
            if (!port_str.empty() && port_str.back() == ';') {
                port_str.pop_back(); 
            }
            
            for (size_t i = 0; i < port_str.length(); ++i) {
                if (!isdigit(port_str[i])) {
                    std::cerr << "Error: Invalid port number: " << port_str << std::endl;
                    exit(1);
                }
            }
            server.port = atoi(port_str.c_str());
            if (server.port < 1 || server.port > 65535) {
                std::cerr << "Error: Port number out of range: " << server.port << std::endl;
                exit(1);
            }
        } else if (key == "server_name") {
            std::string server_name;
            iss >> server_name;
            if (!server_name.empty() && server_name.back() == ';') {
                server_name.pop_back(); 
            }
            server.server_name = server_name;
        } else if (key == "root") {
            std::string root;
            iss >> root;
            if (!root.empty() && root.back() == ';') {
                root.pop_back(); 
            }
            if (!isPathValid(root)) {
                std::cerr << "Error: Invalid root path: " << root << std::endl;
                exit(1);
            }
            server.root = root;
        } else if (key == "client_max_body_size") {
            std::string size_str;
            iss >> size_str;
            if (!size_str.empty() && size_str.back() == ';') {
                size_str.pop_back(); 
            }
            
            for (size_t i = 0; i < size_str.length(); ++i) {
                if (!isdigit(size_str[i])) {
                    std::cerr << "Error: Invalid client_max_body_size: " << size_str << std::endl;
                    exit(1);
                }
            }
            server.client_max_body_size = atoi(size_str.c_str());
        } else if (key == "error_page") {
            std::string code_str, page;
            iss >> code_str >> page;
            if (!code_str.empty() && code_str.back() == ';') {
                code_str.pop_back(); 
            }
            if (!page.empty() && page.back() == ';') {
                page.pop_back(); 
            }
            int code = atoi(code_str.c_str());
            if (!isHttpErrorCodeValid(code)) {
                std::cerr << "Error: Invalid HTTP error code: " << code << std::endl;
                exit(1);
            }
            if (!isFileValid(server.root + page)) {
                std::cerr << "Error: Invalid error page path: " << page << std::endl;
                exit(1);
            }
            server.error_pages[code] = page;
        } else if (key == "location") {
            LocationConfig location;
            iss >> location.path;
            parseLocation(configFile, location);
            server.locations.push_back(location);
        }
    }
}


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
        }
    }

    return config;
}