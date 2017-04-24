#include <cstdio>
#include <cstdarg>
#include <atomic>
#include <thread>
#include <functional>

#include "log.hpp"

//========================================================================================================================================================================================================================
// In C11 standard thread model the functions that operate on IO streams are thread safe : 
//    - each stream has an associated lock that is used to prevent data races when multiple threads of execution access a stream, and to restrict the interleaving of stream operations performed by multiple threads  
//    - only one thread may hold this lock at a time
//    - the lock is reentrant: a single thread may hold the lock multiple times at a given time
//========================================================================================================================================================================================================================

static std::hash<std::thread::id> hasher;

struct logger_t 
{

    static const unsigned int QUEUE_SIZE = 256;
    static const size_t MSG_SIZE = 512;
    static const size_t BUFFER_SIZE = QUEUE_SIZE * MSG_SIZE;

    FILE* file;
    char* buffer;

    std::atomic_uint queue_head;
    std::atomic_bool chunk_free[QUEUE_SIZE];
    uint32_t thread_id_hash;

    logger_t(FILE* file = stdout) : file(file)
    {
        buffer = (char*) malloc(BUFFER_SIZE);
        queue_head = 0;
        for(unsigned int i = 0; i < QUEUE_SIZE; ++i)
            chunk_free[i] = true;    

        std::thread::id this_thread_id = std::this_thread::get_id();
        thread_id_hash = static_cast<uint32_t> (hasher(this_thread_id));

        fprintf(file, "[%08X] : Logging started .. \n", thread_id_hash);
    }
    
    ~logger_t() 
    {
        fprintf(file, "[%08X] : Logging done .. \n", thread_id_hash);
        free(buffer);
    }

    unsigned int reserve_slot()
    {
        while(true)
        {
            bool infinitely_true_value = true;
            unsigned int head = queue_head++;
            head &= 0xFF;
            bool is_head_free = chunk_free[head].compare_exchange_weak(infinitely_true_value, false);
            if (is_head_free) 
                return head;
        }
    }

    char* buffer_chunk(unsigned int slot)
    {
        return buffer + slot * MSG_SIZE;
    }

    void release_slot(unsigned int slot)
    {
        fputs(buffer_chunk(slot), file);
        chunk_free[slot] = true;
    }

};

static logger_t logger;

void impl_debug_msg(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    unsigned int output_slot = logger.reserve_slot();

    std::thread::id this_thread_id = std::this_thread::get_id();
    uint32_t thread_id_hash = static_cast<uint32_t> (hasher(this_thread_id));

    char* output_chunk = logger.buffer_chunk(output_slot);
    char* chunk_ptr = output_chunk;

    *(chunk_ptr++) = '[';
    for(unsigned int i = 0; i < 8; ++i)
    {
        uint32_t hex_digit = thread_id_hash >> 28;
        *(chunk_ptr++) = (hex_digit < 0xA) ? '0' + hex_digit : 'A' + hex_digit - 0xA;
        thread_id_hash <<= 4;
    }

    *(chunk_ptr++) = ']';
    *(chunk_ptr++) = ' ';
    *(chunk_ptr++) = ':';
    *(chunk_ptr++) = ' ';

    vsprintf(chunk_ptr, format, args);
    logger.release_slot(output_slot);
    va_end(args);
}

void impl_put_msg(const char* msg)
{
    fputs(msg, logger.file);
}
