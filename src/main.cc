#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "TetrAMMInterface.h"

int main(int argc, char** argv) {

  // Get --port and --address from command line
  int port = 10001;
  std::string address = "192.168.0.1";

  for(int i = 1; i < argc; i++){
    if( std::string(argv[i]) == "--port" ){
      port = std::stoi(argv[i+1]);
    }
    if( std::string(argv[i]) == "--address" ){
      address = std::string(argv[i+1]);
    }
  }

  TetrAMMInterface tetrAMMInterface;
  tetrAMMInterface.connect(address, port);

  std::cout << "Version: " << tetrAMMInterface.getVersion() << std::endl;
  std::cout << "ASCII: " << tetrAMMInterface.getASCII() << std::endl;
  std::cout << "Channels: " << tetrAMMInterface.getNumberOfChannels() << std::endl;
  std::cout << "Range: " << tetrAMMInterface.getRng() << std::endl;
  std::cout << "Samples: " << tetrAMMInterface.getNumberOfSamples() << std::endl;
  std::cout << "TRG: " << tetrAMMInterface.getTRG() << std::endl;

  //tetrAMMInterface.activateASCII();
  //tetrAMMInterface.deactivateASCII();

  //tetrAMMInterface.readSample();
  //tetrAMMInterface.readNumSamples(2);
  
  tetrAMMInterface.setAvgSamples(100000);
  //tetrAMMInterface.readAvgSample(0);

  tetrAMMInterface.startAcquisition();
  std::this_thread::sleep_for(std::chrono::seconds(2));
  tetrAMMInterface.stopAcquisition();

  return 0;
}
