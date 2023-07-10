#include <iostream>
#include <boost/asio.hpp>

#include "async_server.h"

int main( int argc, char* argv[] )
{
  try
  {
    if ( argc != 3 )
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    server server( io_context, std::atoi( argv[ 1 ] ), std::atoi( argv[ 2 ] ) );

    io_context.run();
  }
  catch ( const std::exception& ex ) 
  {
    std::cerr << "Exception: " << ex.what() << "\n";
  }

  return 0;
}
