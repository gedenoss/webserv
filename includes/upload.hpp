#ifndef UPLOAD_HPP
#define UPLOAD_HPP

#include "request.hpp"
#include <string>

void handleUpload(const Request &request, const std::string &uploadDir);

#endif