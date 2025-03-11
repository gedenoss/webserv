#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <string>
#include <map>

class Request {
    public:
        Request();
        ~Request();

        std::string getMethod() const;
        std::string getUrl() const;

    private :
        std::string _method;
        std::string _url;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
};

#endif