#include "async.h"
#include "bulk_manager.h"

using namespace std;

void* connect( WaitingQueue< std::list< std::string > >& queue,
               WaitingQueue< std::list< std::string > >& console_queue,
               int size )
{
    return new BulkManager( queue, console_queue, size );
}

void recieve( void* context, char* buffer, size_t buffer_size )
{
    if( context == nullptr )
        return;
    
    BulkManager* bulk_manager = reinterpret_cast< BulkManager* >( context );
    string str;
    str.assign( buffer, buffer_size );
    bulk_manager->Add( move( str ) );
}

void disconnect( void* context )
{
    if( context == nullptr )
        return;
    
    BulkManager* bulk_manager = reinterpret_cast< BulkManager* >( context );
    bulk_manager->Stop();
    delete bulk_manager;
    context = nullptr;
}
