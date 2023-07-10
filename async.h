#pragma once

#include <list>
#include "waiting_queue.h"

void* connect( WaitingQueue< std::list< std::string > >& queue,
               WaitingQueue< std::list< std::string > >& console_queue,
               int size );

void recieve( void* context, char* buffer, size_t buffer_size );

void disconnect( void* context );

