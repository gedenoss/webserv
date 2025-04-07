#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"


void Response::setEnv()
{
    _env["REQUEST_METHOD"] = _request.getMethod();
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    
}


void Response::handleCGI()
{
    if (access(_path.c_str(), F_OK) == -1)
    {
        setStatusCode(404);
        return;
    }
    if (access(_path.c_str(), X_OK) == -1)
    {
        setStatusCode(403);
        return;
    }
    //d'abord faire le child
    setEnv();
}