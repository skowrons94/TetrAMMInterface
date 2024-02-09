#include "GraphiteConnector.h"

GraphiteConnector::GraphiteConnector(std::string host, int port) {
    endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port);
}

void GraphiteConnector::connect(void) {
    socket = new boost::asio::ip::tcp::socket(service);
    socket->open(boost::asio::ip::tcp::v4());
    socket->connect(endpoint);
}

void GraphiteConnector::disconnect(void) {
    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
    socket->close();
}

void GraphiteConnector::write(std::string message) {
    socket->write_some(boost::asio::buffer(message));
}
