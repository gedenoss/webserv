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

        std::string send_response(const Request &request);
        std::string generate_response();

        void setStatusCode(int status_code);
        void setStatusMessage(const std::string &status_message);
        void setHeaders(const std::string &key, const std::string &value);
        void setBody(const std::string &body);
        void setTime();

        int getStatusCode() const;
        std::string getStatusMessage() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;

        bool is_acceptable(const std::string &path);
        std::string get_language();

        std::string get_response(const Request &request, const std::string &host);
        std::string post_response(const Request &request, const std::string &root);
        std::string delete_response(const Request &request, const std::string &root);

        std::string generate_200_response(const std::string &path, Errors &errors);

    private:
        Request &request;
        int _status_code;
        std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::vector<std::string> _available_languages;
        std::string _body;
};

#endif