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

void Response::setContentType()
{
    _headers["Content-Type"] = getContentType();
}

void Response::setContentLength()
{
    _headers["Content-Length"] = toString(_body.length());
}

void Response::setContentLanguage()
{
    _headers["Content-Language"] = getLanguage();
}

void Response::setLastModified(const std::string &path)
{
    struct stat fileStat;
    stat(path.c_str(), &fileStat);
    _headers["Last-Modified"] = formatHttpDate(fileStat.st_mtime);
}

void Response::setEtag(const std::string &path)
{
    _headers["ETag"] = generateEtag(path);
}

int Response::getStatusCode() const
{
    return _status_code;
}

std::string Response::getPath() const
{
    return _path;
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
    std::cout << std::endl;
    response << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";
    for(std::vector<std::string>::iterator it = _order.begin(); it != _order.end(); ++it)
    {
        if (_headers.find(*it) != _headers.end() && !_headers[*it].empty())
            response << *it << ": " << _headers[*it] << "\r\n";
    }
    response << "\r\n";
    response << _body;
    std::cout << response.str() << std::endl;
    return response.str();
}

std::string Response::getContentType()
{
    std::map<std::string, std::string> mime_types;
    
    mime_types[".html"] = "text/html";
    mime_types[".css"] = "text/css";
    mime_types[".js"] = "application/javascript";
    mime_types[".png"] = "image/png";
    mime_types[".jpg"] = "image/jpeg";
    mime_types[".jpeg"] = "image/jpeg";
    mime_types[".gif"] = "image/gif";
    mime_types[".php"] = "text/html";
    mime_types[".json"] = "application/json";
    mime_types[".xml"] = "application/xml";
    mime_types[".pdf"] = "application/pdf";
    mime_types[".txt"] = "text/plain";
    mime_types[".csv"] = "text/csv";
    mime_types[".xhtml"] = "text/xhtml";

    size_t dot_pos = _path.find_last_of(".");
    if (dot_pos != std::string::npos) {
        std::string ext = _path.substr(dot_pos);
        if (mime_types.count(ext))
            return mime_types[ext];
    }
    return "application/octet-stream"; // Type par défaut pour fichiers inconnus
}

