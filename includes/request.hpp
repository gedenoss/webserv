#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <string>
#include <map>

class Request {
    public:
        Request();
        ~Request();

        std::string method;
        std::string url;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
};

#endif