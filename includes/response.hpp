#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "webserv.hpp"

class Response {
    public:
        Response();
        ~Response();

        void get_response();
        void send_response();
        void set_status_code(int status_code);
        void set_status_message(std::string status_message);
        void set_time();

    private:
        int _status_code;
        std::string _status_message;
        std::string _version;
        std::string _date;
        std::string _server;
        std::string _content_type;
        std::string _content_length;
        std::string _connection;
        std::string _body;
};

#endif