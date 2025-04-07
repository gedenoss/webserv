#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/config.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>	

// bool Request::isMethodAllowedForRoute(Config &config) {
//     std::cout << "Checking method [" << _method << "] for URL [" << _url << "]\n";

// 	std::string url = getUrl();
//     size_t lastSlash = url.find_last_of('/');
//     if (lastSlash != std::string::npos)
//         url = url.substr(0, lastSlash);
//     if (url.empty())
//     {
//         url = "/";
//     }
// 	const std::vector<ServerConfig> &servers = config.getServers();
//     for (size_t i = 0; i < config.getServers().size(); ++i) {
//         const ServerConfig &server = servers[i];
//         const std::vector<LocationConfig>& locations = server.getLocations();
//         for (size_t j = 0; j < locations.size(); ++j) {
//             const LocationConfig &location = locations[j];
//             if (url.find(location.getPath()) == 0 && 
//                 (url.size() == location.getPath().size() || 
//                  url[location.getPath().size()] == '/' || 
//                  url[location.getPath().size()] == '?')) {
//                 _location = location;    
//                 for (size_t k = 0; k < location.getAllowMethod().size(); ++k) {
//                     if (_method == location.getAllowMethod()[k]) {
//                         return true;
//                     }
//                 }
//                 return false;
//             }
//         }
//     }

//     std::cout << "❌ No matching location for [" << url << "]\n";
//     return false;
// }
bool Request::isMethodAllowedForRoute(Config &config) {
    std::cout << "Checking method [" << _method << "] for URL [" << _url << "]\n";

    std::string url = getUrl();

    std::string originalUrl = url;

    bool locationFound = false;
    const std::vector<ServerConfig> &servers = config.getServers();

    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerConfig &server = servers[i];
        const std::vector<LocationConfig>& locations = server.getLocations();
        
        for (size_t j = 0; j < locations.size(); ++j) {
            const LocationConfig &location = locations[j];
            
            if (url.find(location.getPath()) == 0 && 
                (url.size() == location.getPath().size() || 
                 url[location.getPath().size()] == '/' || 
                 url[location.getPath().size()] == '?')) {
                
                locationFound = true;
                _location = location;

                for (size_t k = 0; k < location.getAllowMethod().size(); ++k) {
                    if (_method == location.getAllowMethod()[k]) {
                        return true;
                    }
                }
                return false;
            }
        }
    }
    if (!locationFound) {
        size_t lastSlash;
        std::string parentUrl = originalUrl;
        
        while ((lastSlash = parentUrl.find_last_of('/')) != std::string::npos) {
            parentUrl = parentUrl.substr(0, lastSlash);
            if (parentUrl.empty()) parentUrl = "/";
            
            for (size_t i = 0; i < servers.size(); ++i) {
                const ServerConfig &server = servers[i];
                const std::vector<LocationConfig>& locations = server.getLocations();
                
                for (size_t j = 0; j < locations.size(); ++j) {
                    const LocationConfig &location = locations[j];
                    
                    if (parentUrl == location.getPath()) {
                        locationFound = true;
                        _location = location;
                        
                        for (size_t k = 0; k < location.getAllowMethod().size(); ++k) {
                            if (_method == location.getAllowMethod()[k]) {
                                return true;
                            }
                        }
                        return false;
                    }
                }
            }
            
            if (parentUrl == "/") break;
        }
    }

    std::cout << "❌ No matching location for [" << url << "]\n";
    return false;
}

bool Request::isValidHttpVersion() {
	return (_httpVersion	== "HTTP/1.1");
}

bool Request::isValidUrl() {
	return !_url.empty()	&& _url[0] == '/' && _url.find("..") == std::string::npos;
}

bool Request::isValidMethod() {
	return (_method == "GET"	|| _method == "POST"	|| _method == "DELETE" ||
			_method == "PUT"	|| _method == "PATCH" || _method == "HEAD" ||
			_method == "OPTIONS"	|| _method == "CONNECT" || _method == "TRACE");
}

size_t Request::safeStringToUlong(const std::string&	str, bool& success) {
	char* end;
	unsigned long result = strtoul(str.c_str(),	&end, 10);
	
	
	if (end	== str.c_str() || *end != '\0')	{
		success	= false;	

		return 0;
	}
	
	success	= true;
	return static_cast<size_t>(result);
}
