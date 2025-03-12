#include "request.hpp"

Request::Request()
{
    _method = "GET";
    _url = "/index.jpg";
    _version = "HTTP/1.1";
    _headers["Host"] = "";
    _headers["User-Agent"] = "";
    _headers["Accept"] = "image/*, text/html";
    _body = "";
}

std::string Request::getMethod() const
{
    return _method;
}

std::string Request::getUrl() const
{
    return _url;
}

std::map<std::string, std::string> Request::getHeaders() const
{
    return _headers;
}

std::string Request::getBody() const
{
    return _body;
}

Request::~Request()
{
}