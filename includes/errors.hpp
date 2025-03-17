#ifndef ERRORS_HPP
# define ERRORS_HPP

#include "response.hpp"


class Errors 
{
    public:
        Errors(Response &resp);
        ~Errors();

        std::string error304();
        std::string error400();
        std::string error403();
        std::string error404();
        std::string error406();

    private:
        Response _response;
};

#endif