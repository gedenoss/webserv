#ifndef CONFIG_HPP
# define CONFIG_HPP


#include <string>
#include <map>
#include <vector>
#include <iostream>

struct LocationConfig {
	std::string path;                  
	std::vector<std::string> allow_methods; 
	std::string root;                  
	std::string index;                
	bool autoindex;                    
	std::string upload_dir;            
	std::string cgi_extension;         
	std::string cgi_path;
	
	LocationConfig() : autoindex(false) {}
};

struct ServerConfig {
	int port;  
	std::string host;
	std::string index;                       
	std::string server_name;           
	std::string root;                  
	size_t client_max_body_size;       
	std::map<int, std::string> error_pages; 
	std::vector<LocationConfig> locations; 
};

struct Config {
	std::vector<ServerConfig> servers; 
};

Config parseConfig(const std::string& filename);

#endif