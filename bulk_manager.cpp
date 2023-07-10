#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include "bulk_manager.h"

using namespace std;

Bulk::Bulk( WaitingQueue< list< string > >& queue, WaitingQueue< list< string > >& console_queue )
    : mWaitingQueue( queue )
    , mConsoleWaitingQueue( console_queue )
{}
    
void Bulk::AddCommand( string&& cmd )
{
    mCommands.push_back( move( cmd ) );
}
    
void Bulk::Stop()
{
    if( mCommands.empty() )
        return;
    
  //  std::cout << "Trying to add Bulk!" << std::endl;
    
    mConsoleWaitingQueue.push( mCommands );
    mWaitingQueue.push( std::move( mCommands ) );
    
    std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    
    Clear();
}
    
void Bulk::Clear()
{
    mCommands.clear();
}


StaticBulk::StaticBulk( WaitingQueue< list< string > >& queue, WaitingQueue< list< string > >& console_queue, int size )
    : Bulk( queue, console_queue )
    , max_size( size )
{}
    
void StaticBulk::AddCommand( string&& cmd )
{
    mCommands.push_back( move( cmd ) );
    if( mCommands.size() == max_size )
    {
        Stop();
    }
}
        
bool StaticBulk::Empty()
{
    return mCommands.empty();
}

BulkManager::BulkManager( WaitingQueue< std::list< std::string > >& queue,
                         WaitingQueue< std::list< std::string > >& console_queue, int size )
    : mWaitingQueue( queue )
    , mConsoleWaitingQueue( console_queue )
    , mStaticBulk( mWaitingQueue, mConsoleWaitingQueue, size )
    , mDynamicBulk( mWaitingQueue, mConsoleWaitingQueue )
    , mDynamicCount( 0 )
{
}
    
void BulkManager::Add( string&& cmd )
{
    std::lock_guard< std::mutex > guard{ mtx };
    
    if( cmd == "{" )
    {
        if( !mStaticBulk.Empty() ) // print static bloсk if exists
            mStaticBulk.Stop();
        
        mDynamicCount ++;
    }
    else if( cmd == "}" )
    {
        mDynamicCount --;
        
        if( mDynamicCount == 0 ) // dynamic block is finished
            mDynamicBulk.Stop();
    }
    else
    {
        if( mDynamicCount )
            mDynamicBulk.AddCommand( move( cmd ) );
        else
            mStaticBulk.AddCommand( move ( cmd ) );
    }
}
    
void BulkManager::Stop()
{
    std::lock_guard< std::mutex > guard{ mtx };
    mStaticBulk.Stop(); // when input data is over only static block needs to be reported
}

