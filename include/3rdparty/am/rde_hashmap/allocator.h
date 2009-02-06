#ifndef RDESTL_ALLOCATOR_H
#define RDESTL_ALLOCATOR_H

#include <stdlib.h>

namespace rde
{

// CONCEPT!
class allocator
{
public:
    explicit allocator(const char* name = "DEFAULT"):	m_name(name) {}
    ~allocator() {}

    void* allocate(size_t bytes, int flags = 0){return operator new(bytes);}
    void deallocate(void* ptr, size_t bytes)
    {
        if (ptr)
            operator delete(ptr);
    }
    const char* get_name() const{return m_name;}

private:
    const char*	m_name;
};

} // namespace rde

//-----------------------------------------------------------------------------
#endif // #ifndef RDESTL_ALLOCATOR_H