std::string Response::getLanguage()
{
    std::string accept_language = "";
    std::map<std::string, std::string>::iterator it = request.getHeaders().find("Accept-Language");
    if (it != request.getHeaders().end()) {
        accept_language = it->second;
    }
    if (accept_language.empty()) {
        return "en-US";
    }
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


std::string Response::response200(Errors &errors)
{
    std::string _body = readFile(_path);

    setStatusCode(200);
    setStatusMessage("OK");
    setTime();
    setBody(_body);
    setContentType();
    setContentLength();
    setContentLanguage();
    setLastModified(_path);
    setEtag(_path);
    if (_headers["Content-Language"] == "")
        return errors.error400();
    return generateResponse();
}

std::string Response::response204()
{
    setStatusCode(204);
    setStatusMessage("No Content");
    setTime();
    setEtag(_path);
    return generateResponse();
}

bool Response::isAcceptable()
{
    std::string accept = request.getHeaders().at("Accept");
    std::vector<std::string> elements;
    split(accept, ",", elements);
    if (accept.find("*/*") != std::string::npos || accept.empty())
        return true;
    std::string content_type = getContentType();
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

bool Response::isModifiedSince(const std::string &ifModifiedSince)
{
    struct stat fileStat;

    if (stat(_path.c_str(), &fileStat) != 0)
        return true;
    time_t client_time = parseHttpDate(ifModifiedSince);
    if (client_time == 0)
        return true;
    return fileStat.st_mtime > client_time;
}

bool Response::handleIfModifiedSince(const std::map<std::string, std::string> &headers)
{
    std::map<std::string, std::string>::const_iterator it = headers.find("If-Modified-Since");
    if (it == headers.end())
        return true;
    if (!isModifiedSince(it->second))
        return false;
    return true;
}

bool Response::isNotModified(const std::map<std::string, std::string> &headers)
{
    std::string etag = generateEtag(_path);
    if (headers.count("If-None-Match") > 0)
    {
        std::string client_etag = headers.at("If-None-Match");
        if (client_etag == etag)
            return true;
    }
    return false;
}

bool Response::handleDirectory()
{
    if (!isDirectory(_path))
        return true;
    // en fait ici il faudrait récupérer le path donné dans le fichier de config qui me donne
    // le fichier par défaut si c'est un directory
    // gérer les répertoires interdits
    return false;
}

bool Response::isCGI()
{
    std::string extensions[] = {".php", ".py", ".pl", ".cgi"};
    for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++)
    {
        if (endsWith(_path, extensions[i]))
            return true;
    }
    return false;
}
void Response::handleCGI()
{
    std::cout << "CGI" << std::endl;
}

std::string Response::handleUpload(Errors &errors)
{
    if (request.getHeaders().count("Content-Type") == 0)
        return (errors.error400());
    std::string contentType = request.getHeaders().at("Content-Type");
    // size_t boundaryPos = contentType.find("boundary=");
    // if (boundaryPos == std::string::npos)
    //     return errors.error400();
    std::string lengthStr = request.getHeaders().at("Content-Length");

    if (contentType.find("multipart/form-data") == std::string::npos)
        return errors.error400();
    size_t pos = contentType.find("boundary=");
    std::string boundary;
    if (pos != std::string::npos)
        boundary = "--" + _headers["Content-Type"].substr(pos + 9);
    else
        return errors.error400();
    char* endPtr;
    double len = std::strtod(lengthStr.c_str(), &endPtr);
    if (*endPtr != '\0' || len < 0)
        return errors.error500();
    if (len > MAX_FILE_SIZE)
        return errors.error507();
    std::string body = request.getBody();
    if (body.empty())
        return errors.error400();
    if (body.length() != len)
        return errors.error500();
    std::ofstream file("upload.txt", std::ios::binary);
    file.write(body.c_str(), body.length());
    file.close();
    return "";
}

std::string Response::getResponse(const Request &request, Errors &errors, const std::string &root)
{
    if (request.getUrl() == "/favicon.ico" || request.getUrl() == "/")
        _path = "./index.html";
    else 
        _path = root + request.getUrl();
    std::cout << _path << std::endl;
    if (!fileExists(_path))
        return (errors.error404());
    if (!isAcceptable())
        return (errors.error406());
    if (!hasReadPermission(_path) ||!handleDirectory())
        return (errors.error403());
    // if (!handleIfModifiedSince(request.getHeaders()) || isNotModified(request.getHeaders()))
    //     return (errors.error304());
    if (isCGI())
        handleCGI();
    return (response200(errors));
}

std::string Response::postResponse(const Request &request, Errors &errors, const std::string &root)
{
    _path = root + request.getUrl();
    if (isCGI())
        handleCGI();
    else if (request.getUrl() == "/upload")
    {
        handleUpload(errors);
    }
    else
        errors.error415();
    return ("POST");
}

std::string Response::deleteResponse(const Request &request, Errors &errors, const std::string &root)
{
    _path = root + request.getUrl();
    if (!fileExists(_path))
        return (errors.error404());
    else if (!hasWritePermission(_path))
        return (errors.error403());
    else if (std::remove(_path.c_str()) != 0)
        return (errors.error500());
    else 
        return (response204());
}

std::string Response::sendResponse(const Request &given_request)
{
    Errors errors(*this);
    request = given_request;
    if (request.getMethod() == "GET") {
        return (getResponse(request, errors, "."));
    } 
    else if (request.getMethod() == "POST") {
        return (postResponse(request, errors, "."));
    }
    else
       return (deleteResponse(request, errors, "."));
}


Response::~Response()
{
}