#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/config.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>		


Request::Request(size_t	maxBody, size_t	maxHeaders) :maxBodySize(maxBody),	maxHeadersSize(maxHeaders) {}


Request::~Request()	{}


std::string	Request::getMethod() const { return	method;	}
std::string	Request::getUrl() const	{ return url; }
std::string	Request::getHttpVersion() const	{ return httpVersion; }
std::string	Request::getBody() const { return body;	}
std::map<std::string, std::string>& Request::getHeaders() { return headers; }
const std::map<std::string, std::string>& Request::getHeaders() const { return headers; }


void Request::setMethod(const std::string& m) {	method = m; }
void Request::setUrl(const std::string&	u) { url = u; }
void Request::setHttpVersion(const std::string&	v) { httpVersion = v; }
void Request::setBody(const	std::string& b) { body = b; }
void Request::addHeader(const std::string& key,	const std::string& value) {	headers[key] = value; }

bool Request::isMethodAllowedForRoute(Config &config) {
    std::cout << "Checking method [" << method << "] for URL [" << url << "]\n";

	const std::vector<ServerConfig> &servers = config.getServers();
    for (size_t i = 0; i < config.getServers().size(); ++i) {
        // Accès à un serveur spécifique
        const ServerConfig &server = servers[i];

        // Récupération des locations pour ce serveur
        const std::vector<LocationConfig>& locations = server.getLocations();  // Accède aux locations pour ce serveur

        // Vérification des locations
        for (size_t j = 0; j < locations.size(); ++j) {
            const LocationConfig &location = locations[j];  // Accède à chaque location

            std::cout << "  -> Testing Location [" << location.getPath() << "]\n";

            // Vérification du chemin URL par rapport à la location
            if (url.find(location.getPath()) == 0 && 
                (url.size() == location.getPath().size() || 
                 url[location.getPath().size()] == '/' || 
                 url[location.getPath().size()] == '?')) {
                std::cout << "  ✅ Matched Location: " << location.getPath() << "\n";

                // Vérification des méthodes autorisées
                for (size_t k = 0; k < location.getAllowMethod().size(); ++k) {
                    std::cout << "     - Allowed Method: " << location.getAllowMethod()[k] << "\n";
                    if (method == location.getAllowMethod()[k]) {
                        std::cout << "  ✅ Method allowed!\n";
                        return true;
                    }
                }
                std::cout << "  ❌ Method [" << method << "] not allowed for [" << location.getPath() << "]\n";
                return false;
            }
        }
    }

    std::cout << "❌ No matching location for [" << url << "]\n";
    return false;
}


bool Request::isValidHttpVersion() {
	return (httpVersion	== "HTTP/1.1");
}

bool Request::isValidUrl() {
	return !url.empty()	&& url[0] == '/' && url.find("..") == std::string::npos;
}

bool Request::isValidMethod() {
	
	return (method == "GET"	|| method == "POST"	|| method == "DELETE" ||
			method == "PUT"	|| method == "PATCH" || method == "HEAD" ||
			method == "OPTIONS"	|| method == "CONNECT" || method == "TRACE");
}




size_t safeStringToULong(const std::string&	str, bool& success)	{
	char* end;
	unsigned long result = strtoul(str.c_str(),	&end, 10);
	
	
	if (end	== str.c_str() || *end != '\0')	{
		success	= false;
		return 0;
	}
	
	success	= true;
	return static_cast<size_t>(result);
}



/*--------------------------------------------------------------------------------------------------------------------------*/



