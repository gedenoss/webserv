#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/config.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>		


Request::Request(size_t	maxBody, size_t	maxHeaders) :_maxBodySize(maxBody),	_maxHeadersSize(maxHeaders){}

Request::~Request()	{}

int Request::getErrorCode() const { return _errorCode;};
std::string	Request::getMethod() const { return	_method;	}
std::string	Request::getUrl() const	{ return _url; }
std::string	Request::getHttpVersion() const	{ return _httpVersion; }
std::string	Request::getBody() const { return _body;	}
std::map<std::string, std::string>& Request::getHeaders() { return _headers; }
const std::map<std::string, std::string>& Request::getHeaders() const { return _headers; }


void Request::setMethod(const std::string& m) {	_method = m; }
void Request::setUrl(const std::string&	u) { _url = u; }
void Request::setHttpVersion(const std::string&	v) { _httpVersion = v; }
void Request::setBody(const	std::string& b) { _body = b; }
void Request::addHeader(const std::string& key,	const std::string& value) {	_headers[key] = value; }


/*--------------------------------------------------------------------------------------------------------------------------*/



void Request::parse(const std::string &rawRequest,	Config &config) {
	std::istringstream stream(rawRequest);
	std::string	line;
	(void)config;
	bool headersFinished = false;

	if (rawRequest.size() >	8000) {
		_errorCode = 431;	
		return;
	}

	
	if (!std::getline(stream, line)	|| line.empty()) {
		_errorCode = 400;	
		return;
	}

	
	if (!line.empty() && line[line.size() -	1] == '\r')	{
		line.erase(line.size() - 1);
	}

	std::istringstream requestLine(line);
	std::string	method,	url, httpVersion, queryString;
	requestLine	>> method >> url >> httpVersion;

	if (url.length() > 8000) {
		_errorCode = 414;	
		return;
	}

	if (url.find("?") != std::string::npos) {
		url = url.substr(0, url.find("?"));
		queryString = url.substr(url.find("?") + 1);
	}
	setMethod(method);
	setUrl(url);
	setHttpVersion(httpVersion);
	setQueryString(queryString);

	if (!isValidMethod())
	{
		_errorCode = 400;	
		return;
	}
	else if(method != "GET"	&& method != "POST"	&& method != "DELETE"){
			_errorCode = 501;
			return;
	}

	if (!isValidUrl()) {
				_errorCode = 400;	
		return;
	}


	if (!isMethodAllowedForRoute(config)) {
		_errorCode = 405;	
		return;
	}

	
	if (!isValidHttpVersion()) {
		_errorCode = 505;	
		return;
	}
	

	//le petit header bien kawaii bien sexy
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
			
			_errorCode =	400;	
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
		
		if (headersSize	> _maxHeadersSize) {
			_errorCode = 431;	
			return;
		}
	}

	
	if (getHttpVersion() == "HTTP/1.1" && getHeaders().find("Host")	== getHeaders().end()) {
		_errorCode = 400;	
		return ;
	}

	
	if (headersFinished) {
		std::map<std::string, std::string>::const_iterator contentLengthIt = getHeaders().find("Content-Length");
		
		if (contentLengthIt	!= getHeaders().end()) {
			
			bool conversionSuccess = false;
			size_t contentLength = safeStringToUlong(contentLengthIt->second, conversionSuccess);
			
			if (!conversionSuccess)	{
				_errorCode = 400;	
				return ;
			}
			if (method == "POST" && contentLength == 0) {
				_errorCode = 400; 
				return;
			}
			
			if (contentLength >	_maxBodySize) {
				_errorCode = 413;	
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
					_errorCode = 400;	
					return;
				}
			}
		} else if (method == "POST") {
			
			_errorCode = 411;	
			return;
		}
		
		if ((method	== "POST") && 
			getHeaders().find("Content-Type") == getHeaders().end()) {
			_errorCode = 400;	
			return ;
		}


		
		std::map<std::string, std::string>::const_iterator contentTypeIt = getHeaders().find("Content-Type");
		if (contentTypeIt != getHeaders().end()) {
			std::string contentType = contentTypeIt->second;
			
			size_t paramPos = contentType.find(';');
			if (paramPos != std::string::npos) {
				contentType = contentType.substr(0, paramPos);
			}
		
			size_t lastNonSpace = contentType.find_last_not_of(" \t\r");
			if (lastNonSpace != std::string::npos)
				contentType = contentType.substr(0, lastNonSpace + 1);
						
			static const std::string allowedTypes[] = {
				"text/html", "image/png", "image/jpeg", "text/css",
				"application/javascript", "application/json", "application/xml",
				"application/pdf", "text/plain", "text/csv", "application/x-www-form-urlencoded", "multipart/form-data"
			};
			
			bool isAllowed = false;
			for (size_t i = 0; i < sizeof(allowedTypes)/sizeof(allowedTypes[0]); ++i) {
				if (contentType == allowedTypes[i]) {
					isAllowed = true;
					break;
				}
			}
			
			if (!isAllowed) {
				_errorCode = 415;
				return;
			}
		}
		

		
		std::map<std::string, std::string>::const_iterator expectIt	= getHeaders().find("Expect");
		if (expectIt != getHeaders().end())	{
			std::string	expect = expectIt->second;
			if (expect.find("100-continue")	!= std::string::npos) {
				_errorCode = 417;	
				return ;
			}
		}
	} else {
		
		_errorCode = 400;	
		return ;
	}

	_errorCode = 200;	
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
