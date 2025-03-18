#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include "webserv.hpp"
#include "request.hpp"

class Errors;

class Response {
    public:
        Response(Request &req);
        ~Response();

        std::string sendResponse(const Request &request);
        std::string generateResponse();

        void setStatusCode(int status_code);
        void setStatusMessage(const std::string &status_message);
        void setHeaders(const std::string &key, const std::string &value);
        void setBody(const std::string &body);
        void setTime();

        int getStatusCode() const;
        std::string getStatusMessage() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;

        std::string getContentType();
        std::string getLanguage();

        bool isAcceptable();
        bool isModifiedSince(const std::string &ifModifiedSince);
        bool handleIfModifiedSince(const std::map<std::string, std::string> &headers);
        bool isNotModified(const std::map<std::string, std::string> &headers);
        bool handleDirectory();
        bool isCGI();

        std::string getResponse(const Request &request, const std::string &host);
        std::string postResponse(const Request &request, const std::string &root);
        std::string deleteResponse(const Request &request, const std::string &root);

        std::string response200(Errors &errors);

    private:
        Request &request;
        std::string _path;
        int _status_code;
        std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::vector<std::string> _available_languages;
        std::string _body;
        std::vector<std::string> _order;
};

#endif