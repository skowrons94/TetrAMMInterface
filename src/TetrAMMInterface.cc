#include "TetrAMMInterface.h"

#include <iostream>
#include <string.h>

double endian_swap(double d)
{
    long x = *(long *)&d;
    x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
    x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
    x = (x & 0x00FF00FF00FF00FF) << 8  | (x & 0xFF00FF00FF00FF00) >> 8;

    return *(double *)&x;
}

float precision( float f, int places )
{
    float n = std::pow(10.0f, places ) ;
    return std::round(f * n) / n ;
}

TetrAMMInterface::TetrAMMInterface( ){

  port = 0;
  sockfd = -1;
  address = "";
  verbose = 1;
  reporter = NULL;

  cleanBuffers( );

}

void TetrAMMInterface::setGraphite( std::string host, int port ){
  reporter = new GraphiteReporter( host, port );
  reporter->connect();
}

void TetrAMMInterface::removeGraphite( ){
  reporter->disconnect();
  delete reporter;
  reporter = NULL;
}

void TetrAMMInterface::sendGraphite( ){

  if( reporter != NULL )
  {
    for( int i = 0; i < nChannels; i++ )
    {
      std::string channel = "current/channel" + std::to_string(i);
      reporter->send( channel, dataBuffer[i] );
    }
  }

}

void TetrAMMInterface::cleanBuffers( ){

  memset( &inBuffer, 0, sizeof(inBuffer) );
  memset( &outBuffer, 0, sizeof(outBuffer) );

  for( int i = 0; i < MAX_CHANNELS; i++ ){
    for( int j = 0; j < SAMPLING_RATE; j++ ){
      sampleBuffer[i][j] = 0;
    }
  }

  for( int i = 0; i < MAX_CHANNELS; i++ ){
    dataBuffer[i] = 0;
  }

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

  if( verbose > 0 ) std::cout << "Connected to " << address << " on port " << port << std::endl;

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

  if( verbose > 1 ) std::cout << "Command: " << inBuffer << std::endl;
  status = send( sockfd, (char*)&inBuffer, sizeof(command), 0 );
  
  if( status < 0 ) return false;
  return true;

}

bool TetrAMMInterface::receiveCommand( ){

  int status;

  memset( outBuffer, 0, sizeof(outBuffer) );

  status = recv( sockfd, (char*)&outBuffer, sizeof(outBuffer), 0 );
  if( verbose > 1 ) std::cout << "Bytes: " << status << std::endl;
  if( verbose > 1 ) std::cout << "Response: " << outBuffer << std::endl;
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
  if( verbose > 1 ) std::cout << "Bytes: " << status << std::endl;
  if( status < 0 ) return false;

  return true;

}

void TetrAMMInterface::decodeSample( int iSample ){

  if( isASCII )
  {
    std::string out = std::string( outBuffer );
    for( int i = 0; i < numBytes / 8; ++i ){
      sampleBuffer[i][iSample] = std::stod( out.substr(16*i, 16) );
      if( verbose > 1 ) std::cout << sampleBuffer[i][iSample] << std::endl;
    }
  }
  else
  {
    for( int i = 0; i < numBytes / 8; ++i ){
      memcpy(&sampleBuffer[i][iSample], &outBuffer[i*sizeof(double)], sizeof(double));
      sampleBuffer[i][iSample] = endian_swap(sampleBuffer[i][iSample]);
      if( verbose > 1 ) std::cout << sampleBuffer[i][iSample] << std::endl;
    }
  }

}

void TetrAMMInterface::decodeData( ){

  if( isASCII )
  {
    std::string out = std::string( outBuffer );
    for( int i = 0; i < nChannels; ++i ){
      dataBuffer[i] = std::stod( out.substr(16*i, 16) );
      if( verbose > 1 ) std::cout << dataBuffer[i] << std::endl;
    }
  }
  else
  {
    for( int i = 0; i < numBytes / 8; ++i ){
      memcpy(&dataBuffer[i], &outBuffer[i*sizeof(double)], sizeof(double));
      dataBuffer[i] = endian_swap(dataBuffer[i]);
      if( verbose > 1 ) std::cout << dataBuffer[i] << std::endl;
    }
  }

}

