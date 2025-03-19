#include "request.hpp"

Request::Request()
{
    _method = "DELETE";
    _url = "/index.json";
    _version = "HTTP/1.1";
    _headers["Host"] = "";
    _headers["User-Agent"] = "";
    _headers["Accept"] = "";
    _headers["If-Modified-Since"] = "";
    _headers["Accept-Language"] = "en-US";
    _body = "hello everyone";
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