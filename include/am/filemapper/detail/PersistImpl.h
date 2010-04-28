#ifndef PERSISTIMPL_H
#define PERSISTIMPL_H

#include <am/filemapper/persist.h>
#ifndef WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif
// Whether to reuse freed memory (yes, you want to do this)
#define RECYCLE 1
// Whether to report memory allocations and deallocations
#define TRACE_ALLOCS 0
// Whether to include extra debugging information (slightly slower, bigger heap)
#define CHECK_MEM 0


NS_IZENELIB_AM_BEGIN

#ifndef WIN32
class PersistImplBase
{
protected:
    int fd;
    pthread_mutex_t memMutex;
    pthread_mutex_t userMutex;
    int mapFlags;
    size_t pageSize;

public:
    PersistImplBase();
    ~PersistImplBase();
    void setPageSize(size_t size);
};

#else
class PersistImplBase
{
protected:
    HANDLE hFile;
    HANDLE hMapFile;
    HANDLE hUserMutex;
    HANDLE hMemoryMutex;
    HANDLE hEvent;

    const char *sharename;	// TODO: A string?
    int mapFlags;
    size_t pageSize;

    CRITICAL_SECTION userCriticalSection, memoryCriticalSection;

public:
    PersistImplBase();
    ~PersistImplBase();

    void setPageSize(size_t size);
};

#endif


class PersistImpl : public PersistImplBase
{
    struct MappedData *mappedData;
    int num_mapped_blocks;
    int flags;

    void *map(size_t offset, size_t length, void *base=0);
    void unmap(void *base, size_t length);

    void openFile(const char *filename);
    void closeFile();

    void lockMem();
    void unlockMem();

    void initialize_map();
    void remapFile(size_t newLen);
    void addBlock(size_t size);

    void unmapAll();

    void mapBlocks();
    void updateMappedBlocks();
    void mapBase();
    void roundToPageSize(size_t &size);

public:
    PersistImpl();
    ~PersistImpl();

    void open(const char*, int uid, int flags, size_t init_size);
    void close();

    void *malloc(size_t);
    void free(void*);
    void free(void*, size_t);
    void *root() const;
    bool empty() const;
    bool usable() const;

    bool lock(int ms);
    void unlock();
    void signal();
    bool wait(int ms);
};


// object_cell
//
// "free_space" is a table of free blocks.  We round the size up using
// object_cell() into 64 discrete sizes, 4, 6, 8, 12, 16, 24, 32 ...
//
// Returns the cell number, and also rounds req_size up to the cell size

// TODO: Must align at 8-byte boundary!!!

inline int object_cell(size_t &req_size)
{
    int cell=0;
    size_t cell_size=2*sizeof(void*);

    // TODO: A more efficient way of extracting just the top two bits of a number

    while (cell<64) // NB that's just 32 bits!
    {
        size_t s0 = cell_size>>1;

        if (req_size <= cell_size)
        {
            req_size = cell_size;
            return cell;
        }
        cell++;
        cell_size += s0;

        if (req_size <= cell_size)
        {
            req_size = cell_size;
            return cell;
        }
        cell++;
        cell_size += s0;
    }

    return 0;   // Failure
}


NS_IZENELIB_AM_END

#endif
