#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"
#include "../includes/errors.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <sstream>


std::string generateFileName(const std::string &scriptName, std::string typeOfFile)
{
    static int counter = 0;
    std::stringstream infileName;
    infileName << "." << scriptName << "_" << counter++ << "_" << typeOfFile;
    return infileName.str();
}

void Response::handleCGI(Errors &errors)
{
    char buffer[PATH_MAX];
    std::string cwd = getcwd(buffer, sizeof(buffer)) ? std::string(buffer) : "";
    if (cwd.empty()) {
    perror("getcwd");
    errors.error500();
    return;
    }
    std::string fullpath = cwd + "/";
    // _cgiPath = _path.substr(0, _path.find_last_of('/') + 1);
    _cgiScriptName = _path.substr(_path.find_first_of('.') + 1, _path.npos);
    _cgiPath = fullpath;
    if (_request.getMethod() == "POST")
    {
        std::string infileName = generateFileName(_cgiScriptName, "infile");
        _cgiInfilePath = _cgiPath + infileName;
    }
    std::string outfileName = generateFileName(_cgiScriptName, "outfile");
    _cgiOutfilePath = _cgiPath + outfileName;
    if ((_cgiPid = fork()) == -1)
        errors.error500();
    else if (_cgiPid == 0)
        childRoutine();
    else
    {
        _cgiIsRunning = true;
        checkCgiStatus();
    }
}

void Response::checkCgiStatus() 
{
    int status;
    const int TIMEOUT = 5; // Timeout en secondes
    const int INTERVAL_USEC = 100000; // Intervalle de vérification en microsecondes
    time_t start = time(NULL);

    while (true)
    {
        pid_t result = waitpid(_cgiPid, &status, WNOHANG);
        if (result == -1)
        {
            perror("waitpid");
            setStatusCode(500);
            break;
        }
        else if (result > 0)
        {
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    std::cerr << "CGI exited with status: " << WEXITSTATUS(status) << std::endl;
                    setStatusCode(500);
                } else {
                    readOutfile();
                }
            } else {
                std::cerr << "CGI script did not exit normally." << std::endl;
                setStatusCode(500);
            }
            break;
        }
        if (time(NULL) - start >= TIMEOUT) {
            std::cerr << "CGI script timeout. Killing..." << std::endl;
            killCgi();
            break;
        }
        usleep(INTERVAL_USEC);
    }   
    _cgiIsRunning = false;
    _responseIsReady = true;
}


void Response::readOutfile()
{
    std::ifstream toSend;
    toSend.open(_cgiOutfilePath.c_str());
    if (toSend.fail())
        setStatusCode(500);
    else
    {
        std::stringstream buffer;
        buffer << toSend.rdbuf();
        std::string full = buffer.str();
    size_t pos = full.find("\r\n\r\n");
    if (pos == std::string::npos)
        pos = full.find("\n\n");

    if (pos != std::string::npos) {
        _headerCgi = full.substr(0, pos);
        _body = full.substr(pos);
        } else {
        _body = full;  // fallback si pas de headers
        }
    }
}

void Response::killCgi()
{
    if (_cgiIsRunning)
    {
        kill(_cgiPid, SIGKILL);
        if (waitpid(_cgiPid, NULL, 0) < 0)
        {
            perror("waitpid");
            setStatusCode(500);  // erreur interne si le waitpid échoue
        }
        else
        {
            cleanUpCgiFiles();
            setStatusCode(504); // timeout : Gateway Timeout
        }
    }
}


void Response::childRoutine()
{
    std::vector<char *> envp;
    std::vector<char *> args;

    try {
        if (_request.getMethod() == "POST")
            manageBodyForCgi();
        manageCgiOutfile();
        if (chdir(_cgiPath.c_str()) == -1) 
        {
            perror("chdir");
            exit(errno);
        }
    
        setEnv();
        vectorToCStringTab(_env, envp);
        _arg.push_back(_cgiBinPath);
        _arg.push_back(joinPaths(_cgiPath, _cgiScriptName));
        vectorToCStringTab(_arg, args);
        
        // On execute le script CGI
        execve(args[0], &args[0], &envp[0]);
        perror("execve");
        for (size_t i = 0; i < envp.size(); i++)
            delete[] envp[i];
        exit(1);
    }
    catch(const std::exception &e)
    {
        perror("childRoutine exception");
        exit(errno);
    }
}

void Response::manageBodyForCgi()
{
    const std::string file = _cgiInfilePath;
    std::ofstream infile;
    //On ouvre l'infile
    infile.open(file.c_str());
    if (infile.fail())
        exit(errno);
    //On écrit le body dans l'infile
    infile << _request.getBody();
    std::stringstream ss;
    //On recupere la taille du body
    ss << _request.getBody().size();
    _cgiInfileSize = ss.str();
    infile.close();
    const int fd = open(file.c_str(), O_CREAT, O_RDWR);
    if (fd < 0)
        exit(errno);
    //On redirige l'entrée standard vers l'infile
    if (dup2(fd, STDIN_FILENO) == -1)
        exit(errno);
    close(fd);
}

void Response::manageCgiOutfile()
{
    //On ouvre l'outfile
    const std::string file = _cgiOutfilePath;
    const int fd = open(file.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0)
        exit(errno);
    //On redirige la sortie standard vers l'outfile
    if (dup2(fd, STDOUT_FILENO) == -1)
        exit(errno);
    close(fd);
}

void Response::setEnv()
{
    _env.push_back("DOCUMENT_ROOT=" + _root);
    _env.push_back("REQUEST_METHOD=" + _request.getMethod());
    _env.push_back("SERVER_NAME=" + _server.getServerName());
    _env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    _env.push_back("SERVER_PORT=" + toString(_server.getPort()));
    _env.push_back("SERVER_SOFTWARE=webserv/1.0");
    if (_request.getHeaders().count("User-Agent") != 0)
        _env.push_back("HTTP_USER_AGENT=" + _request.getHeaders().at("User-Agent"));
    if (_request.getHeaders().count("Accept") != 0)
        _env.push_back("HTTP_ACCEPT=" + _request.getHeaders().at("Accept"));
    _env.push_back("QUERY_STRING=" + _request.getQueryString());
    if (_request.getMethod() == "POST")
    {
        _env.push_back("CONTENT_LENGTH=" + _cgiInfileSize);
        if (_request.getHeaders().count("Content-Type") != 0)
            _env.push_back("CONTENT_TYPE=" + _request.getHeaders().at("Content-Type"));
        else
            _env.push_back("CONTENT_TYPE=text/plain");
    }
}