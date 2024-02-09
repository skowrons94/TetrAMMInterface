#include "GraphiteReporter.h"

GraphiteReporter::GraphiteReporter(std::string host, int port) {
    connector = new GraphiteConnector(host, port);
}

void GraphiteReporter::connect(void) {
    connector->connect();
}

void GraphiteReporter::disconnect(void) {
    connector->disconnect();
}

std::string GraphiteReporter::sanitizeString(std::string string) {
    std::replace(string.begin(), string.end(), ' ', '-');
    return string;
}

void GraphiteReporter::send(std::string string, double value) {
    std::string metric = sanitizeString(string);
    auto seconds = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    std::stringstream ss;
    ss << metric << " " << value << " " << seconds << "\n" << std::endl;
    std::string message = ss.str();
    connector->write(message);
}

