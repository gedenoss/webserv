#include "utils.hpp"
#include <ctime>
#include <cstring>
#include <sstream>

std::string toString(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

size_t stringToSizeT(const std::string &str) {
    std::stringstream ss(str);
    size_t result;
    ss >> result;
    return result;
}

void vectorToCStringTab(const std::vector<std::string>& str, std::vector<char*>& cstr) {
    cstr.reserve(str.size() + 1);  // Allouer de l'espace pour les éléments et le NULL final
    for (size_t i = 0; i < str.size(); ++i) {
        // Créer une copie de chaque chaîne et l'ajouter à cstr
        char* tmp = new char[str[i].size() + 1];  // +1 pour le caractère de fin '\0'
        std::strcpy(tmp, str[i].c_str());
        cstr.push_back(tmp);
    }
    cstr.push_back(NULL);  // Ajouter un NULL à la fin du tableau pour execve
}


bool fileExists(const std::string &name)
{
    struct stat pathStat;
    if (stat(name.c_str(), &pathStat) != 0)
        return false; // Impossible d'accéder au fichier
    return S_ISREG(pathStat.st_mode);
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
    if (timestamp == 0)
        return "";
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

bool onlySpace(std::string value){
    for(long unsigned int i = 0; i < value.size(); i++)
        if(value[i] != ' ')
            return false;
    return true;
}

void checkPV(std::string& value)
{
    if (value.empty() || onlySpace(value) ==  true)
        return;
	else if (value[value.size() - 1] != ';' && value[value.size() - 1] != '}' && value[value.size() - 1] != '{'){
		std::cout << "ERROR SYNTAX: missing ';' look dumbass : " << value[value.size() - 1] << std::endl;
		exit (1);
	}
    if (value[value.size() - 1] == ';')
            value.erase(value.size() - 1);
}

std::string ensureRelativeDotPath(const std::string& path)
{
    if (path.empty())
        return "./";
    if (path[0] == '/')
        return "." + path;
         // → "/images/..." devient "./images/..."
    return "./" + path;      // → "images/..." devient "./images/..."
}

std::string readPartialFile(const std::string &path, Range &range)
{
    std::string body;
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return "";
    file.seekg(range.start);
    size_t length = range.end - range.start + 1;
    char *buffer = new char[length];
    file.read(buffer, length);
    body = std::string(buffer, length);
    delete[] buffer;
    file.close();
    return body;
}

//----------------------------HAS FUNCTIONS----------------------------------------------------//

bool hasReadPermission(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0)
        return false;
    if (access(path.c_str(), R_OK) != 0)
        return false;
    return true;
}

bool hasWritePermission(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0)
        return false;
    if (access(path.c_str(), W_OK) != 0)
        return false;
    return true;
}

//--------------------------------------IS FUNCTIONS--------------------------------------//

bool isOnlyWhiteSpace(const std::string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        if (!std::isspace(str[i])) {
            return false;
        }
    }
    return true;
}

bool isDirectory(const std::string &path)
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0)
        return false;
    return S_ISDIR(fileStat.st_mode);
}

bool isValidIP(const std::string& ip)
{
	int	numDots	= 0;
	int	numDigits =	0;
	int	currentNumber =	0;

	for	(size_t	i =	0; i < ip.length();	++i)
	{
		char c = ip[i];

		if (c == '.')
		{
			if (currentNumber <	0 || currentNumber > 255)
				return false;
			numDots++;
			currentNumber =	0;
			numDigits =	0;
		}
		else if (isdigit(c))
		{
			currentNumber =	currentNumber *	10 + (c - '0');
			numDigits++;
			if (numDigits >	3)
				return false;
		}
		else
			return false;
	}
	if (numDots	!= 3 || currentNumber <	0 || currentNumber > 255) 
		return false;
	return true;
}

bool isPathValid(const std::string&	path)
{
	struct stat	info;
	return (stat(path.c_str(), &info) == 0);
}

bool isFileValid(const std::string&	filePath)
{
	struct stat	info;
	return (stat(filePath.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

bool isHttpErrorCodeValid(int code)
{
	return (code >= 400	&& code	<= 599);
}

bool isNumber(const	std::string& str)
{
	for	(size_t	i =	0; i < str.length(); ++i)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------------------//

int countWords(const std::string& str) {
    std::stringstream ss(str);
    std::string word;
    int wordCount = 0;

    while (ss >> word) {
        wordCount++;
    }

    return wordCount;
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