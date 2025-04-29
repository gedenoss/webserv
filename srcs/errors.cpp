#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

Errors::Errors(Response &resp) : _response(resp) {}

std::string Errors::getError(int code, const std::string &message)
{
    _response.setStatusCode(code);
    _response.setStatusMessage(message);
    _response.setTime();
    std::string errorPath = toString(code) + ".html";
    if (fileExists(errorPath))
        _response.setBody(readFile(errorPath));
    else
    {
        std::string body = "<html><body><h1>" + toString(code) + " " + message + "</h1></body></html>";
        _response.setBody(body);
    }
    
    _response.setHeaders("Content-Type", "text/html");
    _response.setContentLength();
    _response.setContentLanguage();

    if (code == 304)
        _response.setLastModified(_response.getPath());

    return _response.generateResponse(true);
}

std::string Errors::generateError(int code)
{
    switch(code) {
        case 304:
            return error304();
        case 400:
            return error400();
        case 403:
            return error403();
        case 404:
            return error404();
        case 405:
            return error405();
        case 406:
            return error406();
        case 411:
            return error411();
        case 413:
            return error413();
        case 414:
            return error414();
        case 415:
            return error415();
        case 416:
            return error416();
        case 417:   
            return error417();
        case 431:
            return error431();
        case 500:
            return error500();
        case 501:
            return error501();
        case 504:
            return error504();
        case 505:
            return error505();
        case 507:
            return error507();
        default:
            return error500();
    }
}

std::string Errors::error304() { return getError(304, "Not Modified"); }
std::string Errors::error400() { return getError(400, "Bad Request"); }
std::string Errors::error403() { return getError(403, "Forbidden"); }
std::string Errors::error404() { return getError(404, "Not Found"); }
std::string Errors::error405() { return getError(405, "Method Not Allowed"); }
std::string Errors::error406() { return getError(406, "Not Acceptable"); }
std::string Errors::error411() { return getError(411, "Length Required"); }
std::string Errors::error413() { return getError(413, "Payload Too Large"); }
std::string Errors::error414() { return getError(414, "URI Too Long"); }
std::string Errors::error415() { return getError(415, "Unsupported Media Type"); }
std::string Errors::error416() { return getError(416, "Range Not Satisfiable"); }
std::string Errors::error417() { return getError(417, "Expectation Failed"); }
std::string Errors::error431() { return getError(431, "Request Header Fields Too Large"); }
std::string Errors::error500() { return getError(500, "Internal Server Error"); }
std::string Errors::error501() { return getError(501, "Not Implemented"); }
std::string Errors::error504() { return getError(504, "Gateway Timeout"); }
std::string Errors::error505() { return getError(505, "HTTP Version Not Supported"); }
std::string Errors::error507() { return getError(507, "Insufficient Storage"); }

Errors::~Errors() {}
