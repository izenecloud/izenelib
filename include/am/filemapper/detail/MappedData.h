#ifndef MAPPEDDATA_H
#define MAPPEDDATA_H

NS_IZENELIB_AM_BEGIN

#ifndef WIN32
#include <pthread.h>

struct MappedDataBase
{
    pthread_mutex_t memMutex;
    pthread_mutex_t userMutex;

    MappedDataBase()
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&memMutex, &attr);
        pthread_mutex_init(&userMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
};
#else
struct MappedDataBase
{
};
#endif

struct MappedData : public MappedDataBase
{

    int created;	// A boolean: true if this has been created

    size_t file_length;
    bool auto_grow;         // Whether this length should be increased on demand

    int uid;

    void *bottom;
    char *top;
    void *end;

    struct mapped_block
    {
        void *base;
        size_t file_offset;
        size_t size;
    };

    void *free_space[64];   // An embarrassingly simple memory manager

    //num_blocks*block_size equals to the maximum size that the file mapper supported
    enum { block_size = 1<<20, num_blocks = 1<<20 };

    int num_mapped_blocks;	// The number of mapped blocks
    mapped_block blocks[num_blocks];

    MappedData()
    {
        top=0;
        end=0;
        num_mapped_blocks=0;
        created=1;
        // TODO: Do more here
    }
};


NS_IZENELIB_AM_END

#endif
