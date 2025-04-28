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
std::string Request::getIp() const {
	std::string ip;
	// const std::map<std::string,	std::string>& headersRef = getHeaders();	
	for	(std::map<std::string, std::string>::const_iterator	it = _headers.begin(); it != _headers.end(); ++it)
	{
		if(it->first == "Host")
			ip = it->second;
	}
	return ip;
}

void Request::parse(const std::string &rawRequest,	Config &config) {
	std::istringstream stream(rawRequest);
	std::string	line;
	(void)config;
	_errorCode = 0;
	size_t headersSize = 0;
	bool headersFinished = false;
	
	if (rawRequest.size() >	8000 * 100 * 100) {
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
		queryString = url.substr(url.find("?") + 1, std::string::npos);
		url = url.substr(0, url.find("?"));
	}

	this->initializeRequest(*this, method, url, httpVersion, queryString);
	
	parseHostHeader(stream);
	
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

    if (!validateMethodAndVersion(config)) {
        return;
    }
	
	//suite du petit header bien kawaii bien sexy
	parseHeaders(stream, headersSize, headersFinished);
	if (_errorCode != 0) {
        return;
    }

	std::cout << "HeadersFinished? : " << headersFinished << std::endl;

	
	if (getHttpVersion() == "HTTP/1.1" && getHeaders().find("Host")	== getHeaders().end()) {
		_errorCode = 400;	
		return ;
	}

	processHeaders(stream, headersFinished);
    if (_errorCode != 0) {
		std::cout << "Error code: " << _errorCode << std::endl;
        return;
    }

	std::map<std::string, std::string>::const_iterator multipartIt = getHeaders().find("Content-Type");
	if (multipartIt != getHeaders().end()) {
    std::string contentType = multipartIt->second;
    if (contentType.find("multipart/form-data") != std::string::npos) {
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos) {
            std::string boundary = contentType.substr(boundaryPos + 9);
            parseMultipartFormData(getBody(), boundary);
        	}
    	}
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
		// if(it->first == "Host")
		// 	importantValue = it->second;
		std::cout << "  " << it->first << ": " << it->second << "\n";
		
	}

	std::cout << "Body:	" << (getBody().empty()	? "[empty]"	: getBody()) << "\n";
	// std::cout << "  " << importantValue <<"\n";
	// return importantValue;
}
