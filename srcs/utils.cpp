#include "utils.hpp"

std::string to_string(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

bool file_exists(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

bool ends_with(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length())
        return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

void split(const std::string &s, const std::string &delimiter, std::vector<std::string> &elements)
{
    std::string token;
    std::stringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter[0]))
    {
        elements.push_back(token);
    }
}
