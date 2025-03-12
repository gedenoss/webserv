#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"

Response::Response()
{
    _status_code = 0;
    _status_message = "";
    _headers["Date"] = "";
    _headers["Server"] = "Webserv";
    _headers["Content-Type"] = "";
    _headers["Content-Length"] = "";
    _headers["Connection"] = "close";
    _body = "";
}

void Response::setTime()
{
    time_t now = time(0);
    tm *utc = gmtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", utc);
    _headers["Date"] = buffer;
}

std::string Response::generate_response()
{
    std::stringstream response;
    response << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";
    for (std::map<std::string, std::string>::reverse_iterator it = _headers.rbegin(); it != _headers.rend(); ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n";
    response << _body;
    std::cout << response.str() << std::endl;
    return response.str();
}

void Response::setStatusCode(int status_code)
{
    _status_code = status_code;
}

void Response::setStatusMessage(const std::string &status_message)
{
    _status_message = status_message;
}

void Response::setBody(const std::string &body)
{
    _body = body;
}

std::string read_file(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Response::generate_404_response()
{
    std::string body = read_file("./static/404.html");
    if (_body.empty())
    {
        _body = "";
    }
    setStatusCode(404);
    setStatusMessage("Not Found");
    setTime();
    _headers["Content-Type"] = "text/html";
    _headers["Content-Length"] = to_string(_body.length());
    setBody(body);
    return generate_response();
}

std::string Response::generate_403_response()
{
    std::string body = "<html><body><h1>404 Not Found</h1></body></html>";
    setStatusCode(403);
    setStatusMessage("Forbidden");
    setTime();
    _headers["Content-Type"] = "text/html";
    _headers["Content-Length"] = to_string(_body.length());
    setBody(body);
    return generate_response();
}

std::string get_content_type(const std::string &path)
{
    std::map<std::string, std::string> mime_types;
    
    mime_types[".html"] = "text/html";
    mime_types[".css"] = "text/css";
    mime_types[".js"] = "application/javascript";
    mime_types[".png"] = "image/png";
    mime_types[".jpg"] = "image/jpeg";
    mime_types[".jpeg"] = "image/jpeg";
    mime_types[".gif"] = "image/gif";
    mime_types[".svg"] = "image/svg+xml";
    mime_types[".ico"] = "image/x-icon";
    mime_types[".json"] = "application/json";
    mime_types[".xml"] = "application/xml";
    mime_types[".pdf"] = "application/pdf";
    mime_types[".mp4"] = "video/mp4";
    mime_types[".txt"] = "text/plain";
    mime_types[".csv"] = "text/csv";
    mime_types[".mp3"] = "audio/mpeg";

    size_t dot_pos = path.find_last_of(".");
    if (dot_pos != std::string::npos) {
        std::string ext = path.substr(dot_pos);
        if (mime_types.count(ext))
            return mime_types[ext];
    }
    return "application/octet-stream"; // Type par défaut pour fichiers inconnus
}

std::string Response::generate_200_response(const std::string &path)
{
    std::string _body = read_file(path);

    setStatusCode(200);
    setStatusMessage("OK");
    setTime();
    _headers["Content-Type"] = get_content_type(path);
    _headers["Content-Length"] = to_string(_body.length());
    setBody(_body);
    return generate_response();
}

bool has_read_permission(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0)
        return false;
    if (access(path.c_str(), R_OK) != 0)
        return false;
    return true;
}


bool is_acceptable(const Request &request, const std::string &path)
{
    std::string accept = request.getHeaders().at("Accept");
    std::vector<std::string> elements;
    split(accept, ",", elements);
    if (accept.find("*/*") != std::string::npos || accept.empty())
        return true;
    std::string content_type = get_content_type(path);
    if (accept.find(content_type) != std::string::npos)
        return true;
    return false;
}

std::string Response::generate_406_response()
{
    setStatusCode(406);
    setStatusMessage("Not Acceptable");
    setTime();
    _headers["Content-Type"] = "text/html";
    _headers["Content-Length"] = to_string(_body.length());
    setBody("<html><body><h1>406 Not Acceptable</h1></body></html>");
    return generate_response();
}

std::string Response::get_response(const Request &request, const std::string &root)
{
    std::string path = root + request.getUrl();
    std::cout << path << std::endl;
    if (!file_exists(path))
        return (generate_404_response());
    else if (!is_acceptable(request, path))
        return (generate_406_response());
    else if (!has_read_permission(path))
        return (generate_403_response());
    else
        return (generate_200_response(path));
}

std::string Response::post_response(const Request &request, const std::string &root)
{
    (void)request;
    (void)root;
    return ("POST");
}

std::string Response::delete_response(const Request &request, const std::string &root)
{
    (void)request;
    (void)root;
    return ("DELETE");
}

std::string Response::send_response(const Request &request)
{
    if (request.getMethod() == "GET") {
        return (get_response(request, "./static"));
    } 
    else if (request.getMethod() == "POST") {
        return (post_response(request, "www"));
    }
    else
       return (delete_response(request, "www"));
}



Response::~Response()
{
}