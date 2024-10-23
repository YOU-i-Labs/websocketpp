#include <iostream>
#include <vector>

#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::ref;
using namespace websocketpp::lib;

void print_protocols(const std::vector<char*> &supported_protocols)
{
    std::cout << "Supported sub-protocols: ";
    
    for (auto p: supported_protocols)
    {
        std::cout << p << " ";
    }
    
    std::cout << std::endl;
}

bool validate_protocol(std::string requested, const std::vector<char*> &supported_protocols)
{
    for (auto p: supported_protocols)
    {
        if(requested == p)
        {
            return true;
        }
    }

    return false;
}

bool validate(server & s, const std::vector<char*> &supported_protocols, connection_hdl hdl)
{
    server::connection_ptr con = s.get_con_from_hdl(hdl);

    std::cout << "Cache-Control: " << con->get_request_header("Cache-Control") << std::endl;

    const std::vector<std::string> & subp_requests = con->get_requested_subprotocols();
    std::vector<std::string> valid_subp_requests;

    for (const auto &request : subp_requests)
    {
        std::cout << "Requested: " << request << std::endl;
        if(validate_protocol(request, supported_protocols))
        {
            valid_subp_requests.push_back(request);
        }
    }

    if (valid_subp_requests.size() > 0)
    {
        con->select_subprotocol(valid_subp_requests[0]);
    }

    return true;
}

void on_message(server &s, connection_hdl hdl, server::message_ptr msg)
{
    server::connection_ptr con = s.get_con_from_hdl(hdl);
    
    std::cout << "Received: " << msg->get_raw_payload() << std::endl;
    
    con->send(msg->get_raw_payload());
}

int main(int argc, char *argv[])
{
    try
    {
        server s;
        
        if(argc < 2)
        {
            std::cout << "Error: must pass in a sub_protocol" << std::endl;
            return 1;
        }
        
        std::vector<char*> supported_protocols(&argv[1], &argv[argc]);
        std::cout << "Starting server with ";
        print_protocols(supported_protocols);

        s.set_validate_handler(bind(&validate, ref(s), supported_protocols, placeholders::_1));
        s.set_message_handler(bind(&on_message, ref(s), placeholders::_1, placeholders::_2));

        s.init_asio();
        s.listen(9005);
        s.start_accept();

        s.run();
    }
    catch (websocketpp::exception const & e)
    {
        std::cout << e.what() << std::endl;
    }
}
