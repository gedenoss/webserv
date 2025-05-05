#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

Response::Response(Request &req, ServerConfig &serv) : _request(req), _server(serv)
{
    _range.start = 0;
    _range.end = 0;
    _range.isValid = true;
    _range.isPartial = false;
    _status_code = 0;
    _status_message = "";
    _autoindex = false;
    _listingDirectory = false;
    _headers["Date"] = "";
    _headers["Server"] = _server.getServerName();
    _headers["Content-Type"] = "";
    _headers["Content-Length"] = "";
    _headers["Content-Language"] = "";
    _headers["Connection"] = "close";
    _available_languages.push_back("en-US");
    _available_languages.push_back("fr-FR");
    _available_languages.push_back("fr");
    _available_languages.push_back("ar-sa");
    _available_languages.push_back("es-ES");
    _available_languages.push_back("de-DE");
    _available_languages.push_back("en-GB");
    _available_languages.push_back("nl");

    _order.push_back("Connection");
    _order.push_back("Date");
    if (_request.getMethod() == "GET")
    {
        _order.push_back("ETag");
        _order.push_back("Last-Modified");
    }
    _order.push_back("Server");
    _order.push_back("Content-Type");
    _order.push_back("Content-Length");
    _order.push_back("Content-Language");
    _order.push_back("Location");

    _getBody = true;
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
{ _status_code = status_code; }

void Response::setStatusMessage(const std::string &status_message)
{ _status_message = status_message; }

void Response::setHeaders(const std::string &key, const std::string &value)
{ _headers[key] = value; }

void Response::setBody(const std::string &body)
{ _body = body; }

void Response::setContentType()
{ _headers["Content-Type"] = getContentType(); }

void Response::setContentLength()
{  _headers["Content-Length"] = toString(_body.length()); }

void Response::setContentLanguage()
{ _headers["Content-Language"] = getLanguage(); }

void Response::setLastModified(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) {
        _headers["Last-Modified"] = formatHttpDate(fileStat.st_mtime);
    } else {
        _headers["Last-Modified"] = "Thu, 01 Jan 1970 00:00:00 GMT";
    }
}


void Response::setEtag(const std::string &path)
{ _headers["ETag"] = generateEtag(path); }


void Response::setStatusCodeAndMessage()
{
    if (_status_code == 204)
    {
        setStatusMessage("No Content");
        setTime();
        setEtag(_path);
        setContentLanguage();
    }
    else if (_status_code == 201)
    { 
        setStatusMessage("Created");
        setHeadersForResponse();
    }
    else if (_status_code == 304) {
        setStatusMessage("Not Modified");
        setHeadersForResponse();
    }
    else if (!_range.isPartial) {
        setStatusCode(200);
        setStatusMessage("OK");
        setHeadersForResponse();
    } else {
        setStatusCode(206);
        setStatusMessage("Partial Content");
        setHeadersForResponse();
    }
}

void Response::setHeadersForResponse()
{
    setTime();
    setBody(_body);
    setContentType();
    setContentLength();
    setContentLanguage();
    setLastModified(_path);
    if (_request.getMethod() == "GET")
        setEtag(_path);
}



void Response::setInfoRequest()
{
    _location = _request.getLocation();
    _root = _location.getRoot();
    _autoindex = _location.getAutoindex();
    _index = _location.getIndex();
    if (_root.empty())
        _root = ".";
}

std::string Response::getContentType()
{
    std::map<std::string, std::string> mime_types;
    if (_listingDirectory == true)
    {
        _listingDirectory = false;
        return "text/html";
    }
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
    return "text/plain"; // Type par défaut pour fichiers inconnus
}

std::string Response::getLanguage()
{
    std::string accept_language = "";
    std::map<std::string, std::string>::iterator it = _request.getHeaders().find("Accept-Language");
    if (it != _request.getHeaders().end()) {
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

Response::~Response()
{
}