#ifndef PERSIST_H
#define PERSIST_H

#include <cassert>
#include <iostream>
#include <memory>

#include "persist_fwd.h"
#include "detail/MappedData.h"
#include "detail/PersistImpl.h"

NS_IZENELIB_AM_BEGIN

// map_file
// A wrapper around a block of shared memory.
// This provides memory management functions, locking, and
// extends the heap when necessary.
class map_file
{
    class PersistImpl *impl;
    static map_file *global;  // The global pointer

    // Disable copying
    map_file &operator=(const map_file&);
    map_file(const map_file&);

public:

    map_file(const char *filename,int uid,int flags,size_t init_size);

    ~map_file();

    void open(const char *filename,int uid,int flags,size_t init_size);

    void close();

    void *malloc(size_t);
    void free(void*);
    void free(void*, size_t);

    bool lock(int ms=0);    // Mutex the entire heap
    void unlock();          // Release the entire heap

    bool wait(int ms=0);    // Wait for event
    void signal();          // Signal event

    void *root() const;     // The root object

    static map_file *instance()
    {
        return global;
    }

    // Returns true if the heap is empty: no objects have yet been created
    bool empty() const;

    // Returns true if the heap is valid and usable
    operator bool() const;
};


// An allocator compatible with the STL
// This allocator uses the default map_file.
// We don't actually include a reference to the shared file here, since that
// would get stored in persistent memory.
template<class T>
class allocator
{
private:
    map_file * map_instance_;
public:
    // Construct from another allocator
    template<class O>
    allocator(const allocator<O>&) :map_instance_(NULL)
    {
        map_instance_ = map_file::instance();
    }

    allocator():map_instance_(NULL)
    {
        map_instance_ = map_file::instance();
    }

    allocator(map_file* instance):map_instance_(instance) {}


    typedef T value_type;
    typedef const T *const_pointer;
    typedef T *pointer;
    typedef const T &const_reference;
    typedef T &reference;
    typedef long difference_type;
    typedef size_t size_type;

    pointer allocate(size_type n)
    {
        if (!map_instance_) throw std::bad_alloc();
        pointer p = static_cast<pointer>(map_instance_->malloc(n * sizeof(T)));
        if (!p) throw std::bad_alloc();

        return p;
    }

    void deallocate(pointer p, size_type count)
    {
        map_instance_->free(p, count * sizeof(T));
    }

    void construct(pointer p, const_reference v)
    {
        new(p) T(v);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

    template<class Other>
    struct rebind
    {
        typedef izenelib::am::allocator<Other> other;
    };

    bool operator==(const allocator<T> &) const
    {
        return true;
    }
};


// map_data
// A type-safe wrapper around the root object of a map_file
// It also constructs a new object when the file is empty
template<class T>
class map_data : public map_file
{
public:
    map_data(const char *filename,int uid=0,int flags = auto_grow|lock_cs, size_t init_size=0)
            :map_file(filename, uid, flags, init_size)
    {
        if (empty())
        {
            new(*this) T();
        }
    }

    T &operator*()
    {
        return *static_cast<T*>(root());
    }

    T *operator->()
    {
        return static_cast<T*>(root());
    }
};


NS_IZENELIB_AM_END

// operator new
//
// Allocates space for one object in the shared memory

void *operator new(size_t size, izenelib::am::map_file &file);

// operator delete
//
// Matches operator new.  Not used.

void operator delete(void *p, izenelib::am::map_file &file);

#endif

