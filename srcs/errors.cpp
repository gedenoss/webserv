#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

Errors::Errors(Response &resp) : _response(resp) {}

std::string Errors::generateError(int code, const std::string &message)
{
    _response.setStatusCode(code);
    _response.setStatusMessage(message);
    _response.setTime();
    
    std::string body = "<html><body><h1>" + toString(code) + " " + message + "</h1></body></html>";
    _response.setBody(body);
    
    _response.setContentType();
    _response.setContentLength();
    _response.setContentLanguage();

    if (code == 304)
        _response.setLastModified(_response.getPath());

    return _response.generateResponse();
}

std::string Errors::error304() { return generateError(304, "Not Modified"); }
std::string Errors::error400() { return generateError(400, "Bad Request"); }
std::string Errors::error403() { return generateError(403, "Forbidden"); }
std::string Errors::error404() { return generateError(404, "Not Found"); }
std::string Errors::error406() { return generateError(406, "Not Acceptable"); }
std::string Errors::error415() { return generateError(415, "Unsupported Media Type"); }
std::string Errors::error500() { return generateError(500, "Internal Server Error"); }

Errors::~Errors() {}
