#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <string>

class Request {
    private:
        std::string method;
        std::string url;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::string body;

    public:
        Request();
        ~Request();

        // Getters
        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getBody() const;
        std::map<std::string, std::string> getHeaders() const;

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
        
        Request parse(const std::string &rawRequest, int &errorCode);

        static bool isValidMethod(const std::string &method);
        static bool isValidHttpVersion(const std::string &version);
        static bool isValidUrl(const std::string &url);
};

#endif // REQUEST_HPP
