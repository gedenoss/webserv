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

struct Range
{
    size_t start;
    size_t end;
    bool isValid;
    bool isPartial;
};

class Response {
    public:
        Response(Request &req, ServerConfig &serv);
        ~Response();

        std::string sendResponse();
        std::string generateResponse();
        void findLocation(Errors &errors);

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
        bool isCGI();
        std::string sendFileResponse();
        void findPath();

        void setEnv();
        void handleCGI();
        void handleCGIGet();
        void handleCGIPost();
        std::string handleForm(Errors &errors);

        std::string getResponse(Errors &errors);
        std::string postResponse(Errors &errors);
        std::string deleteResponse(Errors &errors);

        std::string response200(Errors &errors);
        std::string response204();

    private:
        Request &_request;
        Range _range;
        LocationConfig _location;
        ServerConfig _server;
        std::string _path;
        std::string _root;
        bool _autoindex;
        std::string _index;
        int _status_code;
        std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::vector<std::string> _available_languages;
        std::string _body;
        std::vector<std::string> _order;
        std::map<std::string, std::string> _env;
};

#endif