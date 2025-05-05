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

        // Setters
        void    setStatusCode(int status_code);
        void    setStatusMessage(const std::string &status_message);
        void    setHeaders(const std::string &key, const std::string &value);
        void    setBody(const std::string &body);
        void    setTime();
        void    setContentType();
        void    setContentLength();
        void    setContentLanguage();
        void    setLastModified(const std::string &path);
        void    setEtag(const std::string &path);
        void    setHeadersForResponse();
        void    setStatusCodeAndMessage();
        void    setInfoRequest();

        // Getters
        int                                 getStatusCode() const  { return _status_code; };
        std::string                         getPath() const { return _path; };
        std::string                         getStatusMessage() const { return _status_message; };
        std::map<std::string, std::string>  getHeaders() const { return _headers; };
        std::string                         getBody() const { return _body; };
        std::string                         getContentType();
        std::string                         getLanguage();
        ServerConfig                        getServer() const { return _server; };
        std::string                         getRoot() const { return _root; };
        LocationConfig                      getLocation() const { return _location; };


        // Methods to find errors
        bool isAcceptable();
        bool isModifiedSince(const std::string &ifModifiedSince);
        bool handleIfModifiedSince(const std::map<std::string, std::string> &headers);
        bool isNotModified(const std::map<std::string, std::string> &headers);
        bool isContentLanguageEmpty();
        bool handleFileErrors();
        bool handleRange();

        // Methods to handle path and directory
        bool tryPath(const std::string &p);
        void findPath();
        void listDirectory();

        // Methods to generate response
        std::string sendFileResponse();
        std::string sendResponse();
        std::string generateResponse(bool isError);
        std::string generateResponseCgi();
        std::string validResponse(Errors &errors);

        // Methods to handle CGI
        bool isCGI();
        void setEnv();
        void handleCGI(Errors &errors);
        void childRoutine();
        void manageBodyForCgi();
        void manageCgiOutfile();
        void checkCgiStatus();
        void readOutfile();
        void killCgi();
        void cleanUpCgiFiles();

        // Methods to handle POST and DELETE
        std::string handleForm(Errors &errors);
        std::string jsonListFiles(Errors &errors);
        std::string handleUpload(Errors &errors);

        std::string getResponse(Errors &errors);
        std::string postResponse(Errors &errors);
        std::string deleteResponse(Errors &errors);

    private:
        // Response variables
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

        // CGI  variables
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