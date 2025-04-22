#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include "config.hpp"

class Request {
    private:
        size_t _maxBodySize;
        size_t _maxHeadersSize;
        std::string _method;
        std::string _url;
        std::string _httpVersion;
        std::string _queryString;
        //std::string _acceptLanguage;
        std::map<std::string, std::string> _headers;
        //std::map<std::string, std::string> _queryParams;
        std::string _body;
        int _errorCode;
        LocationConfig _location;

        //void parseQueryParams();

    public:
        Request(size_t maxBodySize, size_t maxHeadersSize);
        ~Request();

        void parse(const std::string &rawRequest, Config &config);
        void checkRawRequest(const std::string &rawRequest, Config &config, std::istringstream &stream, std::string line );

        size_t safeStringToUlong(const std::string&	str, bool& success);

        bool isValidMethod();
        bool isValidHttpVersion();
        bool isValidUrl();
        bool isMethodAllowedForRoute(Config &config);
        
        std::string getIp() const;
        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getBody() const;
        std::string getQueryString() const { return _queryString; };
        int getErrorCode() const;
        std::map<std::string, std::string>& getHeaders();
        const std::map<std::string, std::string>& getHeaders() const;
        const LocationConfig& getLocation() const {return _location;}
        //std::map<std::string, std::string> getQueryParams() const;

        void setMethod(const std::string& m);
        void setUrl(const std::string& u);
        void setHttpVersion(const std::string& v);
        void setBody(const std::string& b);
        void addHeader(const std::string& key, const std::string& value);

        std::string printRequest() const;
};

#endif
    