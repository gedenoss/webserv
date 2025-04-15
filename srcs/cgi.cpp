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
    _cgiPath = _path.substr(0, _path.find_last_of('/') + 1);
    _cgiScriptName = _path.substr(_path.find_last_of('/') + 1, _path.npos);
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
        parent();
}

void Response::parent()
{
   _cgiIsRunning = true;
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
        _arg.push_back(_cgiScriptName);
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