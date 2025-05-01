#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

std::string extractBoundary(const std::string &contentType) {
    const std::string boundaryPrefix = "boundary=";
    size_t pos = contentType.find(boundaryPrefix);
    if (pos != std::string::npos) {
        return "--" + contentType.substr(pos + boundaryPrefix.length());
    }
    return "";
}

std::string Response::handleUpload(Errors &errors)
{
    try {
        std::string body = _request.getBody();
        std::string boundary = "--" + _request.getHeaders().at("Content-Type").substr(
            _request.getHeaders().at("Content-Type").find("boundary=") + 9);

        size_t pos = 0;
        while ((pos = body.find(boundary, pos)) != std::string::npos)
        {
            pos += boundary.length();
            size_t endPos = body.find(boundary, pos);
            if (endPos == std::string::npos)
                break;

            std::string part = body.substr(pos, endPos - pos);
            size_t fileNamePos = part.find("filename=\"");
            if (fileNamePos != std::string::npos)
            {
                size_t fileNameEnd = part.find("\"", fileNamePos + 10);
                std::string fileName = part.substr(fileNamePos + 10, fileNameEnd - (fileNamePos + 10));
                size_t fileContentStart = part.find("\r\n\r\n", fileNameEnd) + 4;
                size_t fileContentEnd = part.rfind("\r\n", endPos);
                std::string fileContent = part.substr(fileContentStart, fileContentEnd - fileContentStart);
                char buffer [PATH_MAX];
                std::string cwd = getcwd(buffer, sizeof(buffer)) ? std::string(buffer) : "";
                if (cwd.empty())
                    throw std::runtime_error("Failed to get current working directory");
                std::string fullPath = cwd + "/upload/" + fileName;
                std::ofstream outFile((fullPath).c_str(), std::ios::binary);
                if (!outFile.is_open())
                    throw std::runtime_error("Failed to open file for writing");
                outFile.write(fileContent.c_str(), fileContent.size());
                outFile.close();
            }
            pos = endPos;
        }

        setStatusCode(201);
        setStatusMessage("Created");
        setHeaders("Content-Type", "text/plain");
        setBody("File uploaded successfully.");
        return generateResponse();
    } catch (const std::exception &e) {
        return errors.error500();
    }
}