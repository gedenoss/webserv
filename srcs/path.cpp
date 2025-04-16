#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"

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
            setStatusCode(404);
        else if (errno == EACCES)
            setStatusCode(403);
        else
            setStatusCode(500);
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
    setStatusCode(200);
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
    if (_request.getMethod() == "POST")
    {
        if (isDirectory(path.substr(0, path.find_last_of('/') + 1)))
        {
            _path = path;
            return;
        }
        else if (isDirectory(subPath.substr(0, subPath.find_last_of('/') + 1)))
        {
            _path = subPath;
            return;
        }
        setStatusCode(403);
        return;
    }
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
        setStatusCode(403);
        _path = "";
        return;
    }
    // 6. Sinon : rien trouvé → 404
    setStatusCode(404);
    _path = "";
}