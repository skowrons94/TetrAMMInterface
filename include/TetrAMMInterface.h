#ifndef TETRAMMINTERFACE_H
#define TETRAMMINTERFACE_H

#define SAMPLING_RATE 100000
#define MIN_DATA_AVG_BIN 5
#define MIN_DATA_ABG_ASCII 500

#define MAX_SAMPLES 100000

#define MAX_DATA_LEN_BIN 20000
#define MAX_DATA_LEN_ASCII 200

#define MAX_COMMAND_LEN 256
#define SUCCESS "ACK"

#include <string>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <boost/thread.hpp>

class TetrAMMInterface {

    public:
        TetrAMMInterface( );
        ~TetrAMMInterface( ){};

        // Initalization
        bool readSettings( );

        // TCP/IP Connection
        bool connect( std::string address, int port );
        void disconnect( );
        void close( );

        // TCP/IP Communication for Commands
        bool sendCommand( std::string command );
        bool receiveCommand( );
        bool communicate( std::string command );

        // TCP/IP Communication for Data
        bool receiveData( );

        // Error Handling
        void dumpError( int error );

        // Check the response from the TetrAMM (look for "ACK" or "NACK")
        bool checkResponse( );

        // TetrAMM Commands
        bool checkVersion( );

        bool activateASCII( );
        bool checkASCII( );
        bool deactivateASCII( );

        bool setNumberOfChannels( int number );
        bool checkNumberOfChannels( );

        bool readNumSamples( int numSamples );
        bool readSample( );

        bool checkAvgSample( );
        bool setAvgSamples( int numSamples );
        bool readAvgSample( int numSamples );

        bool checkRng( );
        bool setRng( int range );

        bool checkTRG( );
        bool activateTRG( );
        bool deactivateTRG( );

        bool startAcquisition( );
        bool stopAcquisition( );

        bool getASCII( ){ return isASCII; }
        bool getTRG( ){ return isTRG; }
        int getNumberOfChannels( ){ return nChannels; }
        int getNumberOfSamples( ){ return nSamples; }
        int getRng( ){ return rng; }
        std::string getVersion( ){ return ver; }

        bool isAcquiringData( ){ return isAcquiring; }

        void acqusitionThread( );

    private:

        int                 port;
        int                 sockfd;
        std::string         address;
        struct sockaddr_in  server;

        char inBuffer[MAX_COMMAND_LEN];
        char outBuffer[MAX_COMMAND_LEN];

        double sampleBufferBin[MAX_SAMPLES];
        std::string dataBufferASCII[MAX_DATA_LEN_ASCII];

        int rng;
        int nChannels;
        int nSamples;
        bool isASCII;
        bool isTRG;
        bool isAcquiring;
        std::string ver;

        bool startCall;
        boost::thread* dataThread;



};


#endif