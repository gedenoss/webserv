#ifndef ERRORS_HPP
# define ERRORS_HPP

#include "response.hpp"


class Errors 
{
    public:
        Errors(Response &resp);
        ~Errors();

        std::string generate_400_response();
        std::string generate_403_response();
        std::string generate_404_response();
        std::string generate_406_response();

    private:
        Response _response;
};

#endif