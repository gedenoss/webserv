#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include "server.hpp"
#include "request.hpp"

class Errors;

const int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 Mo

class Response {
    public:
        Response(Request &req, ServerConfig &serv);
        ~Response();

        std::string sendResponse();
        std::string generateResponse();
        void findLocation();

        void setStatusCode(int status_code);
        void setStatusMessage(const std::string &status_message);
        void setHeaders(const std::string &key, const std::string &value);
        void setBody(const std::string &body);
        void setTime();
        void setContentType();
        void setContentLength();
        void setContentLanguage();
        void setLastModified(const std::string &path);
        void setEtag(const std::string &path);

        int getStatusCode() const;
        std::string getPath() const;
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

        void handleCGI();
        std::string handleUpload(Errors &errors);

        std::string getResponse(Errors &errors, const std::string &host);
        std::string postResponse(Errors &errors, const std::string &root);
        std::string deleteResponse(Errors &errors, const std::string &root);

        std::string response200(Errors &errors);
        std::string response204();

    private:
        Request &_request;
        ServerConfig _server;
        std::string _path;
        bool _autoindex;
        int _status_code;
        std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::vector<std::string> _available_languages;
        std::string _body;
        std::vector<std::string> _order;
};

#endif