#include <../includes/server.hpp>

ServerConfig parseConfig(const std::string &filename){
    ServerConfig config;
    std::ifstream file(filename);
    if (!file.is_open())
}