void Request::parse(const std::string &rawRequest,	Config &config) {
	std::istringstream stream(rawRequest);
	std::string	line;
	bool headersFinished = false;

	if (rawRequest.size() >	8000) {
		errorCode =	431;	
		return;
	}

	
	if (!std::getline(stream, line)	|| line.empty()) {
		errorCode =	400;	
		return;
	}

	
	if (!line.empty() && line[line.size() -	1] == '\r')	{
		line.erase(line.size() - 1);
	}

	std::istringstream requestLine(line);
	std::string	method,	url, httpVersion;
	requestLine	>> method >> url >> httpVersion;
	std::cout << method << std::endl;

	if (url.length() > 8000) {
		errorCode =	414;	
		return;
	}

	
	if (isValidMethod())	{
		errorCode =	501;	
		return;
	}

	
	if (isValidUrl()) {
		errorCode =	400;	
		return;
	}

	
	if (isMethodAllowedForRoute(config)) {
		errorCode =	405;	
		return;
	}

	
	if (isValidHttpVersion()) {
		errorCode =	505;	
		return;
	}

	std::cout << "ok" << std::endl;
	setMethod(method);
	setUrl(url);
	setHttpVersion(httpVersion);

	
	size_t headersSize = 0;
	while (std::getline(stream,	line)) {
		
		if (!line.empty() && line[line.size() -	1] == '\r')	{
			line.erase(line.size() - 1);
		}
		
		
		if (line.empty()) {
			headersFinished	= true;
			break;
		}
		
		size_t colonPos	= line.find(':');
		if (colonPos == std::string::npos) {
			
			errorCode =	400;	
			return;
		}

		std::string	key	= line.substr(0, colonPos);
		std::string	value =	line.substr(colonPos + 1);
		
		
		if (!key.empty()) {
			size_t lastNonSpace	= key.find_last_not_of(" \t\r");
			if (lastNonSpace != std::string::npos)
				key.erase(lastNonSpace + 1);
			else
				key.clear();	
		}
		
		if (!value.empty())	{
			size_t firstNonSpace = value.find_first_not_of(" \t");
			if (firstNonSpace != std::string::npos)
				value.erase(0, firstNonSpace);
			else
				value.clear();	
		}
		
		
		if (!value.empty() && value[value.size() - 1] == '\r') {
			value.erase(value.size() - 1);
		}

		addHeader(key, value);
		headersSize	+= line.size();
		
		if (headersSize	> maxHeadersSize) {
			errorCode =	431;	
			return;
		}
	}

	
	if (getHttpVersion() == "HTTP/1.1" && getHeaders().find("Host")	== getHeaders().end()) {
		errorCode =	400;	
		return ;
	}

	
	if (headersFinished) {
		std::map<std::string, std::string>::const_iterator contentLengthIt = getHeaders().find("Content-Length");
		
		if (contentLengthIt	!= getHeaders().end()) {
			
			bool conversionSuccess = false;
			size_t contentLength = safeStringToULong(contentLengthIt->second, conversionSuccess);
			
			if (!conversionSuccess)	{
				errorCode =	400;	
				return ;
			}
			
			if (contentLength >	maxBodySize) {
				errorCode =	413;	
				return ;
			}

			if (contentLength >	0) {
				std::string	bodyContent;
				bodyContent.reserve(contentLength);
				
				
				char* buffer = new char[1024];
				size_t totalRead = 0;
				size_t bytesToRead;
				
				while (totalRead < contentLength) {
					bytesToRead	= (contentLength - totalRead < 1024) ? (contentLength -	totalRead) : 1024;
					stream.read(buffer,	bytesToRead);
					size_t bytesRead = stream.gcount();
					
					if (bytesRead == 0) break;	
					
					bodyContent.append(buffer, bytesRead);
					totalRead += bytesRead;
				}
				
				delete[] buffer;
				setBody(bodyContent);
				
				
				if (bodyContent.length() != contentLength) {
					std::cout << "Warning: Body	length (" << bodyContent.length() 
							  << ") does not match Content-Length (" << contentLength << ")" << std::endl;
					errorCode =	400;	
					return;
				}
			}
		} else if (method == "POST") {
			
			errorCode =	411;	
			return;
		}

		
		// if ((method	== "POST") && 
		// 	getHeaders().find("Content-Type") == getHeaders().end()) {
		// 	errorCode =	400;	
		// 	return ;
		// }

		
		std::map<std::string, std::string>::const_iterator contentTypeIt = getHeaders().find("Content-Type");
		if (contentTypeIt != getHeaders().end()) {
			std::string	contentType	= contentTypeIt->second;
			if (contentType.find("text/html") == std::string::npos && 
				contentType.find("image/png") == std::string::npos && 
				contentType.find("image/jpeg") == std::string::npos	&&
				contentType.find("application/x-www-form-urlencoded") == std::string::npos &&
				contentType.find("multipart/form-data")	== std::string::npos) {
				errorCode =	415;	
				return ;
			}
		}

		
		std::map<std::string, std::string>::const_iterator expectIt	= getHeaders().find("Expect");
		if (expectIt != getHeaders().end())	{
			std::string	expect = expectIt->second;
			if (expect.find("100-continue")	!= std::string::npos) {
				errorCode =	417;	
				return ;
			}
		}
	} else {
		
		errorCode =	400;	
		return ;
	}

	errorCode =	200;	
	return;
}


/*----------------------------------------------------------------------------------------------------------------------------------*/



void Request::printRequest() const {
	std::cout << "Method: "	<< getMethod() << "\n"
			  << "URL: " << getUrl() << "\n"
			  << "HTTP Version:	" << getHttpVersion() << "\n"
			  << "Headers:\n";
	
	
	const std::map<std::string,	std::string>& headersRef = getHeaders();	
	for	(std::map<std::string, std::string>::const_iterator	it = headersRef.begin(); it != headersRef.end(); ++it)
	{
			std::cout << "  " << it->first << ": " << it->second << "\n";
	}

	std::cout << "Body:	" << (getBody().empty()	? "[empty]"	: getBody()) << "\n";
}
