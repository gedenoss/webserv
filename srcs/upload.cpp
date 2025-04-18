#include "../includes/request.hpp"
#include "../includes/response.hpp"
#include "../includes/utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

// Fonction pour extraire le boundary à partir du Content-Type
std::string extractBoundary(const std::string &contentType) {
    const std::string boundaryPrefix = "boundary=";
    size_t pos = contentType.find(boundaryPrefix);
    if (pos != std::string::npos) {
        return "--" + contentType.substr(pos + boundaryPrefix.length());
    }
    return "";
}

// Fonction pour gérer l'upload
void handleUpload(const Request &request, const std::string &uploadDir) {
    std::string contentType = request.getHeaders().at("Content-Type");
    std::string boundary = extractBoundary(contentType);
    if (boundary.empty()) throw std::runtime_error("No boundary");

    // Stocker le body en binaire
    const std::string &bodyStr = request.getBody();
    std::vector<char> body(bodyStr.begin(), bodyStr.end());

    std::string fullBoundary = boundary;
    std::string endBoundary = boundary + "--";

    size_t pos = 0;
    while (true) {
        // Chercher le prochain boundary
        size_t boundaryStart = std::search(body.begin() + pos, body.end(), fullBoundary.begin(), fullBoundary.end()) - body.begin();
        if (boundaryStart == body.size()) break;
        pos = boundaryStart + fullBoundary.length();

        // Chercher la fin des headers de la part
        std::string headerEndSeq = "\r\n\r\n";
        size_t headerEnd = std::search(body.begin() + pos, body.end(), headerEndSeq.begin(), headerEndSeq.end()) - body.begin();
        if (headerEnd == body.size()) break;

        std::string partHeaders(body.begin() + pos, body.begin() + headerEnd);
        pos = headerEnd + 4;

        if (partHeaders.find("filename=\"") != std::string::npos) {
            // Extraire le nom de fichier
            size_t filenameStart = partHeaders.find("filename=\"") + 10;
            size_t filenameEnd = partHeaders.find("\"", filenameStart);
            std::string filename = partHeaders.substr(filenameStart, filenameEnd - filenameStart);

            // Chercher le prochain boundary
            size_t partEnd = std::search(body.begin() + pos, body.end(), fullBoundary.begin(), fullBoundary.end()) - body.begin();
            if (partEnd == body.size()) break;

            // Retirer les \r\n de fin de section
            size_t fileEnd = partEnd;
            if (fileEnd >= 2 && body[fileEnd - 2] == '\r' && body[fileEnd - 1] == '\n')
                fileEnd -= 2;

            // Écrire le fichier
            std::string filepath = uploadDir + "/" + filename;
            std::ofstream out(filepath.c_str(), std::ios::binary);
            out.write(&body[pos], fileEnd - pos);
            out.close();

            std::cout << "✅ Uploaded: " << filename << " (" << (fileEnd - pos) << " bytes)\n";
            pos = partEnd;
        }
    }
}
