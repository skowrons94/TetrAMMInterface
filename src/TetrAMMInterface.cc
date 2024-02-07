#include "TetrAMMInterface.h"

#include <iostream>
#include <string.h>

TetrAMMInterface::TetrAMMInterface( ){

    port = 0;
    sockfd = -1;
    address = "";

}

void TetrAMMInterface::dumpError( int error ){

  switch (error)
  {
  case 0:
    std::cout << "Error: Invalid Command" << std::endl;
    break;
  case 10:
    std::cout << "Error: Wrong ACQ Acquisition Parameter" << std::endl;
    break;
  case 11:
    std::cout << "Error: Wrong GET Acquisition Parameter" << std::endl;
    break;
  case 12:
    std::cout << "Error: Wrong NAQ Acquistion Parameter" << std::endl;
    break;
  case 13:
    std::cout << "Error: Wrong TRG Acquisition Parameter" << std::endl;
    break;
  case 14:
    std::cout << "Error: Wrong GATE Acquisition Parameter" << std::endl;
    break;
  case 15:
    std::cout << "Error: Wrong FASTNAQ Acquisition Parameter" << std::endl;
    break;
  case 17:
    std::cout << "Error: Wrong TRGPOOL Acquisition Parameter" << std::endl;
    break;
  case 20:
    std::cout << "Error: Wrong Number of Channel Parameter" << std::endl;
    break;
  case 21:
    std::cout << "Error: Wrong ASCII Parameter" << std::endl;
    break;
  case 22:
    std::cout << "Error: Wrong Range Parameter" << std::endl;
    break;
  case 23:
    std::cout << "Error: Wrong User Correction Parameter" << std::endl;
    break;
  case 24:
    std::cout << "Error: Wrong Number of Samples Parameter" << std::endl;
    break;
  case 25:
    std::cout << "Error: Wrong Status Parameter" << std::endl;
    break;
  case 26:
    std::cout << "Error: Wrong Interlock Parameter" << std::endl;
    break;
  case 27:
    std::cout << "Error: Wrong High Voltage Parameter" << std::endl;
    break;
  case 30:
    std::cout << "Error: HV Fault" << std::endl;
    break;
  case 40:
    std::cout << "Error: Wrong PKTSIZE Parameter" << std::endl;
    break;
  case 54:
    std::cout << "Error: Wrong Voltage Value" << std::endl;
    break;
  case 96:
    std::cout << "Error: Wrong Device ID" << std::endl;
    break;
  
  default:
    std::cout << "Error: Error Code Not Found" << std::endl;
    break;
  }

}

void TetrAMMInterface::disconnect( ){

    if( sockfd != -1 ){
        shutdown(sockfd,SHUT_RDWR);
    }

}

void TetrAMMInterface::close( ){

    if( sockfd != -1 ){
        ::close(sockfd);
        sockfd = -1;
    }

    port = 0;
    address = "";

}

bool TetrAMMInterface::connect( std::string address, int port ){

  // Create socket if it is not already created
  if( sockfd == -1 ){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
      return false;
    }
  }
    
  // Setup address structure
  if( inet_addr(address.c_str( )) == - 1 ){
    
    struct hostent*  he;
    struct in_addr** addr_list;

    if(( he = gethostbyname( address.c_str() ) ) == NULL ){     
      return false;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(int i = 0; addr_list[i] != NULL; i++){
      server.sin_addr = *addr_list[i];
      break;
    }

  }

  else server.sin_addr.s_addr = inet_addr( address.c_str() );

  server.sin_family = AF_INET;
  server.sin_port   = htons( port );
  
  struct timeval timeout;      
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  if( setsockopt ( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout) ) < 0 ){
    std::cout << "Error setting timeout" << std::endl;
    return false;
  }

  if(::connect(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0 ){
    std::cout << "Connection failed" << std::endl;
    return false;
  }

  std::cout << "Connected to " << address << " on port " << port << std::endl;

  if( this->readSettings( ) ) return true;
  else{
    this->disconnect( );
    return false;
  };

  return true;

}

bool TetrAMMInterface::sendCommand( std::string command ){

  int status;
  command += "\r";

  memset( &inBuffer, 0, sizeof(inBuffer) );
  strcpy( inBuffer, command.c_str() );

  std::cout << "Command: " << inBuffer << std::endl;
  status = send( sockfd, (char*)&inBuffer, sizeof(command), 0 );
  
  if( status < 0 ) return false;
  return true;

}

bool TetrAMMInterface::receiveCommand( ){

  int status;

  memset( outBuffer, 0, sizeof(outBuffer) );

  status = recv( sockfd, (char*)&outBuffer, sizeof(outBuffer), 0 );
  std::cout << "Status: " << status << std::endl;
  std::cout << "Response: " << outBuffer << std::endl;
  if( status < 0 ) return false;

  return true;

}

