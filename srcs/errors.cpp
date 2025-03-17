#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

Errors::Errors(Response &resp) : _response(resp)
{
}

std::string Errors::error304()
{
    _response.setStatusCode(304);
    _response.setStatusMessage("Not Modified");
    _response.setTime();
    _response.setBody("<html><body><h1>304 Not Modified</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", toString(_response.getBody().length()));
    _response.setHeaders("Content-Language", "en-US");
    _response.setHeaders("Last-Modified", formatHttpDate(time(0)));
    return _response.generateResponse();
}

std::string Errors::error400()
{
    _response.setStatusCode(400);
    _response.setStatusMessage("Bad Request");
    _response.setTime();
    _response.setBody("<html><body><h1>400 Bad Request</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", toString(_response.getBody().length()));
    _response.setHeaders("Content-Language", "en-US");
    return _response.generateResponse();
}

std::string Errors::error403()
{
    _response.setStatusCode(403);
    _response.setStatusMessage("Forbidden");
    _response.setTime();
    _response.setBody("<html><body><h1>403 Forbidden</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", toString(_response.getBody().length()));
    _response.setHeaders("Content-Language", "en-US");
    return _response.generateResponse();
}

std::string Errors::error404()
{
    _response.setStatusCode(404);
    _response.setStatusMessage("Not Found");
    _response.setTime();
    _response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", toString(_response.getBody().length()));
    _response.setHeaders("Content-Language", "en-US");
    return _response.generateResponse();
}

std::string Errors::error406()
{
    _response.setStatusCode(406);
    _response.setStatusMessage("Not Acceptable");
    _response.setTime();
    _response.setBody("<html><body><h1>406 Not Acceptable</h1></body></html>");
    _response.setHeaders("Content-Type","text/html");
    _response.setHeaders("Content-Length", toString(_response.getBody().length()));
    _response.setHeaders("Content-Language", "en-US");
    return _response.generateResponse();
}

Errors::~Errors()
{
}