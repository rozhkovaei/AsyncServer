#include <iostream>
#include <sstream>
#include <fstream>

#include "async_server.h"
#include "async.h"

using namespace std;

void console_producer( WaitingQueue< list< string > >& queue ) {

    list< string > bulk;
    
    while( queue.pop( bulk ) )
    {
        std::cout << "bulk: ";
        
        for( const auto& cmd : bulk )
        {
            std::cout << cmd << " ";
        }
        
        std::cout << std::endl;
        
        bulk.clear();
    }
}

void file_producer( WaitingQueue< list< string > >& queue ) {

    list< string > bulk;
    
    while( queue.pop( bulk ) )
    {
        if( bulk.empty() )
        {
            continue;
        }
        
        stringstream ss;
        ss << "/Users/isentio/Documents/Projects/Otus/lection 19 STL алгоритмы/repo/";
        ss << std::chrono::system_clock::now().time_since_epoch().count();
        ss << "_";
        ss << std::this_thread::get_id();
        ss << ".log";
        
        string filename = ss.str();
        
        ofstream myfile;
        myfile.open ( filename );
        
        for( const auto& cmd : bulk )
        {
            myfile << cmd << endl;
        }
        
        myfile.close();
        
        bulk.clear();
    }
}

void AsyncContextHandler::PushDataToStaticContext( std::string& line )
{
    std::lock_guard< std::mutex > guard{ mtx_ };
    if( static_context_ == nullptr )
       static_context_ = connect( file_queue_, console_queue_, bulk_size_ );
    
    recieve( static_context_, line.data(), line.length() );
}

void AsyncContextHandler::PushDataToDynamicContext( void* context, std::string& line )
{
    std::lock_guard< std::mutex > guard{ dynamic_mtx_ };
    recieve( context, line.data(), line.length() );
}

void AsyncContextHandler::DestroyStaticContext()
{
    std::lock_guard< std::mutex > guard{ mtx_ };
    disconnect( static_context_ );
    static_context_ = nullptr;
}

void* AsyncContextHandler::ConnectDynamicContext()
{
    std::lock_guard< std::mutex > guard{ dynamic_mtx_ };
    return connect( file_queue_, console_queue_, bulk_size_ );
}

void AsyncContextHandler::DisconnectDynamicContext( void* context )
{
    std::lock_guard< std::mutex > guard{ dynamic_mtx_ };
    disconnect( context );
    context = nullptr;
}

void session::do_read()
{
auto self( shared_from_this() );
  socket_.async_read_some( boost::asio::buffer(data_, max_length ),
    [ this, self ]( boost::system::error_code ec, std::size_t length )
    {
      if ( !ec )
      {
          std::istringstream input;
          input.str( data_ );
          
          for ( string line; getline( input, line ); )
          {
              if( line == "}" )
              {
                  dynamic_count_--;
                  if( dynamic_count_ == 0 )
                  {
                      context_handler_.PushDataToDynamicContext( dynamic_context_, line );
                      continue;
                  }
                  if( dynamic_count_ < 0 ) // ignore
                  {
                      dynamic_count_ = 0;
                      continue;
                  }
              }
              
              if( line == "{" )
              {
                  dynamic_count_++;
                  if(dynamic_context_ == nullptr)
                      dynamic_context_ = context_handler_.ConnectDynamicContext();
              }
              
              if( dynamic_count_ )
              {
                  context_handler_.PushDataToDynamicContext( dynamic_context_, line );
              }
              else
              {
                  context_handler_.PushDataToStaticContext( line );
              }
          }
          
          do_read();
      }
      else
      {
        context_handler_.DestroyStaticContext();
        context_handler_.DisconnectDynamicContext( dynamic_context_ );
      }
    });
}

server::server( boost::asio::io_context& io_context, short port, int block_size )
: acceptor_( io_context, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port ) )
, socket_( io_context )
, context_handler_( file_queue_, console_queue_, block_size )
, block_size_( block_size )
{
    logger_ = std::thread( &console_producer, std::ref( console_queue_ ) );
      
    file1_ = std::thread( &file_producer, std::ref( file_queue_ ) );
    file2_ = std::thread( &file_producer, std::ref( file_queue_ ) );
      
    do_accept();
}

server::~server()
{
  context_handler_.DestroyStaticContext();
  
  file_queue_.stop();
  console_queue_.stop();
  
  logger_.join();
  file1_.join();
  file2_.join();
}

void server::do_accept()
{
    acceptor_.async_accept( socket_, [ this ]( boost::system::error_code ec )
    {
      if ( !ec )
      {
        std::make_shared< session >( std::move( socket_ ), context_handler_ )->start();
      }

      do_accept();
    } );
}
