#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/config.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>	

bool Request::isMethodAllowedForRoute(Config &config) {
    //std::cout << "Checking method [" << _method << "] for URL [" << _url << "]\n";

	std::string url = getUrl();
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash != std::string::npos)
        url = url.substr(0, lastSlash);
    std::cout << "URL: " << url << std::endl;
    if (url.empty())
    {
        url = "/";
    }
	const std::vector<ServerConfig> &servers = config.getServers();
    for (size_t i = 0; i < config.getServers().size(); ++i) {
        const ServerConfig &server = servers[i];
        const std::vector<LocationConfig>& locations = server.getLocations();
        for (size_t j = 0; j < locations.size(); ++j) {
            const LocationConfig &location = locations[j];
            std::cout << "  -> Testing Location [" << location.getPath() << "]\n";
            if (url.find(location.getPath()) == 0 && 
                (url.size() == location.getPath().size() || 
                 url[location.getPath().size()] == '/' || 
                 url[location.getPath().size()] == '?')) {
                //std::cout << "  ✅ Matched Location: " << location.getPath() << "\n";
                _location = location;


                std::cout << "  -> _location path: " << _location.getPath() << "\n";
                std::cout << "  -> _location root: " << _location.getRoot() << "\n";
                std::cout << "  -> _location autoindex: " << (_location.getAutoindex() ? "true" : "false") << "\n";
                std::cout << "  -> _location allowed methods: ";
                std::cout << "  -> _location allowed methods: ";
                std::vector<std::string> allowed_methods = _location.getAllowMethod();
                for (std::vector<std::string>::const_iterator it = allowed_methods.begin(); it != allowed_methods.end(); ++it) {
                    std::cout << *it << " ";
                }
                std::cout << "\n";

                
                for (size_t k = 0; k < location.getAllowMethod().size(); ++k) {
                    //std::cout << "     - Allowed Method: " << location.getAllowMethod()[k] << "\n";
                    if (_method == location.getAllowMethod()[k]) {
                    //    std::cout << "  ✅ Method allowed!\n";
                        return true;
                    }
                }
                //std::cout << "  ❌ Method [" << _method << "] not allowed for [" << location.getPath() << "]\n";
                
                return false;
            }
        }
    }

    // std::cout << "❌ No matching location for [" << url << "]\n";
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
