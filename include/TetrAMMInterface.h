#ifndef TETRAMMINTERFACE_H
#define TETRAMMINTERFACE_H

#define MAX_COMMAND_LEN 256

#include <string>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class TetrAMMInterface {

    public:
        TetrAMMInterface( );
        ~TetrAMMInterface( );

        bool connect( std::string address, int port );
        void disconnect( );
        void close( );

        bool send( std::string command );
        bool receive( std::string& response );

        bool startAcquisition( );
        bool stopAcquisition( );

    private:

        int                 port;
        int                 sockfd;
        std::string         address;
        struct sockaddr_in  server;

        char inBuffer[MAX_COMMAND_LEN];
        char outBuffer[MAX_COMMAND_LEN];

};


#endif