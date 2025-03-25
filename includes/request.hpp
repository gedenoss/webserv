#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include "config.hpp"

class Request {
    private:
        size_t maxBodySize;
        size_t maxHeadersSize;
        std::string method;
        std::string url;
        std::string httpVersion;
        std::string acceptLanguage;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> queryParams;
        std::string body;
        int errorCode;

        void parseQueryParams();  // Nouvelle méthode pour extraire les query parameters

    public:
        Request(size_t maxBodySize, size_t maxHeadersSize);
        //Request(const Request &other);
        //Request& operator=(const Request &other);
        ~Request();
        void parse(const std::string &rawRequest, Config &config);

        bool isValidMethod();
        bool isValidHttpVersion();
        bool isValidUrl();
        bool isMethodAllowedForRoute(Config &config);
        static std::string trim(const std::string &str); // Ajout d'une méthode pour nettoyer les espaces
        
        // Getters
        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getBody() const;
        int getErrorCode() const { return errorCode;};
        std::map<std::string, std::string>& getHeaders();
        const std::map<std::string, std::string>& getHeaders() const;
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


#endif // REQUEST_HPP