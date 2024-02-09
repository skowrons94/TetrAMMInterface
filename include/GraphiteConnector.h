#pragma once

#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>


class GraphiteConnector {

public:

    GraphiteConnector(std::string host, int port);
    
    // Connect to the specified server
    void connect(void);
    // Disconnect from the specified server
    void disconnect(void);

    void write(std::string message);

private:
    boost::asio::io_service service;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::socket* socket;
};
