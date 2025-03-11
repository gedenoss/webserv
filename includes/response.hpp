#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include "webserv.hpp"
#include "request.hpp"

class Response {
    public:
        Response();
        ~Response();

        std::string send_response(const Request &request);
        void send_response();
        void set_status_code(int status_code);
        void set_status_message(const std::string &status_message);
        char *set_time();

        std::string get_response(const Request &request, const std::string &host);
        void post_response(const Request &request, const std::string &root);
        void delete_response(const Request &request, const std::string &root);

    private:
        int _status_code;
        std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::string _body;
};

#endif