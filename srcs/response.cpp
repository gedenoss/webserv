#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

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

bool Response::isCGI()
{
    std::string extension = _path.substr(_path.find_last_of("."), std::string::npos);
    std::map<std::string, std::string>::const_iterator it = _location.getCgi().find(extension);
    if (it != _location.getCgi().end())
    {
        _cgiBinPath = it->second;
        std::cout << "CGI bin path : " << _cgiBinPath << std::endl;
        _getBody = false;
        return true;
    }
    std::cout << "Is not a CGI " << std::endl;
    return false;
}

std::string Response::response200(Errors &errors)
{
    setStatusCodeAndMessage();
    
    if (isContentLanguageEmpty()) {
        return errors.error400();
    }
    
    cleanUpCgiFiles();

    return generateResponse();
}

void Response::setStatusCodeAndMessage()
{
    if (_status_code == 201) {
        setStatusMessage("Created");
    } else if (!_range.isPartial) {
        setStatusCode(200);
        setStatusMessage("OK");
    } else {
        setStatusCode(206);
        setStatusMessage("Partial Content");
    }
}

bool Response::isContentLanguageEmpty()
{
    return _headers["Content-Language"].empty();
}

void Response::cleanUpCgiFiles()
{
    if (!_cgiInfilePath.empty()) {
        remove(_cgiInfilePath.c_str());
        _cgiInfilePath.clear();
    }
    if (!_cgiOutfilePath.empty()) {
        remove(_cgiOutfilePath.c_str());
        _cgiOutfilePath.clear();
    }
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
    if (range.start > range.end || range.end < 0 || range.start > range.end || range.end > static_cast<size_t>(fileSize))
        range.isValid = false;
    if (range.start > 0 || range.end < static_cast<size_t> (fileSize - 1))
        range.isPartial = true;
    return range;
}

std::string Response::generateResponseCgi()
{
    std::stringstream response;
    if (_range.isPartial)
        _status_code = 206; // HTTP 206 Partial Content
    response << "HTTP/1.1 " << _status_code << " " << (_status_code == 206 ? "Partial Content" : "OK") << "\r\n";
    response << _headerCgi << "\r\n";
    if (_range.isPartial)
    {
        size_t realEnd = std::min(_range.end, _body.size() > 0 ? _body.size() - 1 : 0);
        response << "Content-Range: bytes "
                 << _range.start << "-" << realEnd << "/" << _body.size() << "\r\n";
    }
    response << "\r\n";
    if (_range.isPartial && _range.start < _body.size())
    {
        size_t realEnd = std::min(_range.end, _body.size() - 1);
        std::string partialBody = _body.substr(_range.start, realEnd - _range.start + 1);
        response << partialBody;
    }
    else
        response << _body;
    return response.str();
}

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
    if (handleIfModifiedSince(_request.getHeaders()) || isNotModified(_request.getHeaders()))
    {
        setStatusCode(304);
        return true;
    }
    return false;
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

std::string Response::getResponse(Errors &errors)
{
    if (handleRange())
        return (errors.generateError(_status_code));
    if (isCGI())
    {
        handleCGI(errors);
        if (_status_code != 0)
            return (errors.generateError(_status_code));
        return (generateResponseCgi());
    }
    if (handleFileErrors())
        return (errors.generateError(_status_code));
    if (_getBody == true)
        _body = sendFileResponse();
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
    {
        handleCGI(errors);
        if (_status_code != 200)
        return (errors.generateError(_status_code));
    }
    else if (_request.getHeaders().count("Content-Type") > 0)
    {
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

std::string Response::sendResponse()
{
    Errors errors(*this);
    if (_request.getErrorCode() != 200)
        return (errors.generateError(_request.getErrorCode()));
    setInfoRequest();
    findPath();
    if (_path.empty())
        return (errors.error404());
    if (_request.getMethod() == "GET") {
        return (getResponse(errors));
    } 
    else if (_request.getMethod() == "POST") {
        return (postResponse(errors));
    }
    else
        return (deleteResponse(errors));
}

