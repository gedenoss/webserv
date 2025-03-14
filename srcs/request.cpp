#include "request.hpp"

Request::Request()
{
    _method = "GET";
    _url = "/index.html";
    _version = "HTTP/1.1000000";
    _headers["Host"] = "";
    _headers["User-Agent"] = "";
    _headers["Accept"] = "text/html";
    _headers["Accept-Language"] = "en-US;q=5";
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