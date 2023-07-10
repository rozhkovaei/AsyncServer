#include <list>
#include <thread>
#include <boost/asio.hpp>

#include "waiting_queue.h"

class AsyncContextHandler
{
public:
    AsyncContextHandler( WaitingQueue< std::list< std::string > >& queue,
                         WaitingQueue< std::list< std::string > >& console_queue,
                         int size )
    : file_queue_( queue )
    , console_queue_( console_queue )
    , bulk_size_( size )
    , static_context_( nullptr )
    {}
    
    void PushDataToStaticContext( std::string& line );
    void PushDataToDynamicContext( void* context, std::string& line );
    
    void DestroyStaticContext();
    
    void* ConnectDynamicContext();
    void DisconnectDynamicContext( void* context );
    
private:
    WaitingQueue< std::list< std::string > >& file_queue_;
    WaitingQueue< std::list< std::string > >& console_queue_;
    
    int bulk_size_;
    void* static_context_;
    
    std::mutex mtx_;
    std::mutex dynamic_mtx_;
};

class session
  : public std::enable_shared_from_this<session>
{
public:
  session( boost::asio::ip::tcp::socket socket, AsyncContextHandler& context_handler )
    : socket_( std::move( socket ) )
    , context_handler_( context_handler )
    , dynamic_context_( nullptr )
    , dynamic_count_( 0 )
  {}

  void start() { do_read(); }

private:
    void do_read();

  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  AsyncContextHandler& context_handler_;
  void* dynamic_context_;
  int dynamic_count_;
};

class server
{
public:
    server( boost::asio::io_context& io_context, short port, int block_size );

    ~server();
    
private:
    
  void do_accept();
  
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::socket socket_;
    
  AsyncContextHandler context_handler_;
  int block_size_;
    
  WaitingQueue< std::list< std::string > > file_queue_;
  WaitingQueue< std::list< std::string > > console_queue_;

  std::thread logger_;
  std::thread file1_;
  std::thread file2_;
};
