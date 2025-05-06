#include "../includes/request.hpp"
#include "../includes/utils.hpp"
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
void Request::parse(const std::string &rawRequest, Config &config) {
    _errorCode = 0;

    if (!validateRequestSize(rawRequest)) return;

    std::istringstream stream(rawRequest);
    if (!parseRequestLine(stream)) return;

    parseHostHeader(stream);

    if (!validateMethod()) {
        _errorCode = 400;
        std::cout << "Invalid method: " << _method << std::endl;
        return;
    }

    if (!validateUrl()) return;

    if (!validateMethodAndVersion(config)) return;
    size_t headersSize = 0;
    bool headersFinished = false;
    parseHeaders(stream, headersSize, headersFinished);
    if (_errorCode != 0) return;
   
    if (!validateHostHeader()) return;

    processHeaders(stream, headersFinished);

    if (_errorCode != 0) return;

    if (getHeaders().find("Transfer-Encoding") != getHeaders().end() &&
        getHeaders()["Transfer-Encoding"] == "chunked")
        processChunkedBody(stream);
    if(_errorCode != 0) return;

    handleMultipartFormData();

    _errorCode = 200;
}

bool Request::validateRequestSize(const std::string &rawRequest) {
    if (rawRequest.size() > 8000 * 100 * 100) {
        _errorCode = 431;
        return false;
    }
    return true;
}

bool Request::parseRequestLine(std::istringstream &stream) {
    std::string line;
    if (!std::getline(stream, line) || line.empty()) {
        _errorCode = 400;
        std::cout << "Empty request line" << std::endl;
        return false;
    }

    if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
    }

    std::istringstream requestLine(line);
    std::string method, url, httpVersion, queryString;
    requestLine >> method >> url >> httpVersion;

    if (url.length() > 8000) {
        _errorCode = 414;
        return false;
    }

    if (url.find("?") != std::string::npos) {
        queryString = url.substr(url.find("?") + 1, std::string::npos);
        url = url.substr(0, url.find("?"));
    }

    this->initializeRequest(*this, method, url, httpVersion, queryString);
    return true;
}

bool Request::validateMethod() {
    const std::string validMethods[] = {"GET", "POST", "DELETE"};

    for (size_t i = 0; i < sizeof(validMethods) / sizeof(validMethods[0]); ++i) {
        if (_method == validMethods[i]) {
            return true;
        }
    }

    _errorCode = 501;
    return false;
}

bool Request::validateUrl() {
    if (!isValidUrl()) {
        _errorCode = 400;
        std::cout << "Invalid URL: " << _url << std::endl;
        return false;
    }
    return true;
}

bool Request::validateHostHeader() {
    if (getHttpVersion() == "HTTP/1.1" && getHeaders().find("Host") == getHeaders().end()) {
        _errorCode = 400;
        std::cout << "Missing Host header" << std::endl;
        return false;
    }
    return true;
}

void Request::handleMultipartFormData() {
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
}

/*----------------------------------------------------------------------------------------------------------------------------------*/



void Request::printRequest() const {
	std::cout << "\r\n" << BOLD << getMethod() << " "
			  << getUrl() << " "
			  << getHttpVersion() << "\r\n" << RESET;
			//   << "Headers:\n";
	
	
	const std::map<std::string,	std::string>& headersRef = getHeaders();	
	for	(std::map<std::string, std::string>::const_iterator	it = headersRef.begin(); it != headersRef.end(); ++it)
	{
		// if(it->first == "Host")
		// 	importantValue = it->second;
		// std::cout << it->first << ": " << it->second << "\n";
		
	}
	std::cout << std::endl;
	// std::cout << "Body:	" << (getBody().empty()	? "[empty]"	: getBody()) << "\n";
	// std::cout << "  " << importantValue <<"\n";
	// return importantValue;
}
