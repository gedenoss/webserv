#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/config.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>		


Request::Request() {}


Request::~Request()	{}

HttpRequestParser::HttpRequestParser(size_t	maxBody, size_t	maxHeaders)	
	: maxBodySize(maxBody),	maxHeadersSize(maxHeaders) {}

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

bool HttpRequestParser::isMethodAllowedForRoute(const std::string &method, const std::string &url, const Config	&config) {
	std::cout << "Checking method [" << method << "] for URL [" << url << "]\n";

	for	(size_t	i =	0; i < config.servers.size(); ++i) {
		const ServerConfig &server = config.servers[i];
		
		for	(size_t	j =	0; j < server.locations.size();	++j) {
			const LocationConfig &location = server.locations[j];

			std::cout << "  -> Testing Location	[" << location.path	<< "]\n";

			if (url.find(location.path)	== 0 && (url.size()	== location.path.size()	|| url[location.path.size()] == '/'	
			  || url[location.path.size()] == '?'))	{
				std::cout << "  ✅ Matched Location:	" << location.path << "\n";
				
				for	(size_t	k =	0; k < location.allow_methods.size(); ++k) {
					std::cout << "     - Allowed Method: " << location.allow_methods[k]	<< "\n";
					if (method == location.allow_methods[k]) {
						std::cout << "  ✅ Method allowed!\n";
						return true;
					}
				}
				std::cout << "  ❌ Method [" << method << "] not	allowed	for	[" << location.path	<< "]\n";
				return false;
			}
		}
	}
	std::cout << "❌ No matching	location for [" << url << "]\n";
	return false;
}

bool HttpRequestParser::isValidHttpVersion(const std::string &version) {
	return (version	== "HTTP/1.1");
}

bool HttpRequestParser::isValidUrl(const std::string &url) {
	return !url.empty()	&& url[0] == '/' && url.find("..") == std::string::npos;
}

bool HttpRequestParser::isValidMethod(const	std::string	&method) {
	
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



Request	HttpRequestParser::parse(const std::string &rawRequest,	int	&errorCode,	const Config &config) {
	Request	request;
	std::istringstream stream(rawRequest);
	std::string	line;
	bool headersFinished = false;

	
	if (rawRequest.size() >	8000) {
		errorCode =	431;	
		return request;
	}

	
	if (!std::getline(stream, line)	|| line.empty()) {
		errorCode =	400;	
		return request;
	}

	
	if (!line.empty() && line[line.size() -	1] == '\r')	{
		line.erase(line.size() - 1);
	}

	std::istringstream requestLine(line);
	std::string	method,	url, httpVersion;
	requestLine	>> method >> url >> httpVersion;

	
	if (url.length() > 8000) {
		errorCode =	414;	
		return request;
	}

	
	if (!isValidMethod(method))	{
		errorCode =	501;	
		return request;
	}

	
	if (!isValidUrl(url)) {
		errorCode =	400;	
		return request;
	}

	
	if (!isMethodAllowedForRoute(method, url, config)) {
		errorCode =	405;	
		return request;
	}

	
	if (!isValidHttpVersion(httpVersion)) {
		errorCode =	505;	
		return request;
	}

	
	request.setMethod(method);
	request.setUrl(url);
	request.setHttpVersion(httpVersion);

	
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
			return request;
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

		request.addHeader(key, value);
		headersSize	+= line.size();
		
		if (headersSize	> maxHeadersSize) {
			errorCode =	431;	
			return request;
		}
	}

	
	if (request.getHttpVersion() == "HTTP/1.1" && request.getHeaders().find("Host")	== request.getHeaders().end()) {
		errorCode =	400;	
		return request;
	}

	
	if (headersFinished) {
		std::map<std::string, std::string>::const_iterator contentLengthIt = request.getHeaders().find("Content-Length");
		
		if (contentLengthIt	!= request.getHeaders().end()) {
			
			bool conversionSuccess = false;
			size_t contentLength = safeStringToULong(contentLengthIt->second, conversionSuccess);
			
			if (!conversionSuccess)	{
				errorCode =	400;	
				return request;
			}
			
			if (contentLength >	maxBodySize) {
				errorCode =	413;	
				return request;
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
				request.setBody(bodyContent);
				
				
				if (bodyContent.length() != contentLength) {
					std::cout << "Warning: Body	length (" << bodyContent.length() 
							  << ") does not match Content-Length (" << contentLength << ")" << std::endl;
					errorCode =	400;	
					return request;
				}
			}
		} else if (method == "POST") {
			
			errorCode =	411;	
			return request;
		}

		
		// if ((method	== "POST") && 
		// 	request.getHeaders().find("Content-Type") == request.getHeaders().end()) {
		// 	errorCode =	400;	
		// 	return request;
		// }

		
		std::map<std::string, std::string>::const_iterator contentTypeIt = request.getHeaders().find("Content-Type");
		if (contentTypeIt != request.getHeaders().end()) {
			std::string	contentType	= contentTypeIt->second;
			if (contentType.find("text/html") == std::string::npos && 
				contentType.find("image/png") == std::string::npos && 
				contentType.find("image/jpeg") == std::string::npos	&&
				contentType.find("application/x-www-form-urlencoded") == std::string::npos &&
				contentType.find("multipart/form-data")	== std::string::npos) {
				errorCode =	415;	
				return request;
			}
		}

		
		std::map<std::string, std::string>::const_iterator expectIt	= request.getHeaders().find("Expect");
		if (expectIt != request.getHeaders().end())	{
			std::string	expect = expectIt->second;
			if (expect.find("100-continue")	!= std::string::npos) {
				errorCode =	417;	
				return request;
			}
		}
	} else {
		
		errorCode =	400;	
		return request;
	}

	errorCode =	200;	
	return request;
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
