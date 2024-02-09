#include <chrono>
#include <string>
#include <sstream>
#include "GraphiteConnector.h"

class GraphiteReporter {
  public:

    GraphiteReporter(std::string host, int port);
    
    // Connect to specified server
    void connect(void);
    // Disconnect from server.
    void disconnect(void);

    void send(std::string string, double value);

  private:

    std::string sanitizeString(std::string string);

    GraphiteConnector* connector;
};

