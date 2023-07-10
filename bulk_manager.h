#include <list>
#include "waiting_queue.h"

class Bulk
{
public:
    Bulk( WaitingQueue< std::list< std::string > >& queue, WaitingQueue< std::list< std::string > >& console_queue );
    
    void AddCommand( std::string&& cmd );
    
    void Stop();
    
protected:
    
    std::list< std::string > mCommands;
    WaitingQueue< std::list< std::string > >& mWaitingQueue;
    WaitingQueue< std::list< std::string > >& mConsoleWaitingQueue;
    
private:
    
    void Clear();
};

class StaticBulk : public Bulk
{
public:
    
    StaticBulk( WaitingQueue< std::list< std::string > >& queue,
                WaitingQueue< std::list< std::string > >& console_queue,
                int size );
    
    void AddCommand( std::string&& cmd );
        
    bool Empty();
    
private:
    
    int max_size;
};

class BulkManager
{
public:
    
    BulkManager( WaitingQueue< std::list< std::string > >& queue,
                 WaitingQueue< std::list< std::string > >& console_queue,
                 int size );
    
    void Add( std::string&& cmd );
    void Stop();
    
private:

    WaitingQueue< std::list< std::string > >& mWaitingQueue;
    WaitingQueue< std::list< std::string > >& mConsoleWaitingQueue;
    
    StaticBulk mStaticBulk;
    Bulk mDynamicBulk;
    
    std::mutex mtx;

    int mDynamicCount;
};

