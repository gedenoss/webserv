#ifndef ERRORS_HPP
# define ERRORS_HPP

#include "response.hpp"

typedef std::string (Errors::*ErrorFunction)();

class Errors 
{
    public:
        Errors(Response &resp);
        ~Errors();

        std::string getError(int code, const std::string &message);
        std::string generateError(int code);

        std::string error304();
        std::string error400();
        std::string error403();
        std::string error404();
        std::string error405();
        std::string error406();
        std::string error411();
        std::string error413();
        std::string error414();
        std::string error415();
        std::string error416();
        std::string error417();
        std::string error431();
        std::string error500();
        std::string error501();
        std::string error505();
        std::string error507();

    private:
        Response _response;
};

#endif