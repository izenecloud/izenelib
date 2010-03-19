#include <am/filemapper/persist.h>


NS_IZENELIB_AM_BEGIN


map_file *map_file::global = 0;


map_file::map_file(const char *filename, int uid, int f, size_t init_size)
{
    impl = new PersistImpl;

    try
    {
        open(filename, uid, f, init_size);

        global = this;
    }
    catch (...)
    {
        delete impl;
        throw;
    }
}


map_file::~map_file()
{
    close();

    if (this==global)
        global = 0;

    delete impl;
}


void map_file::open(const char *filename, int uid, int f, size_t init_size)
{
    impl->open(filename, uid, f, init_size);
}


void map_file::close()
{
    impl->close();
}


void *map_file::malloc(size_t size)
{
    return impl->malloc(size);
}


void map_file::free(void *block)
{
    impl->free(block);
}


void map_file::free(void *block, size_t size)
{
    impl->free(block,size);
}


// map_file::root
//
// Returns a pointer to the first object in the heap.

void *map_file::root() const
{
    return impl->root();
}


// map_file::empty
//
// Returns true of the heap is empty - no objects have been allocated.
// This tells us if we need to construct a root object.

bool map_file::empty() const
{
    return impl->empty();
}


map_file::operator bool() const
{
    return impl->usable();
}


bool map_file::lock(int ms)
{
    return impl->lock(ms);
}


void map_file::unlock()
{
    return impl->unlock();
}

NS_IZENELIB_AM_END



// operator new
//
// Allocates space for one object in the shared memory

void *operator new(size_t size, izenelib::am::map_file &file)
{
    void *p = file.malloc(size);

    if (!p) throw std::bad_alloc();

    return p;
}


// operator delete
//
// Matches operator new.  Not used.

void operator delete(void *p, izenelib::am::map_file &file)
{
    assert(0);
}

