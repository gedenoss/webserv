#ifndef CONFIG_HPP
# define CONFIG_HPP


#include <string>
#include <map>
#include <vector>
#include <iostream>

class LocationConfig {
	private:
		std::string path;                  
		std::vector<std::string> allow_methods; 
		std::string root;                  
		std::string index;                
		bool autoindex;                    
		std::string upload_dir;            
		std::string cgi_extension;         
		std::string cgi_path;
		
	public :
		// ~LocationConfig();
		// LocationConfig() : autoindex(false) {}
		void parseLocation(std::ifstream& configFile, LocationConfig& location);
		std::string getPath() const { return path; };
		std::string getRoot() const { return root; };
		bool getAutoindex() const { return autoindex; };
		std::vector<std::string> getAllowMethod() const { return allow_methods; };
		//---------------adem------------------//
		void handleLocRoot(std::istringstream &iss, LocationConfig& location);
		void handleLocIndex(std::istringstream &iss, LocationConfig& location);
		void handleLocAllMethods(std::istringstream &iss, LocationConfig& location);
		void handleAutoIndex(std::istringstream &iss, LocationConfig& location);
		void handleCGI(std::istringstream &iss, LocationConfig& location);
		//------------plus adem----------------//

	friend class ServerConfig;
};

class ServerConfig {
	private:
		int port;  
		std::string host;
		std::string index;                       
		std::string server_name;           
		std::string root;                  
		size_t client_max_body_size;       
		std::map<int, std::string> error_pages; 
		std::vector<LocationConfig> locations; 

	public:
		void parseServer(std::ifstream&	configFile);
		//------------adem--------------//
		void handleListen(std::istringstream& iss, std::string line);
		void handleHost(std::istringstream& iss);
		void handleServerName(std::istringstream& isse);
		void handleIndex(std::istringstream& iss);
		void handleRoot(std::istringstream& iss);
		void handleClientMaxBodySize(std::istringstream& iss);
		void handleErrorPage(std::istringstream& iss);
		void handleLocation(std::istringstream& line, std::ifstream& configFile);
		//----------plus adem-----------//

		int getPort();
		std::vector<LocationConfig> getLocations() const;
};

class Config {
	private:
		std::vector<ServerConfig> servers;
		
	public:
		void parseConfig(const std::string& filename);
		std::vector<ServerConfig> getServers() const;
};


#endif