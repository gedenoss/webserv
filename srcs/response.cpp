#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"
#include "../includes/upload.hpp"


// Functions to generate a response string
std::string Response::generateResponse(bool isError)
{
    std::stringstream response;
    if (isError == true)
        std::cout << RED << BOLD << "HTTP/1.1 " << _status_code << " " << _status_message << RESET << std::endl;
    else
    std::cout << GREEN << BOLD << "HTTP/1.1 " << _status_code << " " << _status_message << RESET << std::endl;
    response << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";
    for(std::vector<std::string>::iterator it = _order.begin(); it != _order.end(); ++it)
    {
        if (_headers.find(*it) != _headers.end() && !_headers[*it].empty())
        {
            response << *it << ": " << _headers[*it] << "\r\n";
            if (isError == true)
                std::cout << RED << *it << ": " << _headers[*it] << RESET << "\r\n";
            else
                std::cout << GREEN << *it << ": " << _headers[*it] << RESET << "\r\n";
        }
    }
    std::cout << std::endl;
    response << "\r\n";
    if (_status_code != 304)
    {
        response << _body;
    }
    return response.str();
}

std::string Response::sendFileResponse()
{
    std::string body = "";
    if (_range.isPartial)
        body = readPartialFile(_path, _range);
    else
        body = readFile(_path);
    return body;
}

std::string Response::validResponse(Errors &errors)
{
    setStatusCodeAndMessage();
    if (isContentLanguageEmpty()) {
        return errors.error400();
    }
    cleanUpCgiFiles();
    return generateResponse(false);
}

// Fonctions to check headers
bool Response::isContentLanguageEmpty() {
    return _headers["Content-Language"].empty();
}

bool Response::isAcceptable()
{
    std::map<std::string, std::string> headers = _request.getHeaders();

    if (headers.find("Accept") == headers.end())
        return true;

    std::string accept = headers["Accept"];
    std::vector<std::string> elements;
    split(accept, ",", elements);

    if (accept.find("*/*") != std::string::npos || accept.empty())
        return true;

    std::string content_type = getContentType();
    std::size_t pos = content_type.find("/");
    if (pos == std::string::npos)
        return false;

    std::string type_content = content_type.substr(0, pos);
    std::string subtype_content = content_type.substr(pos + 1);

    for (std::vector<std::string>::iterator it = elements.begin(); it != elements.end(); ++it)
    {
        std::string element = trim(*it);
        std::size_t slash = element.find("/");

        if (slash != std::string::npos)
        {
            std::string type = trim(element.substr(0, slash));
            std::string subtype = trim(element.substr(slash + 1));

            if (type == "*")
            {
                if (subtype == "*" || subtype == subtype_content)
                    return true;
            }
            else if (type == type_content)
            {
                if (subtype == "*" || subtype == subtype_content)
                    return true;
            }
        }
    }

    return false;
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

//Functions to handle range
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
    if (range.start < 0 || range.end < 0 || range.start > range.end || range.end >= fileSize)
        range.isValid = false;
    if (range.start > 0 || range.end < (fileSize - 1))
        range.isPartial = true;
    return range;
}

bool Response::handleRange()
{
    struct stat fileStat;
    if (stat(_path.c_str(), &fileStat) < 0)
    {
        setStatusCode(500);
        return true;
    }
    size_t fileSize = fileStat.st_size;
    if (_request.getHeaders().count("Range") > 0)
    {
        _range = parseRange(_request.getHeaders().at("Range"), fileSize);
        if (!_range.isValid)
        {
            setStatusCode(416);
            return true;
        }
    }
    return false;
}

