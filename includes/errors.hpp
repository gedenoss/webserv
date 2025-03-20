#ifndef ERRORS_HPP
# define ERRORS_HPP

#include "response.hpp"


class Errors 
{
    public:
        Errors(Response &resp);
        ~Errors();

        std::string generateError(int code, const std::string &message);

        std::string error304();
        std::string error400();
        std::string error403();
        std::string error404();
        std::string error406();
        std::string error415();
        std::string error500();
        std::string error507();

    private:
        Response _response;
};

#endif