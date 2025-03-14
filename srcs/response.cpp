#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

Response::Response(Request &req) : request(req)
{
    _status_code = 0;
    _status_message = "";
    _headers["Date"] = "";
    _headers["Server"] = "Webserv";
    _headers["Content-Type"] = "";
    _headers["Content-Length"] = "";
    _headers["Content-Encoding"] = "identity";
    _headers["Content-Language"] = "";
    _headers["Connection"] = "close";
    _available_languages.push_back("en-US");
    _available_languages.push_back("fr");
    _available_languages.push_back("ar-sa");
    _available_languages.push_back("es-ES");
    _available_languages.push_back("de-DE");
    _available_languages.push_back("en-GB");
    _available_languages.push_back("nl");
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

void Response::setStatusCode(int status_code)
{
    _status_code = status_code;
}

void Response::setStatusMessage(const std::string &status_message)
{
    _status_message = status_message;
}

void Response::setHeaders(const std::string &key, const std::string &value)
{
    _headers[key] = value;
}

void Response::setBody(const std::string &body)
{
    _body = body;
}

int Response::getStatusCode() const
{
    return _status_code;
}

std::string Response::getStatusMessage() const
{
    return _status_message;
}

std::map<std::string, std::string> Response::getHeaders() const
{
    return _headers;
}

std::string Response::getBody() const
{
    return _body;
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

std::string read_file(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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

bool compareLanguages(const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
    return a.second > b.second; // or another comparison logic
}

std::string Response::get_language()
{
    std::string accept_language = request.getHeaders().at("Accept-Language");
    if (accept_language.empty())
        return "en-US";
    std::vector<std::pair<std::string, double> > languages;
    std::istringstream ss(accept_language);
    std::string token;
    while (std::getline(ss, token, ','))
    {
        size_t q_pos = token.find("q=");
        double q_value = 1.0;
        if (q_pos != std::string::npos)
        {
            q_value = std::atof(token.substr(q_pos + 2).c_str());
            if (q_value > 1.0 || q_value < 0.0)
                return "";
            token = token.substr(0, q_pos - 1);
        }
        token.erase(std::remove(token.begin(), token.end(), ' '), token.end());
        languages.push_back(std::make_pair(token, q_value));
    }
    std::sort(languages.begin(), languages.end(), compareLanguages);
    for (size_t i = 0; i < languages.size(); i++)
    {
        std::string lang = languages[i].first;
        std::vector<std::string>::iterator it = std::find(_available_languages.begin(), _available_languages.end(), lang);
        int found = (it != _available_languages.end()) ? std::distance(_available_languages.begin(), it) : -1;
        if (found != -1)
        {
            return lang; // Langue trouvée
        }
    }
    return "en-US"; // Langue par défaut
}


std::string Response::generate_200_response(const std::string &path, Errors &errors)
{
    std::string _body = read_file(path);

    setStatusCode(200);
    setStatusMessage("OK");
    setTime();
    _headers["Content-Type"] = get_content_type(path);
    _headers["Content-Length"] = to_string(_body.length());
    _headers["Content-Language"] = get_language();
    if (_headers["Content-Language"] == "")
        return errors.generate_400_response();
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

bool Response::is_acceptable(const std::string &path)
{
    std::string accept = request.getHeaders().at("Accept");
    std::vector<std::string> elements;
    split(accept, ",", elements);
    if (accept.find("*/*") != std::string::npos || accept.empty())
        return true;
    std::string content_type = get_content_type(path);
    std::size_t pos = content_type.find("/");
    std::string type_content = content_type.substr(0, pos);
    std::string subtype_content = content_type.substr(pos + 1);
    for (std::vector<std::string>::iterator it = elements.begin(); it != elements.end(); ++it)
    {
        std::string element = *it;
        std::size_t pos = element.find("/");
        if (pos != std::string::npos)
        {
            std::string type = element.substr(0, pos);
            std::string subtype = element.substr(pos + 1);
            if (type == "*")
            {
                if (subtype == "*")
                    return true;
                else if (subtype == subtype_content)
                    return true;
            }
            else if (type == type_content)
            {
                if (subtype == "*")
                    return true;
                else if (subtype == subtype_content)
                    return true;
            }
        }
    }
    return false;
}


std::string Response::get_response(const Request &request, const std::string &root)
{
    Errors errors(*this);
    std::string path = root + request.getUrl();
    if (!file_exists(path))
        return (errors.generate_404_response());
    else if (!is_acceptable(path))
        return (errors.generate_406_response());
    else if (!has_read_permission(path))
        return (errors.generate_403_response());
    else
        return (generate_200_response(path, errors));
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

std::string Response::send_response(const Request &given_request)
{
    request = given_request;
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