#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

Errors::Errors(Response &resp) : _response(resp)
{
}

std::string Errors::generate_400_response()
{
    _response.setStatusCode(400);
    _response.setStatusMessage("Bad Request");
    _response.setTime();
    _response.setBody("<html><body><h1>400 Bad Request</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", to_string(_response.getBody().length()));
    _response.setHeaders("Content-Language", _response.get_language());
    return _response.generate_response();
}

std::string Errors::generate_403_response()
{
    _response.setStatusCode(403);
    _response.setStatusMessage("Forbidden");
    _response.setTime();
    _response.setBody("<html><body><h1>403 Forbidden</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", to_string(_response.getBody().length()));
    _response.setHeaders("Content-Language", _response.get_language());
    return _response.generate_response();
}

std::string Errors::generate_404_response()
{
    _response.setStatusCode(404);
    _response.setStatusMessage("Not Found");
    _response.setTime();
    _response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", to_string(_response.getBody().length()));
    _response.setHeaders("Content-Language", _response.get_language());
    return _response.generate_response();
}

std::string Errors::generate_406_response()
{
    _response.setStatusCode(406);
    _response.setStatusMessage("Not Acceptable");
    _response.setTime();
    _response.setBody("<html><body><h1>406 Not Acceptable</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", to_string(_response.getBody().length()));
    _response.setHeaders("Content-Language", _response.get_language());
    return _response.generate_response();
}

Errors::~Errors()
{
}