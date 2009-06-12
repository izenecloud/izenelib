#include <libkern/OSAtomic.h>

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedIncrement(volatile LPLONG lpAddend)
{
	return OSAtomicIncrement32 ((int32_t*)&lpAddend);
}

__forceinline LONG WINAPI InterlockedDecrement(volatile LPLONG lpAddend)
{
	return OSAtomicDecrement32 ((int32_t*)&lpAddend);
 
}


// -------------------------------------------------------------------------
// CompareAndSwap128

#if defined(__GNUC__) && defined(__x86_64__) &&                       \
    ( __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 ) ||                        \
    ( (__GNUC__ >  4) || ( (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 2) ) && defined(__nocona__ ))

__forceinline bool WINAPI CompareAndSwap128(
	volatile LONG64 addr[2], LONG64 old1, LONG64 old2, LONG64 new1, LONG64 new2)
{
	typedef int ATOMIC_LONG128_ __attribute__ ((mode (TI)));
	typedef ATOMIC_LONG128_ ATOMIC_LONG128;
	typedef ATOMIC_LONG128_* PATOMIC_LONG128;

	return __sync_bool_compare_and_swap_16(
		(volatile PATOMIC_LONG128)addr,
		old1 | ((ATOMIC_LONG128)old2 << 64),
		new1 | ((ATOMIC_LONG128)new2 << 64));	
}

#elif defined(__GNUC__) && defined(__x86_64__)

__forceinline bool WINAPI CompareAndSwap128(
	volatile LONG64 addr[2], LONG64 old1, LONG64 old2, LONG64 new1, LONG64 new2)
{
    /* handcoded asm, will crash on early amd processors */
    char result;
    __asm__ __volatile__("lock; cmpxchg16b %0; setz %1"
                         : "=m"(*addr), "=q"(result)
                         : "m"(*addr), "d" (old2), "a" (old1),
                           "c" (new2), "b" (new1) : "memory");
    return (bool)result;
}

#else

#define BOOST_DETAIL_NO_COMPARE_AND_SWAP_128

#endif