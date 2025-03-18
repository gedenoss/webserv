#include "../includes/webserv.hpp"
#include "../includes/response.hpp"
#include "../includes/request.hpp" // Ensure this header defines the Request class
#include "../includes/config.hpp"

Webserv::Webserv()
{
}

void Webserv::webserv(const Config& config)
{
    std::cout << "Configuring server with " << config.servers.size() << " servers..." << std::endl;
    for (std::vector<ServerConfig>::const_iterator server_it = config.servers.begin();
         server_it != config.servers.end(); ++server_it)
    {
        const ServerConfig& server = *server_it;
        std::cout << "Server: " << server.server_name << " on port " << server.port << std::endl;
        for (std::vector<LocationConfig>::const_iterator location_it = server.locations.begin();
             location_it != server.locations.end(); ++location_it)
        {
            const LocationConfig& location = *location_it;
            std::cout << "  Location: " << location.path << std::endl;
            std::cout << "    Allowed methods: ";
            for (std::vector<std::string>::const_iterator method_it = location.allow_methods.begin();
                 method_it != location.allow_methods.end(); ++method_it)
            {
                const std::string& method = *method_it;
                std::cout << method << " ";
            }
            std::cout << std::endl;
        }
    }
    Request request;
    Response response;
    response.send_response(request);
}

Webserv::~Webserv()
{
}