bool TetrAMMInterface::communicate( std::string command ){

  bool status;

  status = sendCommand( command );
  if( !status ) return false;
  status = receiveCommand( );
  if( !status ) return false;

  return true;

}

bool TetrAMMInterface::receiveData( ){

  int status;
  memset( outBuffer, 0, sizeof(outBuffer) );
  status = recv( sockfd, (char*)&outBuffer, numBytes, 0 );
  if( status < 0 ) return false;

  return true;

}

void TetrAMMInterface::decodeSample( int iSample ){

  if( isASCII )
  {
    double* out = (double*)outBuffer;
    for( int i = 0; i < numBytes / 8; ++i ){
      memset(&samplesBuffer[i][iSample], out[i], sizeof(double));
    }
  }
  else
  {
    double* out = (double*)outBuffer;
    for( int i = 0; i < numBytes / 8; ++i ){
      memset(&samplesBuffer[i][iSample], out[i], sizeof(double));
    }
  }

}

void TetrAMMInterface::decodeData( ){

  if( isASCII )
  {
    double* out = (double*)outBuffer;
    for( int i = 0; i < numBytes / 8; ++i ){
      memset(&dataBuffer[i], out[i], sizeof(double));
    }
  }
  else
  {
    double* out = (double*)outBuffer;
    for( int i = 0; i < numBytes / 8; ++i ){
      memset(&dataBuffer[i], out[i], sizeof(double));
    }
  }

}

void TetrAMMInterface::writeData( ){

  uint64_t time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startTime).count();

  dataFile << time << " ";
  for( int i = 0; i < nChannels; i++ ) dataFile << dataBuffer[i] << " ";
  dataFile << std::endl;

}

bool TetrAMMInterface::readSettings( ){

  bool status;

  status = checkVersion( );
  if( !status ) return false;

  status = checkASCII( );
  if( !status ) return false;

  status = checkNumberOfChannels( );
  if( !status ) return false;

  status = checkRng( );
  if( !status ) return false;

  status = checkAvgSample( );
  if( !status ) return false;

  status = checkTRG( );
  if( !status ) return false;

  if( isASCII ) numBytes = 16 * nChannels + 1;
  else numBytes = 8 * (nChannels + 1);

  return true;

}

bool TetrAMMInterface::checkResponse( ){

  std::string response = outBuffer;
  if( response.find("ACK") != std::string::npos ) return true;
  else if( response.find("NAK") != std::string::npos ){
    int error = std::stoi( response.substr(5,2) );
    dumpError( error );
    return false;
  }
  else return false;

}

bool TetrAMMInterface::checkVersion( ){

  std::string command = "VER:?";

  bool status;

  status = communicate( command );

  if( !status ) return false;

  std::string response = outBuffer;
  ver = response.substr(4);
  return true;

}

bool TetrAMMInterface::checkASCII( ){

  std::string command = "ASCII:?";

  bool status;
  status = communicate( command );
  if( !status ) return false;

  std::string response = outBuffer;
  if( response.find("ON") != std::string::npos )
  { 
    isASCII = true;
    return true;
  }
  else if( response.find("OFF") != std::string::npos )
  {
    isASCII = false;
    return true;
  }
  else return false;

}

bool TetrAMMInterface::checkNumberOfChannels( ){

  std::string command = "CHN:?";

  bool status;
  status = communicate( command );
  if( !status ) return false;

  std::string response = outBuffer;
  nChannels = std::stoi( response.substr(4,1) );
  return true;

}

bool TetrAMMInterface::checkAvgSample( ){

  std::string command = "NRSAMP:?";

  bool status;
  status = communicate( command );
  if( !status ) return false;

  std::string response = outBuffer;
  nSamples = std::stoi( response.substr(7) );
  return true;

}

bool TetrAMMInterface::checkRng( ){

  std::string command = "RNG:?";

  bool status;
  status = communicate( command );
  if( !status ) return false;

  std::string response = outBuffer;
  rng = std::stoi( response.substr(4,1) );
  return true;

}

bool TetrAMMInterface::checkTRG( ){

  std::string command = "TRG:?";

  bool status;
  status = communicate( command );
  if( !status ) return false;

  std::string response = outBuffer;
  if( response.find("ON") != std::string::npos )
  { 
    isTRG = true;
    return true;
  }
  else if( response.find("OFF") != std::string::npos )
  {
    isTRG = false;
    return true;
  }
  else return false;

}

bool TetrAMMInterface::activateASCII( ){

  std::string command = "ASCII:ON";

  bool status;
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;

  numBytes = 16 * nChannels + 1;
  isASCII = true;

  return true;

}

bool TetrAMMInterface::deactivateASCII( ){

  std::string command = "ASCII:OFF";

  bool status;
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;

  numBytes = 8 * (nChannels + 1);    
  isASCII = false;

  return true;

}

bool TetrAMMInterface::activateTRG( ){

  std::string command = "TRG:ON";

  bool status;
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;
    
  isTRG = true;
  return true;

}