//Fonction to handle POST form request
std::string Response::handleForm(Errors &errors)
{
    std::string body = _request.getBody();
    // std::cout << "FILE FORM" << std::endl;
    _path = _path.substr(_path.find_last_of("/") + 1); // On garde juste le nom du fichier
    std::string filePath = "upload/" + _path; // _path contient juste le nom du fichier attendu
    struct stat fileStat;
    bool fileExists = (stat(filePath.c_str(), &fileStat) == 0);

    if (fileExists)
    {
        // Si ce n’est pas un fichier régulier, erreur 403
        if (!S_ISREG(fileStat.st_mode))
            return errors.error403();

        // Si pas les droits en écriture, erreur 403
        if (access(filePath.c_str(), W_OK) == -1)
            return errors.error403();
    }
    else
    {
        // Vérifie que le dossier "upload" existe et est accessible en écriture
        if (access("upload", F_OK) == -1 || access("upload", W_OK) == -1)
            return errors.error403();
    }

    // Ouvrir le fichier (append si existe, sinon out)
    std::ofstream file(filePath.c_str(), fileExists ? std::ios::app : std::ios::out);
    if (!file.is_open())
        return errors.error500();

    file << body << std::endl; // Ajout d'un saut de ligne pour séparer les soumissions
    file.close();

    setStatusCode(201);
    return validResponse(errors);
}

//Fonction to handle errors
bool Response::handleFileErrors()
{
    if (!fileExists(_path))
    {
        setStatusCode(404);
        return true;
    }
    if (!isAcceptable())
    {
        setStatusCode(406);
        return true;
    }
    if (!hasReadPermission(_path))
    {
        setStatusCode(403);
        return true;
    }
    // if (handleIfModifiedSince(_request.getHeaders()) || isNotModified(_request.getHeaders()))
    // {
    //     setStatusCode(304);
    //     return false;
    // }
    return false;
}

std::string Response::jsonListFiles(Errors &errors)
{
    DIR *dir = opendir("upload");
    if (!dir)
    {
        if (errno == ENOENT)
            return errors.error404();
        else if (errno == EACCES)
            return errors.error403();
        else
            return errors.error500();
    }

    std::stringstream json;
    json << "[";
    struct dirent *entry;
    bool first = true;

    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        if (!first)
            json << ",";
        json << "\"" << name << "\"";
        first = false;
    }

    closedir(dir);
    json << "]";
    _status_code = 200;
    return json.str();
}

std::string Response::getResponse(Errors &errors)
{
    //Check all the errors
    if (handleFileErrors())
        return (errors.generateError(_status_code));
    if (handleRange())
        return (errors.generateError(_status_code));
    //CGI handling
    if (isCGI())
    {
        handleCGI(errors);
        if (_status_code != 0)
            return (errors.generateError(_status_code));
        return (generateResponseCgi());
    }
    _body = sendFileResponse();
    return (validResponse(errors));
}

std::string Response::postResponse(Errors &errors)
{
    if (isCGI())
    {
        handleCGI(errors);
        if (_status_code != 0)
            return (errors.generateError(_status_code));
        return (generateResponseCgi());
    }
    else if (_request.getHeaders().count("Content-Type") > 0)
    {
        const std::string &contentType = _request.getHeaders().at("Content-Type");
        if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
            return handleForm(errors);
        else if (contentType.find("multipart/form-data") != std::string::npos)
            return handleUpload(errors);
        else
            return errors.error415();
    }
    return errors.error415();
}

std::string Response::deleteResponse(Errors &errors)
{
    if (!fileExists(_path))
        return (errors.error404());
    else if (!hasWritePermission(_path))
        return (errors.error403());
    else if (std::remove(_path.c_str()) != 0)
        return (errors.error500());
    _status_code = 204;
    return (validResponse(errors));
    
}

std::string Response::sendResponse()
{
    setInfoRequest();
    Errors errors(*this);
    if (_request.getErrorCode() != 200)
        return (errors.generateError(_request.getErrorCode()));
    if (_request.getUrl() == "/upload/listing")
    {
        _body = jsonListFiles(errors);
        setStatusCode(200);
        return (validResponse(errors));
    }
    findPath();
    if (_location.getHasReturn() == true)
    {
        setStatusCode(307);
        return (errors.generateError(_status_code));
    }
    if (_listingDirectory == true)
    {
        return (validResponse(errors));
    }
    if (_status_code != 0)
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

