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

char *Response::set_time()
{
    time_t now = time(0);
    tm *utc = gmtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", utc);
    std::cout << "Time of day in UTC : " << buffer << std::endl;
    return (buffer);
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

void Response::set_status_message(const std::string &status_message)
{
    _status_message = status_message;
}

bool file_exists(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
    
}

std::string generate_404_response()
{
    std::string body = read_file("../static/404.html");
    if (body.empty())
    {
        body = "<html><body><h1>404 Not Found</h1></body></html>";
    }
    std::stringstream response;
    response << "HTTP/1.1 404 Not Found\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string generate_200_response(const std::string &path)
{
    
}

std::string Response::get_response(const Request &request, const std::string &root)
{
    std::string path = root + request.url;
    if (!file_exists(path))
    {
        return (generate_404_response());
    }
    else
    {
        return (generate_200_response(path));
    }

}

std::string Response::send_response(const Request &request)
{
    if (request.method == "GET") {
        return (get_response(request, "/var/http/www"));
    } 
    else if (request.method == "POST") {
        post_response(request, "www");
    }
    else if (request.method == "DELETE") {
       delete_response(request, "www");
    }
}



Response::~Response()
{
}