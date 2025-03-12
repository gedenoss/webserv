#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "webserv.hpp"
#include "request.hpp"

class Response {
    public:
        Response();
        ~Response();

        std::string send_response(const Request &request);
        std::string generate_response();

        void setStatusCode(int status_code);
        void setStatusMessage(const std::string &status_message);
        void setBody(const std::string &body);
        void setTime();

        std::string get_response(const Request &request, const std::string &host);
        std::string post_response(const Request &request, const std::string &root);
        std::string delete_response(const Request &request, const std::string &root);

        std::string generate_406_response();
        std::string generate_404_response();
        std::string generate_200_response(const std::string &path);
        std::string generate_403_response();

    private:
        int _status_code;
        std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::string _body;
};

#endif