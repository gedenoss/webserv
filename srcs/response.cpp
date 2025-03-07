#include "../includes/response.hpp"

Response::Response()
{
    _status_code = 0;
    _status_message = "";
    _date = "";
    _server = "Webserv/1.0";
    _content_type = "";
    _content_length = "";
    _connection = "close";
    _body = "";
}

void Response::set_time()
{
    time_t now = time(0);
    tm *utc = gmtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", utc);
    _date = buffer;
    std::cout << "Time of day in UTC : " << _date << std::endl;
}

void Response::send_response()
{
    std::cout << "HTTP/1.1 " << _status_code << " " << _status_message << std::endl;
    std::cout << "Date: " << _date << std::endl;
    std::cout << "Server: " << _server << std::endl;
    std::cout << "Content-Type: " << _content_type << std::endl;
    std::cout << "Content-Length: " << _content_length << std::endl;
    std::cout << "Connection: " << _connection << std::endl;
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