#include "ServerSocket.h"
#include "SocketException.h"

#include <sstream>
#include <errno.h>
#include <string.h>

ServerSocket::ServerSocket ( int port )
{
  if ( ! Socket::create() )
    {
      int e = errno;
      std::stringstream error;
      error << "Could not create server socket: error: (" << e << ") " << strerror(e);
      throw SocketException ( error.str() );
    }

  if ( ! Socket::bind ( port ) )
    {
      int e = errno;
      std::stringstream error;
      error << "Could not bind to port: error: (" << e << ") " << strerror(e);
      throw SocketException ( error.str() );
    }

  if ( ! Socket::listen() )
    {
      int e = errno;
      std::stringstream error;
      error << "Could not listen to socket: error: (" << e << ") " << strerror(e);
      throw SocketException ( error.str() );
    }

}

ServerSocket::~ServerSocket()
{
}


const ServerSocket& ServerSocket::operator << ( const std::string& s ) const
{
  if ( ! Socket::send ( s ) )
    {
      int e = errno;
      std::stringstream error;
      error << "Could not write to socket: error: (" << e << ") " << strerror(e);
      throw SocketException ( error.str() );
    }

  return *this;

}


const ServerSocket& ServerSocket::operator >> ( std::string& s ) const
{
  if ( ! Socket::recv ( s ) )
    {
      int e = errno;
      std::stringstream error;
      error << "Could not read from socket: error: (" << e << ") " << strerror(e);
      throw SocketException ( error.str() );
    }

  return *this;
}

void ServerSocket::accept ( ServerSocket& sock )
{
  if ( ! Socket::accept ( sock ) )
    {
      int e = errno;
      std::stringstream error;
      error << "Could not accept socket: error: (" << e << ") " << strerror(e);
      throw SocketException ( error.str() );
    }
}
