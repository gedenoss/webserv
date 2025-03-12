#ifndef UTILS_HPP
# define UTILS_HPP

#include "webserv.hpp"
#include "response.hpp"

std::string to_string(int value);
bool ends_with(const std::string &str, const std::string &suffix);
bool file_exists(const std::string &name);
void split(const std::string &s, const std::string &delimiter, std::vector<std::string> &elements);

#endif
