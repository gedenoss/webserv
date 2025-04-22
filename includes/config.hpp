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
		std::map<std::string, std::string> cgi;
		
	public :
		// ~LocationConfig();
		LocationConfig() : autoindex(false) {}
		void parseLocation(std::ifstream& configFile, LocationConfig& location);
		std::string getPath() const { return path; };
		std::string getRoot() const { return root; };
		bool getAutoindex() const { return autoindex; };
		std::string getIndex() const { return index; };
		const std::map<std::string, std::string>& getCgi() const { return cgi; };
		std::vector<std::string> getAllowMethod() const { return allow_methods; };
		//---------------adem------------------//
		void handleLocRoot(std::istringstream &iss, LocationConfig& location, std::string line);
		void handleLocIndex(std::istringstream &iss, LocationConfig& location, std::string line);
		void handleLocAllMethods(std::istringstream &iss, LocationConfig& location);
		void handleAutoIndex(std::istringstream &iss, LocationConfig& location, std::string line);
		void handleCGI(std::istringstream &iss, LocationConfig& location, std::string line);
		void handleReturn(std::istringstream &iss, LocationConfig& location, std::string line);
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
		std::vector<std::string> _allowMethods;

	public:
		void parseServer(std::ifstream&	configFile);
		//------------adem--------------//
		void handleListen(std::istringstream& iss, std::string line);
		void handleHost(std::istringstream& iss, std::string line);
		void handleServerName(std::istringstream& iss, std::string line);
		void handleIndex(std::istringstream& iss);
		void handleRoot(std::istringstream& iss);
		void handleClientMaxBodySize(std::istringstream& iss, std::string line);
		void handleErrorPage(std::istringstream& iss);
		void handleLocation(std::istringstream& iss, std::ifstream& configFile, std::string line);
		//----------plus adem-----------//

		int getPort()const { return port; };
		std::string getIndex() { return index; };
		std::string getServerName() { return server_name; };
		std::vector<LocationConfig> getLocations() const;
		const std::vector<std::string>& getAllowMethod() const;
};

class Config {
	private:
		std::vector<ServerConfig> servers;
		
	public:
		void parseConfig(const std::string& filename);
		std::vector<ServerConfig> getServers() const;
};


#endif