#include "TetrAMMInterface.h"

TetrAMMInterface::TetrAMMInterface( ){

    port = 0;
    sockfd = -1;
    address = "";

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
  
  if(::connect(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0 ){ 
    return false;
  }

  return true;

}

bool TetrAMMInterface::startAcquisition( ){

  std::string command = "ACQ:OFF\r\n";
  std::string response;

  bool status;

  status = send( command );
  if( !status ) return false;
  status = receive( response );
  if( !status ) return false;

  return true;

}

bool TetrAMMInterface::stopAcquisition( ){

  std::string command = "ACQ:OFF\r\n";
  std::string response;

  bool status;

  status = send( command );
  if( !status ) return false;
  status = receive( response );
  if( !status ) return false;

  return true;

}

bool TetrAMMInterface::send( std::string command ){

  int status;

  strcpy( outBuffer, command.c_str() );

  status = ::send( sockfd, inBuffer, MAX_COMMAND_LEN, 0 );
  if( status < 0 ) return false;

  return true;

}

bool TetrAMMInterface::receive( std::string& response ){

  int status;

  status = ::recv( sockfd, outBuffer, MAX_COMMAND_LEN, 0 );
  if( status < 0 ) return false;

  response = std::string( outBuffer );

  return true;

}
