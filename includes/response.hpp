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

        std::string getLanguage();

        std::string getResponse(const Request &request, const std::string &host);
        std::string postResponse(const Request &request, const std::string &root);
        std::string deleteResponse(const Request &request, const std::string &root);

        std::string response200(const std::string &path, Errors &errors);

    private:
        Request &request;
        int _status_code;
        std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::vector<std::string> _available_languages;
        std::string _body;
        std::vector<std::string> _order;
};

#endif