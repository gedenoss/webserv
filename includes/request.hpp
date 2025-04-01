#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include "config.hpp"

class Request {
    private:
        LocationConfig _location;
        size_t _maxBodySize;
        size_t _maxHeadersSize;
        std::string _method;
        std::string _url;
        std::string _httpVersion;
        //std::string _acceptLanguage;
        std::map<std::string, std::string> _headers;
        //std::map<std::string, std::string> _queryParams;
        std::string _body;
        int _errorCode;
        LocationConfig location;

        //void parseQueryParams();

    public:
        Request(size_t maxBodySize, size_t maxHeadersSize);
        ~Request();

        void parse(const std::string &rawRequest, Config &config);

        size_t safeStringToUlong(const std::string&	str, bool& success);

        bool isValidMethod();
        bool isValidHttpVersion();
        bool isValidUrl();
        bool isMethodAllowedForRoute(Config &config);
        
        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getBody() const;
        int getErrorCode() const;
        LocationConfig getLocation() const { return location; };
        std::map<std::string, std::string>& getHeaders();
        const std::map<std::string, std::string>& getHeaders() const;
        //std::map<std::string, std::string> getQueryParams() const;

        void setMethod(const std::string& m);
        void setUrl(const std::string& u);
        void setHttpVersion(const std::string& v);
        void setBody(const std::string& b);
        void addHeader(const std::string& key, const std::string& value);

        void printRequest() const;
};

#endif

