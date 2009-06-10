
#ifndef SMALL_OBJ_ALLOCATOR
#define SMALL_OBJ_ALLOCATOR

#include <iostream>
#include <types.h>
#include "fixed_allocator.hpp"

#include <cassert>
#include <algorithm>
#include <time.h>

NS_IZENELIB_AM_BEGIN

template<
  uint8_t  MAX_SMALL_OBJECT_SIZE = 64,
  uint64_t CHUNK_SIZE = MAX_SMALL_OBJECT_SIZE*256
  >
class SmallObjAlloc
{
  typedef FixedAllocator<CHUNK_SIZE> FixedAllocatorT;
  
  FixedAllocatorT* fix_alloc_ptrs_[MAX_SMALL_OBJECT_SIZE];
  
public:
  SmallObjAlloc()
  {
    for (uint8_t i=0; i<MAX_SMALL_OBJECT_SIZE; i++)
      fix_alloc_ptrs_[i] = NULL;//new FixedAllocatorT(i+1);
  }

  void* allocate(std::size_t size)
  {
    if (size>MAX_SMALL_OBJECT_SIZE)
      return operator new(size);

    if (fix_alloc_ptrs_[size-1] == NULL)
      fix_alloc_ptrs_[size-1] = new FixedAllocatorT(size);

    return fix_alloc_ptrs_[size-1]->allocate();
  }

  void deallocate(void* p, std::size_t size)
  {
    if (size> MAX_SMALL_OBJECT_SIZE)
      return operator delete( p);

    assert(fix_alloc_ptrs_[size-1] != NULL);
    
    return fix_alloc_ptrs_[size-1]->deallocate(p);
  }
  
private:
  
  SmallObjAlloc(const SmallObjAlloc&);
  SmallObjAlloc& operator=(const SmallObjAlloc&);

}
  ;
  
NS_IZENELIB_AM_END
#endif // SMALLOBJ_INC_
