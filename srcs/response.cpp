#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"
#include "dirent.h"

Response::Response(Request &req) : request(req)
{
    _status_code = 0;
    _status_message = "";
    _headers["Date"] = "";
    _headers["Server"] = "Webserv";
    _headers["Content-Type"] = "";
    _headers["Content-Length"] = "";
    _headers["Content-Language"] = "";
    _headers["Connection"] = "close";
    _available_languages.push_back("en-US");
    _available_languages.push_back("fr");
    _available_languages.push_back("ar-sa");
    _available_languages.push_back("es-ES");
    _available_languages.push_back("de-DE");
    _available_languages.push_back("en-GB");
    _available_languages.push_back("nl");

    _order.push_back("Connection");
    _order.push_back("Date");
    _order.push_back("ETag");
    _order.push_back("Last-Modified");
    _order.push_back("Server");
    _order.push_back("Content-Type");
    _order.push_back("Content-Length");
    _order.push_back("Content-Language");

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

std::string Response::generateResponse()
{
    std::stringstream response;
    response << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";
    for(std::vector<std::string>::iterator it = _order.begin(); it != _order.end(); ++it)
    {
        if (_headers.find(*it) != _headers.end())
            response << *it << ": " << _headers[*it] << "\r\n";
    }
    response << "\r\n";
    response << _body;
    std::cout << response.str() << std::endl;
    return response.str();
}

std::string getContentType(const std::string &path)
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

std::string Response::getLanguage()
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


std::string Response::response200(const std::string &path, Errors &errors)
{
    std::string _body = readFile(path);

    setStatusCode(200);
    setStatusMessage("OK");
    setTime();
    _headers["Content-Type"] = getContentType(path);
    _headers["Content-Length"] = toString(_body.length());
    _headers["Content-Language"] = getLanguage();
    if (_headers["Content-Language"] == "")
        return errors.error400();
    setBody(_body);
    return generateResponse();
}

bool isAcceptable(const std::string &path, const Request &request)
{
    std::string accept = request.getHeaders().at("Accept");
    std::vector<std::string> elements;
    split(accept, ",", elements);
    if (accept.find("*/*") != std::string::npos || accept.empty())
        return true;
    std::string content_type = getContentType(path);
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

bool isModifiedSince(const std::string &path, const std::string &ifModifiedSince)
{
    struct stat fileStat;

    if (stat(path.c_str(), &fileStat) != 0)
        return true;
    time_t client_time = parseHttpDate(ifModifiedSince);
    if (client_time == 0)
        return true;
    return fileStat.st_mtime > client_time;
}

bool handleIfModifiedSince(const std::string &path, const std::map<std::string, std::string> &headers)
{
    std::map<std::string, std::string>::const_iterator it = headers.find("If-Modified-Since");
    if (it == headers.end())
        return true;
    if (!isModifiedSince(path, it->second))
        return false;
    return true;
}

bool isNotModified(const std::string &path, const std::map<std::string, std::string> &headers)
{
    std::string etag = generateEtag(path);
    if (headers.count("If-None-Match") > 0)
    {
        std::string client_etag = headers.at("If-None-Match");
        if (client_etag == etag)
            return true;
    }
    return false;
}

bool isDirectory(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0)
        return false;
    if (S_ISDIR(fileStat.st_mode))
    {
        DIR *dir = opendir(path.c_str());
        if (dir == NULL)
            return false;
        struct dirent *entry;
        for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
        {
        }
    }
    return false;
}

std::string Response::getResponse(const Request &request, const std::string &root)
{
    Errors errors(*this);
    std::string path = root + request.getUrl();
    if (!fileExists(path))
        return (errors.error404());

    struct stat fileStat;
    stat(path.c_str(), &fileStat);
    this->setHeaders("Last-Modified", formatHttpDate(fileStat.st_mtime));
    this->setHeaders("Etag", generateEtag(path));

    if (!isAcceptable(path, this->request))
        return (errors.error406());
    if (!hasReadPermission(path) || !isDirectory(path))
        return (errors.error403());
    if (!handleIfModifiedSince(path, request.getHeaders()) || isNotModified(path, request.getHeaders()))
        return (errors.error304());
    return (response200(path, errors));
}

std::string Response::postResponse(const Request &request, const std::string &root)
{
    (void)request;
    (void)root;
    return ("POST");
}

std::string Response::deleteResponse(const Request &request, const std::string &root)
{
    (void)request;
    (void)root;
    return ("DELETE");
}

std::string Response::sendResponse(const Request &given_request)
{
    request = given_request;
    if (request.getMethod() == "GET") {
        return (getResponse(request, "./static"));
    } 
    else if (request.getMethod() == "POST") {
        return (postResponse(request, "www"));
    }
    else
       return (deleteResponse(request, "www"));
}


Response::~Response()
{
}