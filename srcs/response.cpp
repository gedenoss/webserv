#include "../includes/response.hpp"

Response::Response()
{
    _status_code = 0;
    _status_message = "";
    _headers["Date"] = "";
    _headers["Server"] = "";
    _headers["Content-Type"] = "";
    _headers["Content-Length"] = "";
    _headers["Connection"] = "";
    _body = "";
}

void Response::set_time()
{
    time_t now = time(0);
    tm *utc = gmtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", utc);
    _headers["Date"] = buffer;
    std::cout << "Time of day in UTC : " << buffer << std::endl;
}

void Response::send_response()
{
    std::cout << "HTTP/1.1 " << _status_code << " " << _status_message << std::endl;
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << std::endl;
    std::cout << _body << std::endl;
}

void Response::set_status_code(int status_code)
{
    _status_code = status_code;
}

void Response::get_response()
{

}



Response::~Response()
{
}