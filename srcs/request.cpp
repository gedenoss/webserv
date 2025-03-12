#include "request.hpp"

Request::Request()
{
    _method = "GET";
    _url = "/index.f";
    _version = "HTTP/1.1";
    _headers["Host"] = "";
    _headers["User-Agent"] = "";
    _headers["Accept"] = "";
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

Request::~Request()
{
}