bool TetrAMMInterface::deactivateTRG( ){

  std::string command = "TRG:OFF";

  bool status;
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;
    
  isTRG = false;
  return true;

}

bool TetrAMMInterface::setRng( int range ){

  std::string command = "RNG:" + std::to_string( range );

  bool status;
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;

  rng = range;
  return true;

}

bool TetrAMMInterface::setNumberOfChannels( int number ){

  std::string command = "CHN:" + std::to_string( number );

  bool status;
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;
  
  nChannels = number;
  if( isASCII ) numBytes = 16 * nChannels + 1;
  else numBytes = 8 * (nChannels + 1);

  return true;

}

bool TetrAMMInterface::setAvgSamples( int numSamples ){

  std::string command = "NRSAMP:" + std::to_string( numSamples );

  bool status;
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;

  nSamples = numSamples;
  return true;

}

bool TetrAMMInterface::readSample( ){

  std::string command = "GET:?";

  bool status;
  status = sendCommand( command );
  if( !status ) return false;

  status = receiveData( );
  if( !status ) return false;

  return true;

}

bool TetrAMMInterface::readNumSamples( int seconds ){

  int numSamples = 100000 * seconds;
  std::string command = "FASTNAQ:" + std::to_string( numSamples );

  bool status;
  status = sendCommand( command );
  if( !status ) return false;

  for( int i = 0; i < numSamples; i++ ){
    status = receiveData( );
    std::cout << "Read Data:  " << i << " " << status << std::endl;
    if( !status ) return false;
      decodeSample( i );
  }

  if( isASCII )
  {
    status = receiveCommand( );
    if( !status ) return false;
    status = checkResponse( );
    if( !status ) return false;
  }
  else
  {
    status = receiveData( );
    std::cout << "Read Data:  " << status << std::endl;
    if( !status ) return false;
  }

  return true;

}

// FIXME: This function is not working properly
bool TetrAMMInterface::readAvgSample( int seconds ){

  this->setAvgSamples(100000 * seconds);
  std::string command = "NAQ:1";

  bool status;
  status = sendCommand( command );
  if( !status ) return false;

  status = receiveData( );
  std::cout << "Read Data: " << status << std::endl;
  if( !status ) return false;
  decodeData( );

  if( isASCII )
  {
    status = receiveCommand( );
    if( !status ) return false;
    status = checkResponse( );
    if( !status ) return false;
  }
  else
  {
    status = receiveData( );
    std::cout << "Read Data:  " << status << std::endl;
    if( !status ) return false;
  }

  return true;

}

bool TetrAMMInterface::startAcquisition( int seconds, std::string fileName ){

  std::string command = "ACQ:ON";

  // We want to read the samples averaged over 1 second
  this->setAvgSamples(100000 * seconds);
  
  dataFile.open(fileName);
  writeHeader();

  bool status;
  status = sendCommand( command );
  if( !status ) return false;

  startCall = true;
  dataThread = new boost::thread( &TetrAMMInterface::acqusitionThread, this );
    
  isAcquiring = true;
  return true;

}

bool TetrAMMInterface::stopAcquisition( ){

  std::string command = "ACQ:OFF";

  // Stopping the acquisition thread
  startCall = false;
  dataThread->join();
  dataThread = NULL;

  // Writing the footer
  writeFooter();
  dataFile.close();

  // We call communicate to flush the remaining buffer
  bool status;
  status = communicate( command );
  if( !status ) return false;

  // We now send the actual command
  status = communicate( command );
  if( !status ) return false;
  status = checkResponse( );
  if( !status ) return false;
    
  isAcquiring = false;
  return true;

}

void TetrAMMInterface::acqusitionThread( ){

  startTime = std::chrono::system_clock::now();

  bool status;
  while( startCall ){
    status = receiveData( );
    decodeData( );
    writeData( );
  }

  stopTime = std::chrono::system_clock::now();

}

void TetrAMMInterface::writeHeader( ){
  
  std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
  
  dataFile << "#TetrAMM Data File" << std::endl;
  dataFile << "#Version: " << ver << std::endl;
  dataFile << "#ASCII: " << isASCII << std::endl;
  dataFile << "#Channels: " << nChannels << std::endl;
  dataFile << "#Range: " << rng << std::endl;
  dataFile << "#Samples: " << nSamples << std::endl;
  dataFile << "#Trigger: " << isTRG << std::endl;
  dataFile << "#" << std::endl;
  dataFile << "#Start Time: " << startTime << std::endl;
  dataFile << "#Time (s) ";
  for( int i = 0; i < nChannels; i++ ) dataFile << "Channel " << i << " ";
  dataFile << std::endl;

}

void TetrAMMInterface::writeFooter( ){

  std::string stopTime = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

  dataFile << "#Stop Time: " << stopTime << std::endl;
  dataFile << std::endl;

}