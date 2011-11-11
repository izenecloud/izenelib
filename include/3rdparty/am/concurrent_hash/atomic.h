#ifndef CONCURRENT_ATOMIC_H
#define CONCURRENT_ATOMIC_H

namespace concurrent{

namespace detail
{

inline void atomic_fence_acquire()
{
asm volatile("lfence" : : : "memory");
}
inline void atomic_fence_release()
{
asm volatile("sfence" : : : "memory");
}
inline void atomic_fence_seq_cst()
{
asm volatile("mfence" : : : "memory");
}
inline void reorder_partition()
{
asm volatile("" : : : "memory");
}


}

}
#endif
