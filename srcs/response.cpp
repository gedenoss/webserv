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
    _status_code = 200;
    _status_message = "";
    _autoindex = false;
    _headers["Date"] = "";
    _headers["Server"] = "Webserv";
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
    _order.push_back("ETag");
    _order.push_back("Last-Modified");
    _order.push_back("Server");
    _order.push_back("Content-Type");
    _order.push_back("Content-Length");
    _order.push_back("Content-Language");

    _body = "";
    _listingDirectory = false;
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
    stat(path.c_str(), &fileStat);
    _headers["Last-Modified"] = formatHttpDate(fileStat.st_mtime);
}

void Response::setEtag(const std::string &path)
{ _headers["ETag"] = generateEtag(path); }

int Response::getStatusCode() const
{ return _status_code; }

std::string Response::getPath() const
{ return _path; }

std::string Response::getStatusMessage() const
{ return _status_message;}

std::map<std::string, std::string> Response::getHeaders() const
{ return _headers; }

std::string Response::getBody() const
{   return _body; }

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
    std::cout << response.str() << std::endl;
    response << "\r\n";
    response << _body;
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

std::string Response::sendFileResponse()
{
    std::string body = "";
    if (_range.isPartial)
    {
        std::ifstream file(_path.c_str(), std::ios::binary);
        if (!file.is_open())
            return "";
        file.seekg(_range.start);
        size_t length = _range.end - _range.start + 1;
        char *buffer = new char[length];
        file.read(buffer, length);
        body = std::string(buffer, length);
        delete[] buffer;
        file.close();
    }
    else
        body = readFile(_path);
    return body;
}

std::string Response::response200(Errors &errors)
{
    if (_listingDirectory == true)
    {
        setStatusCode(200);
        setStatusMessage("OK");
        setTime();
        setBody(_body);
        setHeaders("Content-Type", "text/html");
        setContentLength();
        setContentLanguage();
        return generateResponse();
    }
    std::string _body = sendFileResponse();
    if (_status_code == 201)
        setStatusMessage("Created");
    else if (!_range.isPartial)
    {
        setStatusCode(200);
        setStatusMessage("OK");
    }
    else 
    {
        setStatusCode(206);
        setStatusMessage("Partial Content");
    }
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
    std::string accept = _request.getHeaders().at("Accept");
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

bool Response::isCGI()
{
    std::string extensions[] = {".php", ".py", ".pl", ".cgi"};
    if (_path.find("/scripts/") != std::string::npos)
    {
        for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++)
        {
            if (endsWith(_path, extensions[i]))
                return true;
        }
    }
    return false;
}


Range parseRange(const std::string &rangeHeader, long fileSize)
{
    Range range;
    range.start = 0;
    range.end = fileSize - 1;
    range.isValid = true;
    range.isPartial = false;

    size_t pos = rangeHeader.find("bytes=");
    if (pos == std::string::npos)
        return range;
    pos += 6;

    size_t dashPos = rangeHeader.find("-", pos);
    if (dashPos == std::string::npos)
        return range;

    range.start = stringToSizeT(rangeHeader.substr(pos, dashPos - pos).c_str());

    std::string endStr = rangeHeader.substr(dashPos + 1);
    if (!endStr.empty())
        range.end = stringToSizeT(endStr.c_str());
    else
        range.end = fileSize - 1;

    // if (range.start > range.end || range.end < 0 || range.start > range.end || range.end > static_cast<size_t>(fileSize))
    //     range.isValid = false;

    if (range.start > 0 || range.end < static_cast<size_t> (fileSize - 1))
        range.isPartial = true;

    return range;
}


std::string Response::getResponse(Errors &errors)
{
    if (!fileExists(_path))
        return (errors.error404());
    if (!isAcceptable())
        return (errors.error406());
    if (!hasReadPermission(_path))
        return (errors.error403());
    if (!handleIfModifiedSince(_request.getHeaders()) || isNotModified(_request.getHeaders()))
        return (errors.error304());
    if (isCGI())
    {
        handleCGI();
        return ("CGI");
    }
    struct stat fileStat;

    if (stat(_path.c_str(), &fileStat) < 0)
        return errors.error500();
    size_t fileSize = fileStat.st_size;
    if (_request.getHeaders().count("Range") > 0)
    {
        _range = parseRange(_request.getHeaders().at("Range"), fileSize);
        if (!_range.isValid)
            return (errors.error416());
    }
    return (response200(errors));
}

std::string Response::handleForm(Errors &errors)
{
    std::string body = _request.getBody();
    std::ofstream file(_path.c_str(), std::ios::out);

    if (!file.is_open()) // Vérifier si le fichier ne s'est pas ouvert
    {
        if (access(_path.c_str(), F_OK) == -1) // Vérifie si le fichier existe
            return errors.error404();
        if (access(_path.c_str(), W_OK) == -1) // Vérifie si on a le droit d'écrire
            return errors.error403();
        return errors.error500();
    }

    file << body;
    file.close();
    setStatusCode(201);
    return response200(errors);
}




