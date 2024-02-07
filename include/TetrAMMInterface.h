#ifndef TETRAMMINTERFACE_H
#define TETRAMMINTERFACE_H

#define SAMPLING_RATE 100000
#define MIN_DATA_AVG_BIN 5
#define MIN_DATA_ABG_ASCII 500

#define MAX_CHANNELS 4

#define MAX_COMMAND_LEN 256
#define SUCCESS "ACK"

#include <string>
#include <chrono>
#include <fstream>

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
        void decodeData( );
        void decodeSample( int iSample );
        void writeData( );

        // Error Handling
        void dumpError( int error );

        // Check the response from the TetrAMM (look for "ACK" or "NACK")
        bool checkResponse( );

        // Check settings
        bool checkRng( );
        bool checkTRG( );
        bool checkASCII( );
        bool checkVersion( );
        bool checkAvgSample( );
        bool checkNumberOfChannels( );

        // Change settings
        bool activateTRG( );
        bool deactivateTRG( );
        bool activateASCII( );
        bool deactivateASCII( );

        bool setRng( int range );
        bool setAvgSamples( int numSamples );
        bool setNumberOfChannels( int number );
        
        // Read data
        bool readSample( );
        bool readNumSamples( int seconds = 1 );
        bool readAvgSample( int seconds = 1 ); // FIXME: This function is not working properly

        // Acqusition
        bool startAcquisition( int seconds = 1, std::string filename = "data.txt");
        bool stopAcquisition( );
        void acqusitionThread( );
        void writeHeader( );
        void writeFooter( );

        // Getters
        bool getTRG( ){ return isTRG; }
        bool getASCII( ){ return isASCII; }
        bool getAcqusition( ){ return isAcquiring; }

        int getRng( ){ return rng; }
        int getNumberOfSamples( ){ return nSamples; }
        int getNumberOfChannels( ){ return nChannels; }
        
        std::string getVersion( ){ return ver; }

    private:

        int                 port;
        int                 sockfd;
        std::string         address;
        struct sockaddr_in  server;

        char inBuffer[MAX_COMMAND_LEN];
        char outBuffer[MAX_COMMAND_LEN];

        double dataBuffer[MAX_CHANNELS];
        double samplesBuffer[MAX_CHANNELS][SAMPLING_RATE];

        int rng;
        int nChannels;
        int nSamples;
        int numBytes;
        bool isASCII;
        bool isTRG;
        bool isAcquiring;
        std::string ver;

        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point stopTime;

        bool startCall;
        boost::thread* dataThread;

        std::ofstream dataFile;

};


#endif