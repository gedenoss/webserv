#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/config.hpp"
#include <cctype>
#include <cstdlib>
// Constructeur
Request::Request() {}

// Destructeur
Request::~Request() {}

HttpRequestParser::HttpRequestParser(size_t maxBody, size_t maxHeaders)	
	: maxBodySize(maxBody), maxHeadersSize(maxHeaders) {}

std::string Request::getMethod() const { return method; }
std::string Request::getUrl() const { return url; }
std::string Request::getHttpVersion() const { return httpVersion; }
std::string Request::getBody() const { return body; }
std::map<std::string, std::string> Request::getHeaders() const { return headers; }

// Ajout des setters pour modifier les attributs privés
void Request::setMethod(const std::string& m) { method = m; }
void Request::setUrl(const std::string& u) { url = u; }
void Request::setHttpVersion(const std::string& v) { httpVersion = v; }
void Request::setBody(const std::string& b) { body = b; }
void Request::addHeader(const std::string& key, const std::string& value) { headers[key] = value; }

bool HttpRequestParser::isValidMethod(const std::string &method) {
	return (method == "GET" || method == "POST" || method == "DELETE");
}

bool HttpRequestParser::isMethodAllowedForRoute(const std::string &method, const std::string &url, const Config &config) {
    // Trouver la route correspondante à l'URL
    for (const ServerConfig &server : config.servers) {
        for (const LocationConfig &location : server.locations) {
            if (url.find(location.path) == 0) {  // Vérifie si l'URL commence par le chemin de la route
                // Vérifier si la méthode est autorisée pour cette location
                for (const std::string &allowedMethod : location.allow_methods) {
                    if (method == allowedMethod) {
                        return true;  // La méthode est autorisée
                    }
                }
                break;
            }
        }
    }
    return false;  // La méthode n'est pas autorisée pour la route
}
bool HttpRequestParser::isValidHttpVersion(const std::string &version) {
	return (version == "HTTP/1.1");
}

bool HttpRequestParser::isValidUrl(const std::string &url) {
	return !url.empty() && url[0] == '/' && url.find("..") == std::string::npos;
}

Request HttpRequestParser::parse(const std::string &rawRequest, int &errorCode, const Config &config) {
	Request request;
	std::istringstream stream(rawRequest);
	std::string line;

	if (!std::getline(stream, line) || line.empty()) {
		errorCode = 400;	
		return request;
	}

	std::istringstream requestLine(line);
	std::string method, url, httpVersion;
	requestLine >> method >> url >> httpVersion;

	if (!isValidMethod(method)) {
		errorCode = 405;
		return request;
	}
	if (!isMethodAllowedForRoute(method, url, config)) {
        errorCode = 405;  // Méthode non autorisée
        return request;
    }
	if (!isValidUrl(url)) {
		errorCode = 400;
		return request;
	}
	if (!isValidHttpVersion(httpVersion)) {
		errorCode = 505;
		return request;
	}

	// Utilisation des setters
	request.setMethod(method);
	request.setUrl(url);
	request.setHttpVersion(httpVersion);

	size_t headersSize = 0;
	while (std::getline(stream, line) && line != "\r") {
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) {
			errorCode = 400;
			return request;
		}

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		
		key.erase(key.find_last_not_of(" \t\r") + 1);
		value.erase(0, value.find_first_not_of(" \t"));

		// Utilisation de addHeader() au lieu d'accès direct à headers
		request.addHeader(key, value);
		headersSize += line.size();
	}

	if (headersSize > maxHeadersSize) {
		errorCode = 431;
		return request;
	}

	// Vérification de l'en-tête "Host"
	if (request.getHttpVersion() == "HTTP/1.1" && request.getHeaders().find("Host") == request.getHeaders().end()) {
		errorCode = 400;
		return request;
	}

	// Gestion de Content-Length
	if (request.getHeaders().find("Content-Length") != request.getHeaders().end()) {
		size_t contentLength = std::atoi(request.getHeaders().at("Content-Length").c_str());
		if (contentLength > maxBodySize) {
			errorCode = 413;
			return request;
		}

		char *buffer = new char[contentLength];
		stream.read(buffer, contentLength);
		request.setBody(std::string(buffer, contentLength));
		delete[] buffer;
	}

	errorCode = 200;
	return request;
}

void Request::printRequest() const {
	std::cout << "Method: " << getMethod() << "\n"
			  << "URL: " << getUrl() << "\n"
			  << "HTTP Version: " << getHttpVersion() << "\n"
			  << "Headers:\n";
	
	// Utilisation du getter pour parcourir les headers
	const std::map<std::string, std::string>& headersRef = getHeaders(); 
	for (std::map<std::string, std::string>::const_iterator it = headersRef.begin(); it != headersRef.end(); ++it)
	{
			std::cout << "  " << it->first << ": " << it->second << "\n";
	}

	std::cout << "Body: " << (getBody().empty() ? "[empty]" : getBody()) << "\n";
}