std::string Response::postResponse(Errors &errors)
{
    std::cout << "Post response" << std::endl;
    if (isCGI())
        handleCGI();
    else if (_request.getHeaders().count("Content-Type") > 0)
    {
        std::cout << "Content type" << _request.getHeaders().at("Content-Type") << std::endl;
        if (_request.getHeaders().at("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos)
            return handleForm(errors);
        else
            return errors.error415();
    }
    return (errors.error415());
}

std::string Response::deleteResponse(Errors &errors)
{
    if (!fileExists(_path))
        return (errors.error404());
    else if (!hasWritePermission(_path))
        return (errors.error403());
    else if (std::remove(_path.c_str()) != 0)
        return (errors.error500());
    else 
        return (response204());
}

std::string findIndex(const std::string &dirPath, const std::string &root)
{
    std::string actualPath = (dirPath == "/") ? root : dirPath;
    const std::string indexFiles[] = {"index.html", "index.htm", "index.php"};
    DIR *dir = opendir(actualPath.c_str());
    if (!dir)
        return "";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        for (size_t i = 0; i < sizeof(indexFiles) / sizeof(indexFiles[0]); i++)
        {
            if (indexFiles[i] == entry->d_name)
            {
                closedir(dir);
                return actualPath + "/" + entry->d_name;
            }
        }
    }
    closedir(dir);
    return "";
}

std::string joinPaths(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    if (b.empty()) return a;

    if (a[a.size() - 1] == '/' && b[0] == '/')
        return a + b.substr(1); // évite double slash
    if (a[a.size() - 1] != '/' && b[0] != '/')
        return a + "/" + b;     // ajoute slash manquant
    return a + b;
}

std::string trimLocationPath(const std::string& url, const std::string& locationPath)
{
    // Si locationPath est vide ou juste "/", on ne touche pas à l'URL
    if (locationPath.empty() || locationPath == "/")
        return url;

    // Si l'URL commence bien par le locationPath
    if (url.find(locationPath) == 0)
    {
        std::string trimmed = url.substr(locationPath.length());

        // Pour éviter un résultat vide, on rajoute "/" si nécessaire
        if (trimmed.empty() || trimmed[0] != '/')
            trimmed = "/" + trimmed;
        return trimmed;
    }

    // Sinon on retourne l'URL d'origine
    return url;
}


void Response::listDirectory()
{
    _listingDirectory = true;
    DIR *dir = opendir(_path.c_str());
    if (dir == NULL)
    {
        if (errno == ENOENT)
            _status_code = 404;
        else if (errno == EACCES)
            _status_code = 403;
        else
            _status_code = 500;
        return;
    }

    std::stringstream html;
    html << "<html><head><title>Index of " << _request.getUrl() << "</title></head><body>";
    html << "<h1>Index of " << _request.getUrl() << "</h1><hr><ul>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        std::string fullPath = joinPaths(_path, name);  // Chemin complet vers le fichier
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0)
        {
            std::string displayName = name;
            if (S_ISDIR(fileStat.st_mode))
                displayName += "/";

            // Générer l'URL relative en utilisant _request.getUrl()
            std::string relativePath = _path;  // URL de base
            if (relativePath[relativePath.length() - 1] != '/')
                relativePath += '/';
            relativePath += name;
            std::cout << "Relative path : " << relativePath << std::endl;
            // Ajout du lien vers le fichier ou dossier
            html << "<li><a href=\"" << displayName << "\">" << displayName << "</a></li>";
        }
    }

    closedir(dir);
    html << "</ul><hr></body></html>\n";
    _body = html.str();
    _status_code = 200;
}



bool Response::tryPath(const std::string& p)
{
    if (fileExists(p))
    {
        _path = p;
        return true;
    }
    return false;
}

void Response::findPath()
{
    std::string path = joinPaths(_root, _request.getUrl());
    std::string trimmed = trimLocationPath(_request.getUrl(), _location.getPath());
    std::string subPath = joinPaths(_root, trimmed);
    bool pathIsDir = isDirectory(path);
    bool subPathIsDir = isDirectory(subPath);

    // 1. Fichier brut
    if (tryPath(path) || tryPath(subPath))
        return;

    // 2. Avec index + autoindex ON
    if (_autoindex && !_index.empty())
    {
        std::string indexPath = joinPaths(path, _index);
        std::string indexSubPath = joinPaths(subPath, _index);
        if (tryPath(indexPath) || tryPath(indexSubPath))
            return;
    }

    // 3. Autoindex activé et pas d'index : afficher un répertoire
    if (_autoindex && _index.empty())
    {
        if (pathIsDir)
        {
            _path = path;
            listDirectory();
            return;
        }
        if (subPathIsDir)
        {
            _path = subPath;
            listDirectory();
            return;
        }
    }

    // 4. Index fourni mais autoindex désactivé → autorisé si index existe
    if (!_index.empty() && !_autoindex)
    {
        std::string indexPath = joinPaths(path, _index);
        std::string indexSubPath = joinPaths(subPath, _index);
        if (tryPath(indexPath) || tryPath(indexSubPath))
            return;
    }

    // 5. Si c’est un dossier mais autoindex désactivé → 403
    if (!_autoindex && _index.empty() && (pathIsDir || subPathIsDir))
    {
        _status_code = 403;
        _path = "";
        return;
    }
    // 6. Sinon : rien trouvé → 404
    _status_code = 404;
    _path = "";
}


std::string Response::sendResponse()
{
    Errors errors(*this);
    if (_request.getErrorCode() != 200)
        return (errors.generateError(_request.getErrorCode()));
    _location = _request.getLocation();
    _root = _location.getRoot();
    _autoindex = _location.getAutoindex();
    _index = _location.getIndex();
    _request.printRequest();
    std::cout << "Root : " << _root << "Index : " << _index << std::endl;
    std::cout << "Location path : " << _location.getPath() << std::endl;
    if (_autoindex)
        std::cout << "Autoindex : true" << std::endl;
    else
        std::cout << "Autoindex : false" << std::endl;
    if (_root.empty())
        _root = ".";
    findPath();
    std::cout << "Path : " << _path << std::endl;
    if (_listingDirectory == true)
        return(response200(errors));
    if (_status_code != 200)
        return (errors.generateError(_status_code));
    if (_request.getMethod() == "GET") {
        return (getResponse(errors));
    } 
    else if (_request.getMethod() == "POST") {
        return (postResponse(errors));
    }
    else
       return (deleteResponse(errors));
}


Response::~Response()
{
}