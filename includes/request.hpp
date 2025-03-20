#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include "config.hpp"

class Request {
    private:
        std::string method;
        std::string url;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> queryParams;
        std::string body;

        void parseQueryParams();  // Nouvelle méthode pour extraire les query parameters

    public:
        Request();
        //Request(const Request &other);
        //Request& operator=(const Request &other);
        ~Request();

        // Getters
        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getBody() const;
        std::map<std::string, std::string> getHeaders() const;
        std::map<std::string, std::string> getQueryParams() const; // Récupérer les paramètres GET

        // Setters
        void setMethod(const std::string& m);
        void setUrl(const std::string& u);
        void setHttpVersion(const std::string& v);
        void setBody(const std::string& b);
        void addHeader(const std::string& key, const std::string& value);

        // Debug
        void printRequest() const;
};

class HttpRequestParser {
    private:
        size_t maxBodySize;
        size_t maxHeadersSize;

    public:
        HttpRequestParser(size_t maxBody, size_t maxHeaders);
        
        Request parse(const std::string &rawRequest, int &errorCode, const Config &conif);

        static bool isValidMethod(const std::string &method);
        static bool isValidHttpVersion(const std::string &version);
        static bool isValidUrl(const std::string &url);
        static bool isMethodAllowedForRoute(const std::string &method, const std::string &url, const Config &config);
        static std::string trim(const std::string &str); // Ajout d'une méthode pour nettoyer les espaces
};

#endif // REQUEST_HPP