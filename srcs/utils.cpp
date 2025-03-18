#include "utils.hpp"
#include <ctime>
#include <cstring>

std::string toString(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

bool fileExists(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

bool endsWith(const std::string &str, const std::string &suffix)
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

time_t parseHttpDate(const std::string &httpDate)
{
    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(struct tm));
    const char *format = "%a, %d %b %Y %H:%M:%S GMT";

    if (strptime(httpDate.c_str(), format, &timeinfo) == NULL)
        return 0;

    return timegm(&timeinfo);
}

std::string formatHttpDate(time_t timestamp)
{
    char buffer[50];
    struct tm *timeinfo = gmtime(&timestamp);
    strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return std::string(buffer);
}

std::string generateEtag(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0)
    {
        std::stringstream etag;
        etag << "\"" << fileStat.st_size << "-" << fileStat.st_mtime << "\"";
        return (etag.str());
    }
    return "";
}

std::string readFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool compareLanguages(const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
    return a.second > b.second; // or another comparison logic
}

bool hasReadPermission(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0)
        return false;
    if (access(path.c_str(), R_OK) != 0)
        return false;
    return true;
}

bool isDirectory(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0)
        return false;
    return S_ISDIR(fileStat.st_mode);
}
