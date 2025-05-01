#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <cstdio>
#include "server.hpp"
#include "request.hpp"

class Errors;

const int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 Mo

struct Range
{
    long start;
    long end;
    bool isValid;
    bool isPartial;
};

class Response {
    public:
        Response(Request &req, ServerConfig &serv);
        ~Response();


        std::string sendResponse();
        std::string generateResponse(bool isError);
        std::string generateResponseCgi();
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
        void setHeadersForResponse();
        void setStatusCodeAndMessage();
        void setInfoRequest();

        int getStatusCode() const;
        std::string getPath() const;
        std::string getStatusMessage() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;

        std::string getContentType();
        std::string getLanguage();
        int getContentLength();


        bool isAcceptable();
        bool isModifiedSince(const std::string &ifModifiedSince);
        bool handleIfModifiedSince(const std::map<std::string, std::string> &headers);
        bool isNotModified(const std::map<std::string, std::string> &headers);
        bool isCGI();
        bool isContentLanguageEmpty();
        bool handleFileErrors();
        bool handleRange();
        std::string sendFileResponse();
        bool tryPath(const std::string &p);
        void findPath();
        void listDirectory();

        void setEnv();
        void handleCGI(Errors &errors);
        void childRoutine();
        void manageBodyForCgi();
        void manageCgiOutfile();
        void checkCgiStatus();
        void readOutfile();
        void killCgi();
        std::string handleForm(Errors &errors);
        std::string handleUpload(Errors &errors);

        std::string getResponse(Errors &errors);
        std::string postResponse(Errors &errors);
        std::string deleteResponse(Errors &errors);

        std::string validResponse(Errors &errors);
        std::string jsonListFiles(Errors &errors);

        void cleanUpCgiFiles();

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
        std::string _headerCgi;
        std::vector<std::string> _order;
        std::vector<std::string> _env;
        std::vector<std::string> _arg;
        bool _listingDirectory;
        bool _getBody;
        std::string _cgiPath;
        std::string _cgiBinPath;
        std::string _cgiScriptName;
        std::string _cgiInfilePath;
        std::string _cgiOutfilePath;
        int _cgiPid;
        std::string _cgiInfileSize;
        bool _cgiIsRunning;
        bool _responseIsReady;
};

#endif