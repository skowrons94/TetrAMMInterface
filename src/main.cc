#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "TetrAMMInterface.h"

int main(int argc, char** argv) {

  // Get --port and --address from command line
  int port = 10001;
  bool ascii = false;
  int channels = 4;
  int range = 0;
  float avgSec = 1;
  int verbose = 0;
  std::string address = "192.168.0.10";

  for(int i = 1; i < argc; i++){
    if( std::string(argv[i]) == "--port" ){
      port = std::stoi(argv[i+1]);
    }
    if( std::string(argv[i]) == "--address" ){
      address = std::string(argv[i+1]);
    }
    if( std::string(argv[i]) == "--ascii" ){
      ascii = true;
    }
    if( std::string(argv[i]) == "--channels" ){
      channels = std::stoi(argv[i+1]);
      if( channels > 4 || channels < 1 ){
        std::cout << "Invalid number of channels. Must be between 1 and 4." << std::endl;
        return 1;
      }
    }
    if( std::string(argv[i]) == "--range" ){
      range = std::stoi(argv[i+1]);
      if( range > 1 || range < 0 ){
        std::cout << "Invalid range. Must be 0 or 1." << std::endl;
        return 1;
      }
    }
    if( std::string(argv[i]) == "--avg" ){
      avgSec = std::stof(argv[i+1]);
    }
    if( std::string(argv[i]) == "-v" ){
      verbose = std::stoi(argv[i+1]);
    }
    if( std::string(argv[i]) == "--verbose" ){
      verbose = std::stoi(argv[i+1]);
    }
    if( std::string(argv[i]) == "--help" ){
      std::cout << "Usage: " << argv[0] << " [--port <port>] [--address <address>] [--ascii] [--channels <channels>] [--range <range>]" << std::endl;
      return 0;
    }
    if( std::string(argv[i]) == "-h" ){
      std::cout << "Usage: " << argv[0] << " [--port <port>] [--address <address>] [--ascii] [--channels <channels>] [--range <range>]" << std::endl;
      return 0;
    }
  }



  TetrAMMInterface tetrAMMInterface;

  tetrAMMInterface.setVerbose(verbose);
  tetrAMMInterface.connect(address, port); 

  tetrAMMInterface.setNumberOfChannels(channels);
  tetrAMMInterface.setRng(range);

  if( ascii ) tetrAMMInterface.activateASCII();
  else tetrAMMInterface.deactivateASCII();

  std::cout << std::endl;
  std::cout << "Connected to TetrAMM at " << address << ":" << port << std::endl;
  std::cout << "Number of channels: " << channels << std::endl;
  std::cout << "Range: " << range << std::endl;
  std::cout << "ASCII: " << ascii << std::endl;
  std::cout << std::endl;

  std::cout << "Commands:" << std::endl;
  std::cout << "s - start acquisition" << std::endl;
  std::cout << "q - stop acquisition" << std::endl;
  std::cout << "n - read samples" << std::endl;
  std::cout << "d - dump samples" << std::endl;
  std::cout << std::endl;

  std::string cmd;
  while( std::cin >> cmd ){
    if(      cmd[0] == 's' )
    { 
      tetrAMMInterface.startAcquisition( avgSec );
      std::cout << "Acquisition started..." << std::endl << std::endl;
    }
    else if( cmd[0] == 'q' )
    { 
      tetrAMMInterface.stopAcquisition( );
      std::cout << "Acquisition stopped." << std::endl << std::endl;
    }
    else if( cmd[0] == 'n' )
    { 
      tetrAMMInterface.readSamples( );
      std::cout << "Samples read." << std::endl << std::endl;
    }
    else if( cmd[0] == 'd' )
    { 
      tetrAMMInterface.dumpSamples( );
      std::cout << "Samples dumped." << std::endl << std::endl;
    }
  }

  return 0;
}