void TetrAMMInterface::writeData( ){

  // Get time in float format
  std::chrono::duration<float> elapsed_seconds = std::chrono::system_clock::now() - startTime;
  float time = elapsed_seconds.count();

  dataFile << std::setw(10) << precision(time, 1) << " ";
  for( int i = 0; i < nChannels; i++ ) dataFile << std::setw(21) << dataBuffer[i] << " ";
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
  ver = response.substr(4, response.size( ) - 6);
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

bool TetrAMMInterface::readSamples( float seconds ){

  int numSamples = 100000 * seconds;
  std::string command = "FASTNAQ:" + std::to_string( numSamples );

  cleanBuffers();

  bool status;
  status = sendCommand( command );
  if( !status ) return false;

  for( int i = 0; i < numSamples; i++ ){
    status = receiveData( );
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
    if( !status ) return false;
  }

  return true;

}

void TetrAMMInterface::dumpSamples( ){

  dataFile.open("dump.txt");

  for( int i = 0; i < nSamples; i++ ){
    for( int j = 0; j < nChannels; j++ ){
      if( sampleBuffer[j][i] != 0 ){
        dataFile << sampleBuffer[j][i] << " ";
      }
    }
    dataFile << std::endl;
  }

  dataFile.close();

}

bool TetrAMMInterface::startAcquisition( float seconds, std::string fileName ){

  std::string command = "ACQ:ON";

  // We want to read the samples averaged over 1 second * float scaling
  int numSamples = 100000 * seconds;
  this->setAvgSamples(numSamples);
  
  startTime = std::chrono::system_clock::now();
  dataFile.open(fileName);
  writeHeader();
  cleanBuffers();

  bool status;
  status = sendCommand( command );
  if( !status ) return false;

  startCall = true;
  dataThread = new boost::thread( &TetrAMMInterface::acqusitionThread, this );
  isAcquiring = true;

  if( verbose > 0 ) std::cout << "Acquisition started." << std::endl;

  return true;

}

bool TetrAMMInterface::stopAcquisition( ){

  std::string command = "ACQ:OFF";

  // Stopping the acquisition thread
  startCall = false;
  dataThread->join();
  dataThread = NULL;

  // Writing the footer
  stopTime = std::chrono::system_clock::now();
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

  if( verbose > 0 ) std::cout << "Acquisition stopped." << std::endl;

  return true;

}

void TetrAMMInterface::acqusitionThread( ){

  bool status;
  while( startCall ){
    status = receiveData( );
    if ( status )
    {
      decodeData( );
      writeData( );
      sendGraphite( );
    }
  }

}

void TetrAMMInterface::writeHeader( ){
  
  // Get start time in YYYY-MM-DD HH:MM:SS format
  std::time_t t = std::chrono::system_clock::to_time_t(startTime);
  std::tm tm = *std::localtime(&t);
  char buffer[20];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
  std::string start(buffer);
  
  dataFile << "# TetrAMM Data File" << std::endl;
  dataFile << "# Version: " << ver << std::endl;
  dataFile << "# ASCII: " << isASCII << std::endl;
  dataFile << "# Channels: " << nChannels << std::endl;
  dataFile << "# Range: " << rng << std::endl;
  dataFile << "# Samples: " << nSamples << std::endl;
  dataFile << "# Trigger: " << isTRG << std::endl;
  dataFile << "# Start: " << start << std::endl;
  dataFile << "# Time (s) ";
  for( int i = 0; i < nChannels; i++ ) dataFile << std::setw(16) << "Channel " << i << " (A) ";
  dataFile << std::endl;

}

void TetrAMMInterface::writeFooter( ){

  // Get stop time in YYYY-MM-DD HH:MM:SS format
  std::time_t t = std::chrono::system_clock::to_time_t(stopTime);
  std::tm tm = *std::localtime(&t);
  char buffer[20];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
  std::string stop(buffer);

  dataFile << "# Stop: " << stop << std::endl;
  dataFile << std::endl;

}