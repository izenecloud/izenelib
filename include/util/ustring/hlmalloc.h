#ifndef HLMALLOC_H_
#define HLMALLOC_H_

#include <assert.h>
#include <boost/thread.hpp>

#ifndef WIN32
#ifdef _WIN32
#define WIN32 1
#endif
#endif
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#define HAVE_MMAP 1
#define HAVE_MORECORE 0
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#define MALLOC_FAILURE_ACTION
#define MMAP_CLEARS 0
#endif

#define MAX_SIZE_T (~0U)

#ifndef ONLY_MSPACES
#define ONLY_MSPACES 0
#endif
#ifndef MSPACES
#if ONLY_MSPACES
#define MSPACES 1
#else
#define MSPACES 0
#endif
#endif
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT (8U)
#endif
#ifndef FOOTERS
#define FOOTERS 0
#endif
//#ifndef ABORT
//#define ABORT  abort()
//#endif
#ifndef ABORT_ON_ASSERT_FAILURE
#define ABORT_ON_ASSERT_FAILURE 1
#endif
#ifndef PROCEED_ON_ERROR
#define PROCEED_ON_ERROR 0
#endif
#ifndef USE_LOCKS
#define USE_LOCKS 0
#endif
#ifndef INSECURE
#define INSECURE 0
#endif
#ifndef HAVE_MMAP
#define HAVE_MMAP 1
#endif
#ifndef MMAP_CLEARS
#define MMAP_CLEARS 1
#endif
#ifndef HAVE_MREMAP
#ifdef linux
#define HAVE_MREMAP 1
#else
#define HAVE_MREMAP 0
#endif
#endif
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION  errno = ENOMEM;
#endif
#ifndef HAVE_MORECORE
#if ONLY_MSPACES
#define HAVE_MORECORE 0
#else
#define HAVE_MORECORE 1
#endif
#endif
#if !HAVE_MORECORE
#define MORECORE_CONTIGUOUS 0
#else
#ifndef MORECORE
#define MORECORE sbrk
#endif
#ifndef MORECORE_CONTIGUOUS
#define MORECORE_CONTIGUOUS 1
#endif
#endif
#ifndef DEFAULT_GRANULARITY
#if MORECORE_CONTIGUOUS
#define DEFAULT_GRANULARITY (0)
#else
#define DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#endif
#endif
#ifndef DEFAULT_TRIM_THRESHOLD
#ifndef MORECORE_CANNOT_TRIM
#define DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#else
#define DEFAULT_TRIM_THRESHOLD MAX_SIZE_T
#endif
#endif
#ifndef DEFAULT_MMAP_THRESHOLD
#if HAVE_MMAP
#define DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#else
#define DEFAULT_MMAP_THRESHOLD MAX_SIZE_T
#endif
#endif
#ifndef USE_BUILTIN_FFS
#define USE_BUILTIN_FFS 0
#endif
#ifndef USE_DEV_RANDOM
#define USE_DEV_RANDOM 0
#endif
#ifndef NO_MALLINFO
#define NO_MALLINFO 0
#endif
#ifndef MALLINFO_FIELD_TYPE
#define MALLINFO_FIELD_TYPE size_t
#endif

#ifdef MS_WIN
#define M_TRIM_THRESHOLD     (-1)
#endif
#define M_GRANULARITY        (-2)

#ifdef MS_WIN
#define M_MMAP_THRESHOLD     (-3)
#endif

//#if !NO_MALLINFO

//#ifdef HAVE_USR_INCLUDE_MALLOC_H
#ifdef MS_WIN
#define HAVE_USR_INCLUDE_MALLOC_H
#include <malloc.h>
#else
#define HAVE_USR_INCLUDE_MALLOC_H
#include "/usr/include/malloc.h"
#endif

//#else

#ifdef MS_WIN
 struct mallinfo 
 { 
   MALLINFO_FIELD_TYPE arena; 
   MALLINFO_FIELD_TYPE ordblks; 
   MALLINFO_FIELD_TYPE smblks; 
   MALLINFO_FIELD_TYPE hblks; 
   MALLINFO_FIELD_TYPE hblkhd; 
   MALLINFO_FIELD_TYPE usmblks; 
   MALLINFO_FIELD_TYPE fsmblks; 
   MALLINFO_FIELD_TYPE uordblks; 
   MALLINFO_FIELD_TYPE fordblks; 
   MALLINFO_FIELD_TYPE keepcost; 
 }; 
#endif

/* #endif */
//#endif

//#ifdef __cplusplus
//extern "C"
//{
//#endif

#if !ONLY_MSPACES


#ifndef USE_HL_PREFIX

#endif

#endif

//#ifdef __cplusplus
//}
//#endif

#ifdef WIN32
#pragma warning(disable : 4146)
#endif

#include <stdio.h>

#ifndef LACKS_ERRNO_H
#include <errno.h>
#endif
#if FOOTERS
#include <time.h>
#endif
#ifndef LACKS_STDLIB_H
#include <stdlib.h>
#endif
#ifndef LACKS_STRING_H
#include <string.h>
#endif
#if USE_BUILTIN_FFS
#ifndef LACKS_STRINGS_H
#include <strings.h>
#endif
#endif
#if HAVE_MMAP
#ifndef LACKS_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifndef LACKS_FCNTL_H
#include <fcntl.h>
#endif
#endif
#if HAVE_MORECORE
#ifndef LACKS_UNISTD_H
#include <unistd.h>
#else
#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
extern void*  sbrk(ptrdiff_t);
#endif
#endif
#endif


#ifndef WIN32
#ifndef malloc_getpagesize
#  ifdef _SC_PAGESIZE
#    ifndef _SC_PAGE_SIZE
#      define _SC_PAGE_SIZE _SC_PAGESIZE
#    endif
#  endif
#  ifdef _SC_PAGE_SIZE
#    define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#  else
#    if defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       extern size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    else
#      ifdef WIN32
#        define malloc_getpagesize getpagesize()
#      else
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        else
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            else
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          else
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            else
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              else
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#endif
#endif


#define SIZE_T_SIZE         (sizeof(size_t))
#define SIZE_T_BITSIZE      (sizeof(size_t) << 3)

#define SIZE_T_ZERO ((size_t)0)
#define SIZE_T_ONE ((size_t)1)
#define SIZE_T_TWO ((size_t)2)
#define TWO_SIZE_T_SIZES (SIZE_T_SIZE << 1)
#define FOUR_SIZE_T_SIZES (SIZE_T_SIZE << 2)
#define SIX_SIZE_T_SIZES (FOUR_SIZE_T_SIZES + TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T (MAX_SIZE_T >> 1)

#define CHUNK_ALIGN_MASK (MALLOC_ALIGNMENT - SIZE_T_ONE)

#define is_aligned(A) (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

#define align_offset(A)\
 ((((size_t)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))

#define MFAIL  ((void*)(MAX_SIZE_T))
#define CMFAIL  ((char*)(MFAIL))

class HLmemory
{
 public:
  //static boost::mutex mutex_;
#if !HAVE_MMAP
#define IS_MMAPPED_BIT (SIZE_T_ZERO)
#define USE_MMAP_BIT (SIZE_T_ZERO)
#define CALL_MMAP(s) MFAIL
#define CALL_MUNMAP(a, s) (-1)
#define DIRECT_MMAP(s) MFAIL

#else
#define IS_MMAPPED_BIT (SIZE_T_ONE)
#define USE_MMAP_BIT (SIZE_T_ONE)

#ifndef WIN32
#define CALL_MUNMAP(a, s) munmap((a), (s))
#define MMAP_PROT (PROT_READ|PROT_WRITE)
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS        MAP_ANON
#endif
#ifdef MAP_ANONYMOUS
#define MMAP_FLAGS (MAP_PRIVATE|MAP_ANONYMOUS)
#define CALL_MMAP(s) mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#else

#define MMAP_FLAGS MAP_PRIVATE)
static int dev_zero_fd = -1;
#define CALL_MMAP(s) ((dev_zero_fd < 0) ? \
           (dev_zero_fd = open("/dev/zero", O_RDWR), \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#endif

#define DIRECT_MMAP(s)       CALL_MMAP(s)
#else

static void* win32mmap(size_t size)
{
  void* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  return (ptr != 0)? ptr: MFAIL;
}

static void* win32direct_mmap(size_t size)
{
  void* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN, PAGE_READWRITE);
  return (ptr != 0)? ptr: MFAIL;
}

static int win32munmap(void* ptr, size_t size)
{
  MEMORY_BASIC_INFORMATION minfo;
  char* cptr = (char*)ptr;
  while (size) {
    if (VirtualQuery(cptr, &minfo, sizeof(minfo)) == 0)
      return -1;
    if (minfo.BaseAddress != cptr || minfo.AllocationBase != cptr ||
        minfo.State != MEM_COMMIT || minfo.RegionSize > size)
      return -1;
    if (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
      return -1;
    cptr += minfo.RegionSize;
    size -= minfo.RegionSize;
  }
  return 0;
}

#define CALL_MMAP(s) win32mmap(s)
#define CALL_MUNMAP(a, s) win32munmap((a), (s))
#define DIRECT_MMAP(s) win32direct_mmap(s)
#endif
#endif

#if HAVE_MMAP && HAVE_MREMAP
#define CALL_MREMAP(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#else
#define CALL_MREMAP(addr, osz, nsz, mv) MFAIL
#endif

#if HAVE_MORECORE
#define CALL_MORECORE(S) MORECORE(S)
#else
#define CALL_MORECORE(S) MFAIL
#endif

#define USE_NONCONTIGUOUS_BIT (4U)

#define EXTERN_BIT (8U)

#if USE_LOCKS
#ifndef WIN32

#include <pthread.h>
#define MLOCK_T pthread_mutex_t
#define INITIAL_LOCK(l) pthread_mutex_init(l, NULL)
#define ACQUIRE_LOCK(l) pthread_mutex_lock(l)
#define RELEASE_LOCK(l) pthread_mutex_unlock(l)

#if HAVE_MORECORE
static MLOCK_T morecore_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static MLOCK_T magic_init_mutex = PTHREAD_MUTEX_INITIALIZER;

#else

#define MLOCK_T long
static int win32_acquire_lock (MLOCK_T *sl)
{
  for(;;)
  {
#ifdef InterlockedCompareExchangePointer
    if(!InterlockedCompareExchange(sl, 1, 0))
      return 0;
#else
    if(!InterlockedCompareExchange((void**)sl, (void*)1, (void*)0))
      return 0;
#endif
    Sleep(0);
  }
}

static void win32_release_lock (MLOCK_T *sl) { InterlockedExchange (sl, 0); }

#define INITIAL_LOCK(l) *(l)=0
#define ACQUIRE_LOCK(l) win32_acquire_lock(l)
#define RELEASE_LOCK(l) win32_release_lock(l)
#if HAVE_MORECORE
static MLOCK_T morecore_mutex;
#endif
static MLOCK_T magic_init_mutex;
#endif

#define USE_LOCK_BIT               (2U)
#else
#define USE_LOCK_BIT               (0U)
#define INITIAL_LOCK(l)
#endif

#if USE_LOCKS && HAVE_MORECORE
#define ACQUIRE_MORECORE_LOCK()    ACQUIRE_LOCK(&morecore_mutex);
#define RELEASE_MORECORE_LOCK()    RELEASE_LOCK(&morecore_mutex);
#else
#define ACQUIRE_MORECORE_LOCK()
#define RELEASE_MORECORE_LOCK()
#endif

#if USE_LOCKS
#define ACQUIRE_MAGIC_INIT_LOCK() ACQUIRE_LOCK(&magic_init_mutex);
#define RELEASE_MAGIC_INIT_LOCK() RELEASE_LOCK(&magic_init_mutex);
#else
#define ACQUIRE_MAGIC_INIT_LOCK()
#define RELEASE_MAGIC_INIT_LOCK()
#endif

struct malloc_chunk
{
  size_t prev_foot;
  size_t head;
  struct malloc_chunk* fd;
  struct malloc_chunk* bk;
};

typedef struct malloc_chunk  mchunk;
typedef struct malloc_chunk* mchunkptr;
typedef struct malloc_chunk* sbinptr;

#define MCHUNK_SIZE         (sizeof(mchunk))

#if FOOTERS
#define CHUNK_OVERHEAD (TWO_SIZE_T_SIZES)
#else
#define CHUNK_OVERHEAD (SIZE_T_SIZE)
#endif

#define MMAP_CHUNK_OVERHEAD (TWO_SIZE_T_SIZES)

#define MMAP_FOOT_PAD (FOUR_SIZE_T_SIZES)

#define MIN_CHUNK_SIZE ((MCHUNK_SIZE + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

#define chunk2mem(p) ((void*)((char*)(p) + TWO_SIZE_T_SIZES))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - TWO_SIZE_T_SIZES))
#define align_as_chunk(A) (mchunkptr)((A) + align_offset(chunk2mem(A)))

#define MAX_REQUEST ((-MIN_CHUNK_SIZE) << 2)
#define MIN_REQUEST (MIN_CHUNK_SIZE - CHUNK_OVERHEAD - SIZE_T_ONE)

#define pad_request(req) (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

#define request2size(req) (((req) < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(req))

#define PINUSE_BIT (SIZE_T_ONE)
#define CINUSE_BIT (SIZE_T_TWO)
#define INUSE_BITS (PINUSE_BIT|CINUSE_BIT)

#define FENCEPOST_HEAD (INUSE_BITS|SIZE_T_SIZE)

#define cinuse(p) ((p)->head & CINUSE_BIT)
#define pinuse(p) ((p)->head & PINUSE_BIT)
#define chunksize(p) ((p)->head & ~(INUSE_BITS))

#define clear_pinuse(p) ((p)->head &= ~PINUSE_BIT)
#define clear_cinuse(p) ((p)->head &= ~CINUSE_BIT)

#define chunk_plus_offset(p, s) ((mchunkptr)(((char*)(p)) + (s)))
#define chunk_minus_offset(p, s) ((mchunkptr)(((char*)(p)) - (s)))

#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->head & ~INUSE_BITS)))
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_foot) ))

#define next_pinuse(p) ((next_chunk(p)->head) & PINUSE_BIT)

#define get_foot(p, s) (((mchunkptr)((char*)(p) + (s)))->prev_foot)
#define set_foot(p, s) (((mchunkptr)((char*)(p) + (s)))->prev_foot = (s))

#define set_size_and_pinuse_of_free_chunk(p, s)\
  ((p)->head = (s|PINUSE_BIT), set_foot(p, s))

#define set_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s))

#define is_mmapped(p) (!((p)->head & PINUSE_BIT) && ((p)->prev_foot & IS_MMAPPED_BIT))

#define overhead_for(p) (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)

#if MMAP_CLEARS
#define calloc_must_clear(p) (!is_mmapped(p))
#else
#define calloc_must_clear(p) (1)
#endif

struct malloc_tree_chunk
{

  size_t prev_foot;
  size_t head;
  struct malloc_tree_chunk* fd;
  struct malloc_tree_chunk* bk;

  struct malloc_tree_chunk* child[2];
  struct malloc_tree_chunk* parent;
  size_t index;
};

typedef struct malloc_tree_chunk  tchunk;
typedef struct malloc_tree_chunk* tchunkptr;
typedef struct malloc_tree_chunk* tbinptr;

#define leftmost_child(t) ((t)->child[0] != 0? (t)->child[0] : (t)->child[1])

struct malloc_segment
{
  char*  base;
  size_t size;
  struct malloc_segment* next;
  size_t sflags;
};

#define is_mmapped_segment(S)  ((S)->sflags & IS_MMAPPED_BIT)
#define is_extern_segment(S)   ((S)->sflags & EXTERN_BIT)

typedef struct malloc_segment  msegment;
typedef struct malloc_segment* msegmentptr;

#define NSMALLBINS (32U)
#define NTREEBINS  (32U)
#define SMALLBIN_SHIFT  (3U)
#define SMALLBIN_WIDTH  (SIZE_T_ONE << SMALLBIN_SHIFT)
#define TREEBIN_SHIFT  (8U)
#define MIN_LARGE_SIZE (SIZE_T_ONE << TREEBIN_SHIFT)
#define MAX_SMALL_SIZE  (MIN_LARGE_SIZE - SIZE_T_ONE)
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD)

struct malloc_state
{
  size_t smallmap;
  size_t treemap;
  size_t dvsize;
  size_t topsize;
  char* least_addr;
  mchunkptr dv;
  mchunkptr top;
  size_t trim_check;
  size_t magic;
  mchunkptr smallbins[(NSMALLBINS + 1) << 1];
  tbinptr treebins[NTREEBINS];
  size_t footprint;
  size_t max_footprint;
  size_t mflags;
#if USE_LOCKS
  MLOCK_T    mutex;
#endif
  msegment   seg;
};

//typedef struct malloc_state*  mstate;

struct malloc_params
{
  size_t magic;
  size_t page_size;
  size_t granularity;
  size_t mmap_threshold;
  size_t trim_threshold;
  size_t default_mflags;
};

static struct malloc_params mparams;

static struct malloc_state _gm_;

#define gm  (&_gm_)
#define is_global(M) ((M) == &_gm_)
#define _is_initialized(M) ((M)->top != 0)

#define use_lock(M)  ((M)->mflags & USE_LOCK_BIT)
#define enable_lock(M) ((M)->mflags |= USE_LOCK_BIT)
#define disable_lock(M) ((M)->mflags &= ~USE_LOCK_BIT)

#define use_mmap(M) ((M)->mflags & USE_MMAP_BIT)
#define enable_mmap(M) ((M)->mflags |= USE_MMAP_BIT)
#define disable_mmap(M) ((M)->mflags &= ~USE_MMAP_BIT)

#define use_noncontiguous(M) ((M)->mflags & USE_NONCONTIGUOUS_BIT)
#define disable_contiguous(M) ((M)->mflags |= USE_NONCONTIGUOUS_BIT)

#define set_lock(M,L) ((M)->mflags = (L)? ((M)->mflags | USE_LOCK_BIT) : ((M)->mflags & ~USE_LOCK_BIT))

#define page_align(S)\
 (((S) + (mparams.page_size)) & ~(mparams.page_size - SIZE_T_ONE))

#define granularity_align(S) (((S) + (mparams.granularity)) & ~(mparams.granularity - SIZE_T_ONE))

#define is_page_aligned(S) (((size_t)(S) & (mparams.page_size - SIZE_T_ONE)) == 0)
#define is_granularity_aligned(S) (((size_t)(S) & (mparams.granularity - SIZE_T_ONE)) == 0)

#define segment_holds(S, A) ((char*)(A) >= S->base && (char*)(A) < S->base + S->size)

static msegmentptr segment_holding(struct malloc_state* m, char* addr)
{
  msegmentptr sp = &m->seg;
  for(;;)
  {
    if (addr >= sp->base && addr < sp->base + sp->size)
      return sp;
    if((sp = sp->next) == 0)
      return 0;
  }
}

static int has_segment_link(struct malloc_state* m, msegmentptr ss)
{
  msegmentptr sp = &m->seg;
  for(;;)
  {
    if((char*)sp >= ss->base && (char*)sp < ss->base + ss->size)
      return 1;
    if((sp = sp->next) == 0)
      return 0;
  }
}

#ifndef MORECORE_CANNOT_TRIM
#define should_trim(M,s) ((s) > (M)->trim_check)
#else
#define should_trim(M,s) (0)
#endif

#define TOP_FOOT_SIZE\
  (align_offset(chunk2mem(0))+pad_request(sizeof(struct malloc_segment))+MIN_CHUNK_SIZE)


#if USE_LOCKS

#define GLOBALLY_INITIALIZE() (mparams.page_size == 0 && init_mparams())

#define PREACTION(M) ((GLOBALLY_INITIALIZE() || use_lock(M))? ACQUIRE_LOCK(&(M)->mutex) : 0)
#define POSTACTION(M) { if (use_lock(M)) RELEASE_LOCK(&(M)->mutex); }
#else

#ifndef PREACTION
#define PREACTION(M) (0)
#endif

#ifndef POSTACTION
#define POSTACTION(M)
#endif

#endif

#if PROCEED_ON_ERROR

int malloc_corruption_error_count;

static void reset_on_error(struct malloc_state* m);

#define CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#define USAGE_ERROR_ACTION(m, p)

#else

#ifndef CORRUPTION_ERROR_ACTION
#define CORRUPTION_ERROR_ACTION(m) abort()
#endif

#ifndef USAGE_ERROR_ACTION
#define USAGE_ERROR_ACTION(m,p) abort()
#endif

#endif

#if !DEBUG

#define check_free_chunk(M,P)
#define check_inuse_chunk(M,P)
#define check_malloced_chunk(M,P,N)
#define check_mmapped_chunk(M,P)
#define check_malloc_state(M)
#define check_top_chunk(M,P)

#else
#define check_free_chunk(M,P) do_check_free_chunk(M,P)
#define check_inuse_chunk(M,P) do_check_inuse_chunk(M,P)
#define check_top_chunk(M,P) do_check_top_chunk(M,P)
#define check_malloced_chunk(M,P,N) do_check_malloced_chunk(M,P,N)
#define check_mmapped_chunk(M,P) do_check_mmapped_chunk(M,P)
#define check_malloc_state(M) do_check_malloc_state(M)

/* static void   do_check_any_chunk(struct malloc_state* m, mchunkptr p); */
/* static void   do_check_top_chunk(struct malloc_state* m, mchunkptr p); */
/* static void   do_check_mmapped_chunk(struct malloc_state* m, mchunkptr p); */
/* static void   do_check_inuse_chunk(struct malloc_state* m, mchunkptr p); */
/* static void   do_check_free_chunk(struct malloc_state* m, mchunkptr p); */
/* static void   do_check_malloced_chunk(struct malloc_state* m, void* mem, size_t s); */
/* static void   do_check_tree(struct malloc_state* m, tchunkptr t); */
/* static void   do_check_treebin(struct malloc_state* m, size_t i); */
/* static void   do_check_smallbin(struct malloc_state* m, size_t i); */
/* static void   do_check_malloc_state(struct malloc_state* m); */
/* static int    bin_find(struct malloc_state* m, mchunkptr x); */
/* static size_t traverse_and_check(struct malloc_state* m); */
#endif

#define is_small(s) (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s) ((s) >> SMALLBIN_SHIFT)
#define small_index2size(i) ((i)  << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX (small_index(MIN_CHUNK_SIZE))

#define smallbin_at(M, i) ((sbinptr)((char*)&((M) -> smallbins[(i) << 1])))
#define treebin_at(M,i) (&((M)->treebins[i]))

#if defined(__GNUC__) && defined(i386)
#define compute_tree_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int K;\
    __asm__("bsrl %1,%0\n\t" : "=r" (K) : "rm"  (X));\
    I =  (size_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}
#else
#define compute_tree_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int Y = (unsigned int)X;\
    unsigned int N = ((Y - 0x100) >> 16) & 8;\
    unsigned int K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1));\
  }\
}
#endif

#define bit_for_tree_index(i) \
   (i == NTREEBINS-1)? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

#define leftshift_for_tree_index(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

#define minsize_for_tree_index(i) \
   ((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
   (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))

#define idx2bit(i)              ((size_t)(1) << (i))

#define mark_smallmap(M,i) ((M)->smallmap |= idx2bit(i))
#define clear_smallmap(M,i) ((M)->smallmap &= ~idx2bit(i))
#define smallmap_is_marked(M,i) ((M)->smallmap & idx2bit(i))

#define mark_treemap(M,i) ((M)->treemap  |=  idx2bit(i))
#define clear_treemap(M,i) ((M)->treemap  &= ~idx2bit(i))
#define treemap_is_marked(M,i) ((M)->treemap & idx2bit(i))

#if defined(__GNUC__) && defined(i386)
#define compute_bit2idx(X, I)\
{\
  unsigned int J;\
  __asm__("bsfl %1,%0\n\t" : "=r" (J) : "rm" (X));\
  I = (size_t)J;\
}

#else
#if  USE_BUILTIN_FFS
#define compute_bit2idx(X, I) I = ffs(X)-1

#else
#define compute_bit2idx(X, I)\
{\
  unsigned int Y = X - 1;\
  unsigned int K = Y >> (16-4) & 16;\
  unsigned int N = K;        Y >>= K;\
  N += K = Y >> (8-3) &  8;  Y >>= K;\
  N += K = Y >> (4-2) &  4;  Y >>= K;\
  N += K = Y >> (2-1) &  2;  Y >>= K;\
  N += K = Y >> (1-0) &  1;  Y >>= K;\
  I = (size_t)(N + Y);\
}
#endif
#endif

#define least_bit(x) ((x) & -(x))

#define left_bits(x)         ((x << 1) | -(x << 1))

#define same_or_left_bits(x) ((x) | -(x))

#if !INSECURE

#define ok_address(M, a) ((char*)(a) >= (M)->least_addr)

#define ok_next(p, n)    ((char*)(p) < (char*)(n))

#define ok_cinuse(p) cinuse(p)

#define ok_pinuse(p) pinuse(p)

#else
#define ok_address(M, a) (1)
#define ok_next(b, n) (1)
#define ok_cinuse(p) (1)
#define ok_pinuse(p) (1)
#endif

#if (FOOTERS && !INSECURE)

#define ok_magic(M) ((M)->magic == mparams.magic)
#else
#define ok_magic(M) (1)
#endif

#if !INSECURE
#if defined(__GNUC__) && __GNUC__ >= 3
#define RTCHECK(e)  __builtin_expect(e, 1)
#else
#define RTCHECK(e) (e)
#endif
#else
#define RTCHECK(e) (1)
#endif

#if !FOOTERS

#define mark_inuse_foot(M,p,s)

#define set_inuse(M,p,s)\
  ((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
  ((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

#define set_inuse_and_pinuse(M,p,s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  ((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

#define set_size_and_pinuse_of_inuse_chunk(M, p, s) ((p)->head = (s|PINUSE_BIT|CINUSE_BIT))

#else

#define mark_inuse_foot(M,p,s)\
  (((mchunkptr)((char*)(p) + (s)))->prev_foot = ((size_t)(M) ^ mparams.magic))

#define get_mstate_for(p)\
  ((struct malloc_state*)(((mchunkptr)((char*)(p) + (chunksize(p))))->prev_foot ^ mparams.magic))

#define set_inuse(M,p,s)\
  ((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
  (((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT), \
  mark_inuse_foot(M,p,s))

#define set_inuse_and_pinuse(M,p,s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  (((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT),\
 mark_inuse_foot(M,p,s))

#define set_size_and_pinuse_of_inuse_chunk(M, p, s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  mark_inuse_foot(M, p, s))

#endif

static int init_mparams()
{
  if(mparams.page_size == 0)
  {
    size_t s;

    mparams.mmap_threshold = DEFAULT_MMAP_THRESHOLD;
    mparams.trim_threshold = DEFAULT_TRIM_THRESHOLD;
#if MORECORE_CONTIGUOUS
    mparams.default_mflags = USE_LOCK_BIT|USE_MMAP_BIT;
#else
    mparams.default_mflags = USE_LOCK_BIT|USE_MMAP_BIT|USE_NONCONTIGUOUS_BIT;
#endif

#if (FOOTERS && !INSECURE)
    {
#if USE_DEV_RANDOM
      int fd;
      unsigned char buf[sizeof(size_t)];

      if((fd = open("/dev/urandom", O_RDONLY)) >= 0 && read(fd, buf, sizeof(buf)) == sizeof(buf))
      {
        s = *((size_t *) buf);
        close(fd);
      }
      else
#endif
        s = (size_t)(time(0) ^ (size_t)0x55555555U);

      s |= (size_t)8U;
      s &= ~(size_t)7U;

    }
#else
    s = (size_t)0x58585858U;
#endif
    ACQUIRE_MAGIC_INIT_LOCK();
    if (mparams.magic == 0) {
      mparams.magic = s;
      INITIAL_LOCK(&gm->mutex);
      gm->mflags = mparams.default_mflags;
    }
    RELEASE_MAGIC_INIT_LOCK();

#ifndef WIN32
    mparams.page_size = malloc_getpagesize;
    mparams.granularity = ((DEFAULT_GRANULARITY != 0)? DEFAULT_GRANULARITY : mparams.page_size);
#else
    {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      mparams.page_size = system_info.dwPageSize;
      mparams.granularity = system_info.dwAllocationGranularity;
    }
#endif
    if((sizeof(size_t) != sizeof(char*)) ||
        (MAX_SIZE_T < MIN_CHUNK_SIZE)  ||
        (sizeof(int) < 4)  ||
        (MALLOC_ALIGNMENT < (size_t)8U) ||
        ((MALLOC_ALIGNMENT    & (MALLOC_ALIGNMENT-SIZE_T_ONE))    != 0) ||
        ((MCHUNK_SIZE         & (MCHUNK_SIZE-SIZE_T_ONE))         != 0) ||
        ((mparams.granularity & (mparams.granularity-SIZE_T_ONE)) != 0) ||
        ((mparams.page_size   & (mparams.page_size-SIZE_T_ONE))   != 0))
      abort();
  }
  return 0;
}

static int change_mparam(int param_number, int value)
{
  size_t val = (size_t)value;
  init_mparams();
  switch(param_number)
  {
  case M_TRIM_THRESHOLD:
    mparams.trim_threshold = val;
    return 1;
  case M_GRANULARITY:
    if (val >= mparams.page_size && ((val & (val-1)) == 0))
    {
      mparams.granularity = val;
      return 1;
    }
    else
      return 0;
  case M_MMAP_THRESHOLD:
    mparams.mmap_threshold = val;
    return 1;
  default:
    return 0;
  }
}

#if DEBUG

static void do_check_any_chunk(struct malloc_state* m, mchunkptr p)
{
  assert((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
  assert(ok_address(m, p));
}

static void do_check_top_chunk(struct malloc_state* m, mchunkptr p)
{
  msegmentptr sp = segment_holding(m, (char*)p);
  size_t  sz = chunksize(p);
  assert(sp != 0);
  assert((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
  assert(ok_address(m, p));
  assert(sz == m->topsize);
  assert(sz > 0);
  assert(sz == ((sp->base + sp->size) - (char*)p) - TOP_FOOT_SIZE);
  assert(pinuse(p));
  assert(!next_pinuse(p));
}

static void do_check_mmapped_chunk(struct malloc_state* m, mchunkptr p)
{
  size_t  sz = chunksize(p);
  size_t len = (sz + (p->prev_foot & ~IS_MMAPPED_BIT) + MMAP_FOOT_PAD);
  assert(is_mmapped(p));
  assert(use_mmap(m));
  assert((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
  assert(ok_address(m, p));
  assert(!is_small(sz));
  assert((len & (mparams.page_size-SIZE_T_ONE)) == 0);
  assert(chunk_plus_offset(p, sz)->head == FENCEPOST_HEAD);
  assert(chunk_plus_offset(p, sz+SIZE_T_SIZE)->head == 0);
}

static void do_check_inuse_chunk(struct malloc_state* m, mchunkptr p)
{
  do_check_any_chunk(m, p);
  assert(cinuse(p));
  assert(next_pinuse(p));
  assert(is_mmapped(p) || pinuse(p) || next_chunk(prev_chunk(p)) == p);
  if (is_mmapped(p))
    do_check_mmapped_chunk(m, p);
}

static void do_check_free_chunk(struct malloc_state* m, mchunkptr p)
{
  size_t sz = p->head & ~(PINUSE_BIT|CINUSE_BIT);
  mchunkptr next = chunk_plus_offset(p, sz);
  do_check_any_chunk(m, p);
  assert(!cinuse(p));
  assert(!next_pinuse(p));
  assert (!is_mmapped(p));
  if (p != m->dv && p != m->top)
  {
    if (sz >= MIN_CHUNK_SIZE)
    {
      assert((sz & CHUNK_ALIGN_MASK) == 0);
      assert(is_aligned(chunk2mem(p)));
      assert(next->prev_foot == sz);
      assert(pinuse(p));
      assert (next == m->top || cinuse(next));
      assert(p->fd->bk == p);
      assert(p->bk->fd == p);
    }
    else
      assert(sz == SIZE_T_SIZE);
  }
}

static void do_check_malloced_chunk(struct malloc_state* m, void* mem, size_t s)
{
  if (mem != 0)
  {
    mchunkptr p = mem2chunk(mem);
    size_t sz = p->head & ~(PINUSE_BIT|CINUSE_BIT);
    do_check_inuse_chunk(m, p);
    assert((sz & CHUNK_ALIGN_MASK) == 0);
    assert(sz >= MIN_CHUNK_SIZE);
    assert(sz >= s);
    assert(is_mmapped(p) || sz < (s + MIN_CHUNK_SIZE));
  }
}

static void do_check_tree(struct malloc_state* m, tchunkptr t)
{
  tchunkptr head = 0;
  tchunkptr u = t;
  size_t tindex = t->index;
  size_t tsize = chunksize(t);
  size_t idx;
  compute_tree_index(tsize, idx);
  assert(tindex == idx);
  assert(tsize >= MIN_LARGE_SIZE);
  assert(tsize >= minsize_for_tree_index(idx));
  assert((idx == NTREEBINS-1) || (tsize < minsize_for_tree_index((idx+1))));

  do
  {
    do_check_any_chunk(m, ((mchunkptr)u));
    assert(u->index == tindex);
    assert(chunksize(u) == tsize);
    assert(!cinuse(u));
    assert(!next_pinuse(u));
    assert(u->fd->bk == u);
    assert(u->bk->fd == u);
    if (u->parent == 0)
    {
      assert(u->child[0] == 0);
      assert(u->child[1] == 0);
    }
    else
    {
      assert(head == 0);
      head = u;
      assert(u->parent != u);
      assert (u->parent->child[0] == u ||
              u->parent->child[1] == u ||
              *((tbinptr*)(u->parent)) == u);
      if (u->child[0] != 0)
      {
        assert(u->child[0]->parent == u);
        assert(u->child[0] != u);
        do_check_tree(m, u->child[0]);
      }
      if (u->child[1] != 0)
      {
        assert(u->child[1]->parent == u);
        assert(u->child[1] != u);
        do_check_tree(m, u->child[1]);
      }
      if (u->child[0] != 0 && u->child[1] != 0)
        assert(chunksize(u->child[0]) < chunksize(u->child[1]));
    }
    u = u->fd;
  } while (u != t);
  assert(head != 0);
}

static void do_check_treebin(struct malloc_state* m, size_t i)
{
  tbinptr* tb = treebin_at(m, i);
  tchunkptr t = *tb;
  int empty = (m->treemap & (1U << i)) == 0;
  if (t == 0)
    assert(empty);
  if(!empty)
    do_check_tree(m, t);
}

static void do_check_smallbin(struct malloc_state* m, size_t i)
{
  sbinptr b = smallbin_at(m, i);
  mchunkptr p = b->bk;
  unsigned int empty = (m->smallmap & (1U << i)) == 0;
  if (p == b)
    assert(empty);
  if(!empty)
  {
    for(; p != b; p = p->bk)
    {
      size_t size = chunksize(p);
      mchunkptr q;
      do_check_free_chunk(m, p);
      assert(small_index(size) == i);
      assert(p->bk == b || chunksize(p->bk) == chunksize(p));
      q = next_chunk(p);
      if (q->head != FENCEPOST_HEAD)
        do_check_inuse_chunk(m, q);
    }
  }
}

static int bin_find(struct malloc_state* m, mchunkptr x)
{
  size_t size = chunksize(x);
  if (is_small(size))
  {
    size_t sidx = small_index(size);
    sbinptr b = smallbin_at(m, sidx);
    if (smallmap_is_marked(m, sidx))
    {
      mchunkptr p = b;
      do
      {
        if (p == x)
          return 1;
      } while((p = p->fd) != b);
    }
  }
  else
  {
    size_t tidx;
    compute_tree_index(size, tidx);
    if (treemap_is_marked(m, tidx))
    {
      tchunkptr t = *treebin_at(m, tidx);
      size_t sizebits = size << leftshift_for_tree_index(tidx);
      while (t != 0 && chunksize(t) != size)
      {
        t = t->child[(sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1];
        sizebits <<= 1;
      }
      if (t != 0)
      {
        tchunkptr u = t;
        do {
          if (u == (tchunkptr)x)
            return 1;
        } while((u = u->fd) != t);
      }
    }
  }
  return 0;
}

static size_t traverse_and_check(struct malloc_state* m)
{
  size_t sum = 0;
  if (_is_initialized(m))
  {
    msegmentptr s = &m->seg;
    sum += m->topsize + TOP_FOOT_SIZE;
    while (s != 0)
    {
      mchunkptr q = align_as_chunk(s->base);
      mchunkptr lastq = 0;
      assert(pinuse(q));
      while (segment_holds(s, q) && q != m->top && q->head != FENCEPOST_HEAD)
      {
        sum += chunksize(q);
        if (cinuse(q))
        {
          assert(!bin_find(m, q));
          do_check_inuse_chunk(m, q);
        }
        else
        {
          assert(q == m->dv || bin_find(m, q));
          assert(lastq == 0 || cinuse(lastq));
          do_check_free_chunk(m, q);
        }
        lastq = q;
        q = next_chunk(q);
      }
      s = s->next;
    }
  }
  return sum;
}

static void do_check_malloc_state(struct malloc_state* m)
{
  size_t i;
  size_t total;

  for (i = 0; i < NSMALLBINS; ++i)
    do_check_smallbin(m, i);
  for (i = 0; i < NTREEBINS; ++i)
    do_check_treebin(m, i);

  if (m->dvsize != 0)
  {
    do_check_any_chunk(m, m->dv);
    assert(m->dvsize == chunksize(m->dv));
    assert(m->dvsize >= MIN_CHUNK_SIZE);
    assert(bin_find(m, m->dv) == 0);
  }

  if (m->top != 0)
  {
    do_check_top_chunk(m, m->top);
    assert(m->topsize == chunksize(m->top));
    assert(m->topsize > 0);
    assert(bin_find(m, m->top) == 0);
  }

  total = traverse_and_check(m);
  assert(total <= m->footprint);
  assert(m->footprint <= m->max_footprint);
}
#endif


#if !NO_MALLINFO
static struct mallinfo internal_mallinfo(struct malloc_state* m)
{
  struct mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  if(!PREACTION(m))
  {
    check_malloc_state(m);
    if (_is_initialized(m))
    {
      size_t nfree = SIZE_T_ONE;
      size_t mfree = m->topsize + TOP_FOOT_SIZE;
      size_t sum = mfree;
      msegmentptr s = &m->seg;
      while (s != 0)
      {
        mchunkptr q = align_as_chunk(s->base);
        while (segment_holds(s, q) && q != m->top && q->head != FENCEPOST_HEAD)
        {
          size_t sz = chunksize(q);
          sum += sz;
          if(!cinuse(q))
          {
            mfree += sz;
            ++nfree;
          }
          q = next_chunk(q);
        }
        s = s->next;
      }

      nm.arena    = sum;
      nm.ordblks  = nfree;
      nm.hblkhd   = m->footprint - sum;
      nm.usmblks  = m->max_footprint;
      nm.uordblks = m->footprint - mfree;
      nm.fordblks = mfree;
      nm.keepcost = m->topsize;
    }

    POSTACTION(m);
  }
  return nm;
}
#endif

static void internal_malloc_stats(struct malloc_state* m)
{
  if(!PREACTION(m))
  {
    size_t maxfp = 0;
    size_t fp = 0;
    size_t used = 0;
    check_malloc_state(m);
    if (_is_initialized(m))
    {
      msegmentptr s = &m->seg;
      maxfp = m->max_footprint;
      fp = m->footprint;
      used = fp - (m->topsize + TOP_FOOT_SIZE);

      while (s != 0)
      {
        mchunkptr q = align_as_chunk(s->base);
        while (segment_holds(s, q) && q != m->top && q->head != FENCEPOST_HEAD)
        {
          if(!cinuse(q))
            used -= chunksize(q);
          q = next_chunk(q);
        }
        s = s->next;
      }
    }

    fprintf(stderr, "max system bytes = %10lu\n", (unsigned long)(maxfp));
    fprintf(stderr, "system bytes     = %10lu\n", (unsigned long)(fp));
    fprintf(stderr, "in use bytes     = %10lu\n", (unsigned long)(used));

    POSTACTION(m);
  }
}

#define insert_small_chunk(M, P, S) {\
  size_t I  = small_index(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  assert(S >= MIN_CHUNK_SIZE);\
  if(!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  else if (RTCHECK(ok_address(M, B->fd)))\
    F = B->fd;\
  else {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

#define unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  size_t I = small_index(S);\
  assert(P != B);\
  assert(P != F);\
  assert(chunksize(P) == small_index2size(I));\
  if (F == B)\
    clear_smallmap(M, I);\
  else if (RTCHECK((F == smallbin_at(M,I) || ok_address(M, F)) &&\
                   (B == smallbin_at(M,I) || ok_address(M, B)))) {\
    F->bk = B;\
    B->fd = F;\
  }\
  else {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

#define unlink_first_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  assert(P != B);\
  assert(P != F);\
  assert(chunksize(P) == small_index2size(I));\
  if (B == F)\
    clear_smallmap(M, I);\
  else if (RTCHECK(ok_address(M, F))) {\
    B->fd = F;\
    F->bk = B;\
  }\
  else {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

#define replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  if (DVS != 0) {\
    mchunkptr DV = M->dv;\
    assert(is_small(DVS));\
    insert_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}

#define insert_large_chunk(M, X, S) {\
  tbinptr* H;\
  size_t I;\
  compute_tree_index(S, I);\
  H = treebin_at(M, I);\
  X->index = I;\
  X->child[0] = X->child[1] = 0;\
  if(!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->parent = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  else {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_tree_index(I);\
    for(;;) {\
      if (chunksize(T) != S) {\
        tchunkptr* C = &(T->child[(K >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1]);\
        K <<= 1;\
        if(*C != 0)\
          T = *C;\
        else if (RTCHECK(ok_address(M, C))) {\
          *C = X;\
          X->parent = T;\
          X->fd = X->bk = X;\
          break;\
        }\
        else {\
          CORRUPTION_ERROR_ACTION(M);\
          break;\
        }\
      }\
      else {\
        tchunkptr F = T->fd;\
        if (RTCHECK(ok_address(M, T) && ok_address(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->parent = 0;\
          break;\
        }\
        else {\
          CORRUPTION_ERROR_ACTION(M);\
          break;\
        }\
      }\
    }\
  }\
}


#define unlink_large_chunk(M, X) {\
  tchunkptr XP = X->parent;\
  tchunkptr R;\
  if (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    if (RTCHECK(ok_address(M, F))) {\
      F->bk = R;\
      R->fd = F;\
    }\
    else {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  else {\
    tchunkptr* RP;\
    if(((R = *(RP = &(X->child[1]))) != 0) ||\
        ((R = *(RP = &(X->child[0]))) != 0)) {\
      tchunkptr* CP;\
      while((*(CP = &(R->child[1])) != 0) ||\
             (*(CP = &(R->child[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      if (RTCHECK(ok_address(M, RP)))\
        *RP = 0;\
      else {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  if (XP != 0) {\
    tbinptr* H = treebin_at(M, X->index);\
    if (X == *H) {\
      if((*H = R) == 0) \
        clear_treemap(M, X->index);\
    }\
    else if (RTCHECK(ok_address(M, XP))) {\
      if (XP->child[0] == X) \
        XP->child[0] = R;\
      else \
        XP->child[1] = R;\
    }\
    else\
      CORRUPTION_ERROR_ACTION(M);\
    if (R != 0) {\
      if (RTCHECK(ok_address(M, R))) {\
        tchunkptr C0, C1;\
        R->parent = XP;\
        if((C0 = X->child[0]) != 0) {\
          if (RTCHECK(ok_address(M, C0))) {\
            R->child[0] = C0;\
            C0->parent = R;\
          }\
          else\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        if((C1 = X->child[1]) != 0) {\
          if (RTCHECK(ok_address(M, C1))) {\
            R->child[1] = C1;\
            C1->parent = R;\
          }\
          else\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      else\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}


#define insert_chunk(M, P, S)\
  if (is_small(S)) insert_small_chunk(M, P, S)\
  else { tchunkptr TP = (tchunkptr)(P); insert_large_chunk(M, TP, S); }

#define unlink_chunk(M, P, S)\
  if (is_small(S)) unlink_small_chunk(M, P, S)\
  else { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


#if ONLY_MSPACES
#define internal_malloc(m, b) mspace_malloc(m, b)
#define internal_free(m, mem) mspace_free(m,mem);
#else
#if MSPACES
#define internal_malloc(m, b)\
   (m == gm)? hlmalloc(b) : mspace_malloc(m, b)
#define internal_free(m, mem)\
   if (m == gm) hlfree(mem); else mspace_free(m,mem);
#else
#define internal_malloc(m, b) hlmalloc(b)
#define internal_free(m, mem) hlfree(mem)
#endif
#endif

static void* mmap_alloc(struct malloc_state* m, size_t nb)
{
  size_t mmsize = granularity_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
  if (mmsize > nb)
  {
    char* mm = (char*)(DIRECT_MMAP(mmsize));
    if (mm != CMFAIL) {
      size_t offset = align_offset(chunk2mem(mm));
      size_t psize = mmsize - offset - MMAP_FOOT_PAD;
      mchunkptr p = (mchunkptr)(mm + offset);
      p->prev_foot = offset | IS_MMAPPED_BIT;
      (p)->head = (psize|CINUSE_BIT);
      mark_inuse_foot(m, p, psize);
      chunk_plus_offset(p, psize)->head = FENCEPOST_HEAD;
      chunk_plus_offset(p, psize+SIZE_T_SIZE)->head = 0;

      if (mm < m->least_addr)
        m->least_addr = mm;
      if((m->footprint += mmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      assert(is_aligned(chunk2mem(p)));
      check_mmapped_chunk(m, p);
      return chunk2mem(p);
    }
  }
  return 0;
}

static mchunkptr mmap_resize(struct malloc_state* m, mchunkptr oldp, size_t nb)
{
  size_t oldsize = chunksize(oldp);
  if (is_small(nb))
    return 0;
  if (oldsize >= nb + SIZE_T_SIZE && (oldsize - nb) <= (mparams.granularity << 1))
    return oldp;
  else {
    size_t offset = oldp->prev_foot & ~IS_MMAPPED_BIT;
    size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
    size_t newmmsize = granularity_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
    char* cp = (char*)CALL_MREMAP((char*)oldp - offset, oldmmsize, newmmsize, 1);
    if (cp != CMFAIL)
    {
      mchunkptr newp = (mchunkptr)(cp + offset);
      size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
      newp->head = (psize|CINUSE_BIT);
      mark_inuse_foot(m, newp, psize);
      chunk_plus_offset(newp, psize)->head = FENCEPOST_HEAD;
      chunk_plus_offset(newp, psize+SIZE_T_SIZE)->head = 0;

      if (cp < m->least_addr)
        m->least_addr = cp;
      if((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      check_mmapped_chunk(m, newp);
      return newp;
    }
  }
  return 0;
}

static void init_top(struct malloc_state* m, mchunkptr p, size_t psize)
{
  size_t offset = align_offset(chunk2mem(p));
  p = (mchunkptr)((char*)p + offset);
  psize -= offset;

  m->top = p;
  m->topsize = psize;
  p->head = psize | PINUSE_BIT;
  chunk_plus_offset(p, psize)->head = TOP_FOOT_SIZE;
  m->trim_check = mparams.trim_threshold;
}

static void init_bins(struct malloc_state* m)
{
  size_t i;
  for (i = 0; i < NSMALLBINS; ++i)
  {
    sbinptr bin = smallbin_at(m,i);
    bin -> fd = bin -> bk = bin;
  }
}

#if PROCEED_ON_ERROR

static void reset_on_error(struct malloc_state* m)
{
  int i;
  ++malloc_corruption_error_count;
  m->smallbins = m -> treebins = 0;
  m -> dvsize = m -> topsize = 0;
  m -> seg.base = 0;
  m -> seg.size = 0;
  m -> seg.next = 0;
  m -> top = m -> dv = 0;
  for (i = 0; i < NTREEBINS; ++i)
    *treebin_at(m, i) = 0;
  init_bins(m);
}
#endif

static void* prepend_alloc(struct malloc_state* m, char* newbase, char* oldbase, size_t nb)
{
  mchunkptr p = align_as_chunk(newbase);
  mchunkptr oldfirst = align_as_chunk(oldbase);
  size_t psize = (char*)oldfirst - (char*)p;
  mchunkptr q = chunk_plus_offset(p, nb);
  size_t qsize = psize - nb;
  set_size_and_pinuse_of_inuse_chunk(m, p, nb);

  assert((char*)oldfirst > (char*)q);
  assert(pinuse(oldfirst));
  assert(qsize >= MIN_CHUNK_SIZE);

  if (oldfirst == m -> top)
  {
    size_t tsize = m -> topsize += qsize;
    m -> top = q;
    q -> head = tsize | PINUSE_BIT;
    check_top_chunk(m, q);
  }
  else if (oldfirst == m -> dv)
  {
    size_t dsize = m -> dvsize += qsize;
    m -> dv = q;
    set_size_and_pinuse_of_free_chunk(q, dsize);
  }
  else
  {
    if(!cinuse(oldfirst))
    {
      size_t nsize = chunksize(oldfirst);
      unlink_chunk(m, oldfirst, nsize);
      oldfirst = chunk_plus_offset(oldfirst, nsize);
      qsize += nsize;
    }
    set_free_with_pinuse(q, qsize, oldfirst);
    insert_chunk(m, q, qsize);
    check_free_chunk(m, q);
  }

  check_malloced_chunk(m, chunk2mem(p), nb);
  return chunk2mem(p);
}


static void add_segment(struct malloc_state* m, char* tbase, size_t tsize, size_t mmapped)
{
  char* old_top = (char*)m -> top;
  msegmentptr oldsp = segment_holding(m, old_top);
  char* old_end = oldsp -> base + oldsp -> size;
  size_t ssize = pad_request(sizeof(struct malloc_segment));
  char* rawsp = old_end - (ssize + FOUR_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
  size_t offset = align_offset(chunk2mem(rawsp));
  char* asp = rawsp + offset;
  char* csp = (asp < (old_top + MIN_CHUNK_SIZE))? old_top : asp;
  mchunkptr sp = (mchunkptr)csp;
  msegmentptr ss = (msegmentptr)(chunk2mem(sp));
  mchunkptr tnext = chunk_plus_offset(sp, ssize);
  mchunkptr p = tnext;
  int nfences = 0;

  init_top(m, (mchunkptr)tbase, tsize - TOP_FOOT_SIZE);

  assert(is_aligned(ss));
  set_size_and_pinuse_of_inuse_chunk(m, sp, ssize);
  *ss = m -> seg;
  m -> seg.base = tbase;
  m -> seg.size = tsize;
  m -> seg.sflags = mmapped;
  m -> seg.next = ss;

  for(;;)
  {
    mchunkptr nextp = chunk_plus_offset(p, SIZE_T_SIZE);
    p -> head = FENCEPOST_HEAD;
    ++nfences;
    if((char*)(&(nextp -> head)) < old_end)
      p = nextp;
    else
      break;
  }
  assert(nfences >= 2);

  if (csp != old_top)
  {
    mchunkptr q = (mchunkptr)old_top;
    size_t psize = csp - old_top;
    mchunkptr tn = chunk_plus_offset(q, psize);
    set_free_with_pinuse(q, psize, tn);
    insert_chunk(m, q, psize);
  }

  check_top_chunk(m, m -> top);
}

static void* sys_alloc(struct malloc_state* m, size_t nb)
{
  char* tbase = CMFAIL;
  size_t tsize = 0;
  size_t mmap_flag = 0;

  init_mparams();

  if (use_mmap(m) && nb >= mparams.mmap_threshold)
  {
    void* mem = mmap_alloc(m, nb);
    if (mem != 0)
      return mem;
  }

  if (MORECORE_CONTIGUOUS && !use_noncontiguous(m))
  {
    char* br = CMFAIL;
    msegmentptr ss = (m -> top == 0)? 0 : segment_holding(m, (char*)m -> top);
    size_t asize = 0;
    ACQUIRE_MORECORE_LOCK();

    if (ss == 0)
    {
      char* base = (char*)CALL_MORECORE(0);
      if (base != CMFAIL)
      {
        asize = granularity_align(nb + TOP_FOOT_SIZE + SIZE_T_ONE);
        if(!is_page_aligned(base))
          asize += (page_align((size_t)base) - (size_t)base);
        if (asize < HALF_MAX_SIZE_T && (br = (char*)(CALL_MORECORE(asize))) == base)
        {
          tbase = base;
          tsize = asize;
        }
      }
    }
    else
    {
      asize = granularity_align(nb - m -> topsize + TOP_FOOT_SIZE + SIZE_T_ONE);
      if (asize < HALF_MAX_SIZE_T && (br = (char*)(CALL_MORECORE(asize))) == ss -> base+ss -> size)
      {
        tbase = br;
        tsize = asize;
      }
    }

    if (tbase == CMFAIL)
    {
      if (br != CMFAIL)
      {
        if (asize < HALF_MAX_SIZE_T && asize < nb + TOP_FOOT_SIZE + SIZE_T_ONE)
        {
          size_t esize = granularity_align(nb + TOP_FOOT_SIZE + SIZE_T_ONE - asize);
          if (esize < HALF_MAX_SIZE_T)
          {
            char* end = (char*)CALL_MORECORE(esize);
            if (end != CMFAIL)
              asize += esize;
            else
            {
              CALL_MORECORE(-asize);
              br = CMFAIL;
            }
          }
        }
      }
      if(br != CMFAIL)
      {
        tbase = br;
        tsize = asize;
      }
      else
        disable_contiguous(m);
    }

    RELEASE_MORECORE_LOCK();
  }

  if (HAVE_MMAP && tbase == CMFAIL)
  {
    size_t req = nb + TOP_FOOT_SIZE + SIZE_T_ONE;
    size_t rsize = granularity_align(req);
    if (rsize > nb)
    {
      char* mp = (char*)(CALL_MMAP(rsize));
      if (mp != CMFAIL)
      {
        tbase = mp;
        tsize = rsize;
        mmap_flag = IS_MMAPPED_BIT;
      }
    }
  }

  if (HAVE_MORECORE && tbase == CMFAIL)
  {
    size_t asize = granularity_align(nb + TOP_FOOT_SIZE + SIZE_T_ONE);
    if (asize < HALF_MAX_SIZE_T)
    {
      char* br = CMFAIL;
      char* end = CMFAIL;
      ACQUIRE_MORECORE_LOCK();
      br = (char*)(CALL_MORECORE(asize));
      end = (char*)(CALL_MORECORE(0));
      RELEASE_MORECORE_LOCK();
      if (br != CMFAIL && end != CMFAIL && br < end)
      {
        size_t ssize = end - br;
        if (ssize > nb + TOP_FOOT_SIZE)
        {
          tbase = br;
          tsize = ssize;
        }
      }
    }
  }

  if (tbase != CMFAIL)
  {

    if((m -> footprint += tsize) > m -> max_footprint)
      m -> max_footprint = m -> footprint;

    if(!_is_initialized(m))
    {
      m -> seg.base = m -> least_addr = tbase;
      m -> seg.size = tsize;
      m -> seg.sflags = mmap_flag;
      m -> magic = mparams.magic;
      init_bins(m);
      if (is_global(m))
        init_top(m, (mchunkptr)tbase, tsize - TOP_FOOT_SIZE);
      else
      {
        mchunkptr mn = next_chunk(mem2chunk(m));
        init_top(m, mn, (size_t)((tbase + tsize) - (char*)mn) -TOP_FOOT_SIZE);
      }
    }

    else
    {
      msegmentptr sp = &m -> seg;
      while (sp != 0 && tbase != sp -> base + sp -> size)
        sp = sp -> next;
      if(sp != 0 && !is_extern_segment(sp) &&
        (sp -> sflags & IS_MMAPPED_BIT) == mmap_flag &&
          segment_holds(sp, m -> top))
      {
        sp -> size += tsize;
        init_top(m, m -> top, m -> topsize + tsize);
      }
      else
      {
        if (tbase < m -> least_addr)
          m -> least_addr = tbase;
        sp = &m -> seg;
        while (sp != 0 && sp -> base != tbase + tsize)
          sp = sp -> next;
        if (sp != 0 && !is_extern_segment(sp) && (sp -> sflags & IS_MMAPPED_BIT) == mmap_flag)
        {
          char* oldbase = sp -> base;
          sp -> base = tbase;
          sp -> size += tsize;
          return prepend_alloc(m, tbase, oldbase, nb);
        }
        else
          add_segment(m, tbase, tsize, mmap_flag);
      }
    }

    if (nb < m -> topsize)
    {
      size_t rsize = m -> topsize -= nb;
      mchunkptr p = m -> top;
      mchunkptr r = m -> top = chunk_plus_offset(p, nb);
      r -> head = rsize | PINUSE_BIT;
      set_size_and_pinuse_of_inuse_chunk(m, p, nb);
      check_top_chunk(m, m -> top);
      check_malloced_chunk(m, chunk2mem(p), nb);
      return chunk2mem(p);
    }
  }

  MALLOC_FAILURE_ACTION;
  return 0;
}


static size_t release_unused_segments(struct malloc_state* m)
{
  size_t released = 0;
  msegmentptr pred = &m -> seg;
  msegmentptr sp = pred -> next;
  while (sp != 0)
  {
    char* base = sp -> base;
    size_t size = sp -> size;
    msegmentptr next = sp -> next;
    if (is_mmapped_segment(sp) && !is_extern_segment(sp))
    {
      mchunkptr p = align_as_chunk(base);
      size_t psize = chunksize(p);
      if(!cinuse(p) && (char*)p + psize >= base + size - TOP_FOOT_SIZE)
      {
        tchunkptr tp = (tchunkptr)p;
        assert(segment_holds(sp, (char*)sp));
        if (p == m -> dv)
        {
          m -> dv = 0;
          m -> dvsize = 0;
        }
        else
          unlink_large_chunk(m, tp);
        if (CALL_MUNMAP(base, size) == 0)
        {
          released += size;
          m -> footprint -= size;
          sp = pred;
          sp -> next = next;
        }
        else
          insert_large_chunk(m, tp, psize);
      }
    }
    pred = sp;
    sp = next;
  }
  return released;
}

static int sys_trim(struct malloc_state* m, size_t pad)
{
  size_t released = 0;
  if (pad < MAX_REQUEST && _is_initialized(m))
  {
    pad += TOP_FOOT_SIZE;

    if (m -> topsize > pad)
    {
      size_t unit = mparams.granularity;
      size_t extra = ((m -> topsize - pad + (unit - SIZE_T_ONE)) / unit - SIZE_T_ONE) * unit;
      msegmentptr sp = segment_holding(m, (char*)m -> top);

      if(!is_extern_segment(sp))
      {
        if (is_mmapped_segment(sp))
        {
          if (HAVE_MMAP && sp -> size >= extra && !has_segment_link(m, sp))
          {
            size_t newsize = sp -> size - extra;
            if((CALL_MREMAP(sp -> base, sp -> size, newsize, 0) != MFAIL) ||
                (CALL_MUNMAP(sp -> base + newsize, extra) == 0))
              released = extra;
          }
        }
        else if (HAVE_MORECORE)
        {
          if (extra >= HALF_MAX_SIZE_T)
            extra = (HALF_MAX_SIZE_T) + SIZE_T_ONE - unit;
          ACQUIRE_MORECORE_LOCK();
          {
            char* old_br = (char*)(CALL_MORECORE(0));
            if (old_br == sp -> base + sp -> size)
            {
              char* rel_br = (char*)(CALL_MORECORE(-extra));
              char* new_br = (char*)(CALL_MORECORE(0));
              if (rel_br != CMFAIL && new_br < old_br)
                released = old_br - new_br;
            }
          }
          RELEASE_MORECORE_LOCK();
        }
      }

      if (released != 0)
      {
        sp -> size -= released;
        m -> footprint -= released;
        init_top(m, m -> top, m -> topsize - released);
        check_top_chunk(m, m -> top);
      }
    }

    if (HAVE_MMAP)
      released += release_unused_segments(m);

    if (released == 0)
      m -> trim_check = MAX_SIZE_T;
  }

  return (released != 0)? 1 : 0;
}

static void* tmalloc_large(struct malloc_state* m, size_t nb)
{
  tchunkptr v = 0;
  size_t rsize = -nb;
  tchunkptr t;
  size_t idx;
  compute_tree_index(nb, idx);

  if((t = *treebin_at(m, idx)) != 0)
  {
    size_t sizebits = nb << leftshift_for_tree_index(idx);
    tchunkptr rst = 0;
    for(;;)
    {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      if (trem < rsize)
      {
        v = t;
        if((rsize = trem) == 0)
          break;
      }
      rt = t -> child[1];
      t = t -> child[(sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1];
      if (rt != 0 && rt != t)
        rst = rt;
      if (t == 0)
      {
        t = rst;
        break;
      }
      sizebits <<= 1;
    }
  }

  if (t == 0 && v == 0)
  {
    size_t leftbits = left_bits(idx2bit(idx)) & m -> treemap;
    if (leftbits != 0)
    {
      size_t i;
      size_t leastbit = least_bit(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  while (t != 0)
  {
    size_t trem = chunksize(t) - nb;
    if (trem < rsize)
    {
      rsize = trem;
      v = t;
    }
    t = leftmost_child(t);
  }

  if (v != 0 && rsize < (size_t)(m -> dvsize - nb))
  {
    if(RTCHECK(ok_address(m, v)))
    {
      mchunkptr r = chunk_plus_offset(v, nb);
      assert(chunksize(v) == rsize + nb);
      if (RTCHECK(ok_next(v, r)))
      {
        unlink_large_chunk(m, v);
        if (rsize < MIN_CHUNK_SIZE)
          set_inuse_and_pinuse(m, v, (rsize + nb));
        else
        {
          set_size_and_pinuse_of_inuse_chunk(m, v, nb);
          set_size_and_pinuse_of_free_chunk(r, rsize);
          insert_chunk(m, r, rsize);
        }
        return chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  return 0;
}

static void* tmalloc_small(struct malloc_state* m, size_t nb)
{
  tchunkptr t, v;
  size_t rsize;
  size_t i;
  size_t leastbit = least_bit(m -> treemap);
  compute_bit2idx(leastbit, i);

  v = t = *treebin_at(m, i);
  rsize = chunksize(t) - nb;

  while((t = leftmost_child(t)) != 0)
  {
    size_t trem = chunksize(t) - nb;
    if (trem < rsize)
    {
      rsize = trem;
      v = t;
    }
  }

  if (RTCHECK(ok_address(m, v)))
  {
    mchunkptr r = chunk_plus_offset(v, nb);
    assert(chunksize(v) == rsize + nb);
    if (RTCHECK(ok_next(v, r)))
    {
      unlink_large_chunk(m, v);
      if (rsize < MIN_CHUNK_SIZE)
        set_inuse_and_pinuse(m, v, (rsize + nb));
      else
      {
        set_size_and_pinuse_of_inuse_chunk(m, v, nb);
        set_size_and_pinuse_of_free_chunk(r, rsize);
        replace_dv(m, r, rsize);
      }
      return chunk2mem(v);
    }
  }

  CORRUPTION_ERROR_ACTION(m);
  return 0;
}

static void* internal_realloc(struct malloc_state* m, void* oldmem, size_t bytes)
{
  if (bytes >= MAX_REQUEST)
  {
    MALLOC_FAILURE_ACTION;
    return 0;
  }
  if(!PREACTION(m))
  {
    mchunkptr oldp = mem2chunk(oldmem);
    size_t oldsize = chunksize(oldp);
    mchunkptr next = chunk_plus_offset(oldp, oldsize);
    mchunkptr newp = 0;
    void* extra = 0;

    if(RTCHECK(ok_address(m, oldp) && ok_cinuse(oldp) &&
                ok_next(oldp, next) && ok_pinuse(next)))
    {
      size_t nb = request2size(bytes);
      if (is_mmapped(oldp))
        newp = mmap_resize(m, oldp, nb);
      else if (oldsize >= nb)
      {
        size_t rsize = oldsize - nb;
        newp = oldp;
        if (rsize >= MIN_CHUNK_SIZE)
        {
          mchunkptr remainder = chunk_plus_offset(newp, nb);
          set_inuse(m, newp, nb);
          set_inuse(m, remainder, rsize);
          extra = chunk2mem(remainder);
        }
      }
      else if (next == m -> top && oldsize + m -> topsize > nb)
      {
        size_t newsize = oldsize + m -> topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_offset(oldp, nb);
        set_inuse(m, oldp, nb);
        newtop -> head = newtopsize |PINUSE_BIT;
        m -> top = newtop;
        m -> topsize = newtopsize;
        newp = oldp;
      }
    }
    else
    {
      USAGE_ERROR_ACTION(m, oldmem);
      POSTACTION(m);
      return 0;
    }

    POSTACTION(m);

    if (newp != 0)
    {
      if (extra != 0)
        internal_free(m, extra);
      check_inuse_chunk(m, newp);
      return chunk2mem(newp);
    }
    else
    {
      void* newmem = internal_malloc(m, bytes);
      if (newmem != 0)
      {
        size_t oc = oldsize - overhead_for(oldp);
        memcpy(newmem, oldmem, (oc < bytes)? oc : bytes);
        internal_free(m, oldmem);
      }
      return newmem;
    }
  }
  return 0;
}


static void* internal_memalign(struct malloc_state* m, size_t alignment, size_t bytes)
{
  if (alignment <= MALLOC_ALIGNMENT)
    return internal_malloc(m, bytes);
  if (alignment <  MIN_CHUNK_SIZE)
    alignment = MIN_CHUNK_SIZE;
  if((alignment & (alignment-SIZE_T_ONE)) != 0)
  {
    size_t a = MALLOC_ALIGNMENT << 1;
    while (a < alignment) a <<= 1;
    alignment = a;
  }

  if (bytes >= MAX_REQUEST - alignment)
  {
    if (m != 0)
      MALLOC_FAILURE_ACTION;
  }
  else
  {
    size_t nb = request2size(bytes);
    size_t req = nb + alignment + MIN_CHUNK_SIZE - CHUNK_OVERHEAD;
    char* mem = (char*)internal_malloc(m, req);
    if (mem != 0) {
      void* leader = 0;
      void* trailer = 0;
      mchunkptr p = mem2chunk(mem);

      if(PREACTION(m)) return 0;
      if((((size_t)(mem)) % alignment) != 0)
      {
        char* br = (char*)mem2chunk((size_t)(((size_t)(mem + alignment - SIZE_T_ONE)) & -alignment));
        char* pos = ((size_t)(br - (char*)(p)) >= MIN_CHUNK_SIZE)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (char*)(p);
        size_t newsize = chunksize(p) - leadsize;

        if (is_mmapped(p))
        {
          newp -> prev_foot = p -> prev_foot + leadsize;
          newp -> head = (newsize|CINUSE_BIT);
        }
        else
        {
          set_inuse(m, newp, newsize);
          set_inuse(m, p, leadsize);
          leader = chunk2mem(p);
        }
        p = newp;
      }

      if(!is_mmapped(p))
      {
        size_t size = chunksize(p);
        if (size > nb + MIN_CHUNK_SIZE)
        {
          size_t remainder_size = size - nb;
          mchunkptr remainder = chunk_plus_offset(p, nb);
          set_inuse(m, p, nb);
          set_inuse(m, remainder, remainder_size);
          trailer = chunk2mem(remainder);
        }
      }

      assert (chunksize(p) >= nb);
      assert((((size_t)(chunk2mem(p))) % alignment) == 0);
      check_inuse_chunk(m, p);
      POSTACTION(m);
      if (leader != 0)
        internal_free(m, leader);
      if (trailer != 0)
        internal_free(m, trailer);
      return chunk2mem(p);
    }
  }
  return 0;
}


static void** ialloc(struct malloc_state* m, size_t n_elements, size_t* sizes,  int opts, void* chunks[])
{
  size_t element_size;
  size_t contents_size;
  size_t array_size;
  void* mem;
  mchunkptr p;
  size_t remainder_size;
  void** marray;
  mchunkptr array_chunk;
  size_t was_enabled;
  size_t size;
  size_t i;

  if (chunks != 0)
  {
    if (n_elements == 0)
      return chunks;
    marray = chunks;
    array_size = 0;
  }
  else
  {
    if (n_elements == 0)
      return (void**)internal_malloc(m, 0);
    marray = 0;
    array_size = request2size(n_elements * (sizeof(void*)));
  }

  if (opts & 0x1)
  {
    element_size = request2size(*sizes);
    contents_size = n_elements * element_size;
  }
  else
  {
    element_size = 0;
    contents_size = 0;
    for (i = 0; i != n_elements; ++i)
      contents_size += request2size(sizes[i]);
  }

  size = contents_size + array_size;

  was_enabled = use_mmap(m);
  disable_mmap(m);
  mem = internal_malloc(m, size - CHUNK_OVERHEAD);
  if (was_enabled)
    enable_mmap(m);
  if (mem == 0)
    return 0;

  if (PREACTION(m)) return 0;
  p = mem2chunk(mem);
  remainder_size = chunksize(p);

  assert(!is_mmapped(p));

  if(opts & 0x2)
    memset((size_t*)mem, 0, remainder_size - SIZE_T_SIZE - array_size);

  if (marray == 0)
  {
    size_t  array_chunk_size;
    array_chunk = chunk_plus_offset(p, contents_size);
    array_chunk_size = remainder_size - contents_size;
    marray = (void**) (chunk2mem(array_chunk));
    set_size_and_pinuse_of_inuse_chunk(m, array_chunk, array_chunk_size);
    remainder_size = contents_size;
  }

  for (i = 0; ; ++i)
  {
    marray[i] = chunk2mem(p);
    if (i != n_elements-1)
    {
      if (element_size != 0)
        size = element_size;
      else
        size = request2size(sizes[i]);
      remainder_size -= size;
      set_size_and_pinuse_of_inuse_chunk(m, p, size);
      p = chunk_plus_offset(p, size);
    }
    else
    {
      set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
      break;
    }
  }

#if DEBUG
  if (marray != chunks)
  {
    if (element_size != 0)
      assert(remainder_size == element_size);
    else
      assert(remainder_size == request2size(sizes[i]));
    check_inuse_chunk(m, mem2chunk(marray));
  }
  for (i = 0; i != n_elements; ++i)
    check_inuse_chunk(m, mem2chunk(marray[i]));

#endif

  POSTACTION(m);
  return marray;
}


#if !ONLY_MSPACES

static void* hlmalloc(size_t bytes)
{
  return malloc(bytes);
  //boost::mutex::scoped_lock lock(mutex_);
  
  if(!PREACTION(gm))
  {
    void* mem;
    size_t nb;
    if (bytes <= MAX_SMALL_REQUEST)
    {
      size_t idx;
      size_t smallbits;
      nb = (bytes < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(bytes);
      idx = small_index(nb);
      smallbits = gm -> smallmap >> idx;

      if((smallbits & 0x3U) != 0)
      {
        mchunkptr b, p;
        idx += ~smallbits & 1;
        b = smallbin_at(gm, idx);
        p = b -> fd;
        assert(chunksize(p) == small_index2size(idx));
        unlink_first_small_chunk(gm, b, p, idx);
        set_inuse_and_pinuse(gm, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(gm, mem, nb);
        goto postaction;
      }

      else if (nb > gm -> dvsize)
      {
        if(smallbits != 0)
        {
          mchunkptr b, p, r;
          size_t rsize;
          size_t i;
          size_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
          size_t leastbit = least_bit(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(gm, i);
          p = b -> fd;
          assert(chunksize(p) == small_index2size(i));
          unlink_first_small_chunk(gm, b, p, i);
          rsize = small_index2size(i) - nb;
          if (SIZE_T_SIZE != 4 && rsize < MIN_CHUNK_SIZE)
            set_inuse_and_pinuse(gm, p, small_index2size(i));
          else
          {
            set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
            r = chunk_plus_offset(p, nb);
            set_size_and_pinuse_of_free_chunk(r, rsize);
            replace_dv(gm, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(gm, mem, nb);
          goto postaction;
        }

        else if (gm -> treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0)
        {
          check_malloced_chunk(gm, mem, nb);
          goto postaction;
        }
      }
    }
    else if (bytes >= MAX_REQUEST)
      nb = MAX_SIZE_T;
    else
    {
      nb = pad_request(bytes);
      if (gm -> treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0)
      {
        check_malloced_chunk(gm, mem, nb);
        goto postaction;
      }
    }

    if (nb <= gm -> dvsize)
    {
      size_t rsize = gm -> dvsize - nb;
      mchunkptr p = gm -> dv;
      if (rsize >= MIN_CHUNK_SIZE)
      {
        mchunkptr r = gm -> dv = chunk_plus_offset(p, nb);
        gm -> dvsize = rsize;
        set_size_and_pinuse_of_free_chunk(r, rsize);
        set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
      }
      else
      {
        size_t dvs = gm -> dvsize;
        gm -> dvsize = 0;
        gm -> dv = 0;
        set_inuse_and_pinuse(gm, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(gm, mem, nb);
      goto postaction;
    }

    else if (nb < gm -> topsize)
    {
      size_t rsize = gm -> topsize -= nb;
      mchunkptr p = gm -> top;
      mchunkptr r = gm -> top = chunk_plus_offset(p, nb);
      r -> head = rsize | PINUSE_BIT;
      set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
      mem = chunk2mem(p);
      check_top_chunk(gm, gm -> top);
      check_malloced_chunk(gm, mem, nb);
      goto postaction;
    }

    mem = sys_alloc(gm, nb);

  postaction:
    POSTACTION(gm);
    return mem;
  }

  return 0;
}

static void hlfree(void* mem)
{
  return free(mem);

  //boost::mutex::scoped_lock lock(mutex_);
  if (mem != 0)
  {
    mchunkptr p  = mem2chunk(mem);
#if FOOTERS
    struct malloc_state* fm = get_mstate_for(p);
    if(!ok_magic(fm))
    {
      USAGE_ERROR_ACTION(fm, p);
      return;
    }
#else
#define fm gm
#endif
    if(!PREACTION(fm))
    {
      check_inuse_chunk(fm, p);
      if (RTCHECK(ok_address(fm, p) && ok_cinuse(p)))
      {
        size_t psize = chunksize(p);
        mchunkptr next = chunk_plus_offset(p, psize);
        if(!pinuse(p))
        {
          size_t prevsize = p -> prev_foot;
          if((prevsize & IS_MMAPPED_BIT) != 0)
          {
            prevsize &= ~IS_MMAPPED_BIT;
            psize += prevsize + MMAP_FOOT_PAD;
            if (CALL_MUNMAP((char*)p - prevsize, psize) == 0)
              fm -> footprint -= psize;
            goto postaction;
          }
          else
          {
            mchunkptr prev = chunk_minus_offset(p, prevsize);
            psize += prevsize;
            p = prev;
            if (RTCHECK(ok_address(fm, prev)))
            {
              if (p != fm -> dv)
              {
                unlink_chunk(fm, p, prevsize);
              }
              else if((next -> head & INUSE_BITS) == INUSE_BITS)
              {
                fm -> dvsize = psize;
                set_free_with_pinuse(p, psize, next);
                goto postaction;
              }
            }
            else
              goto erroraction;
          }
        }

        if(RTCHECK(ok_next(p, next) && ok_pinuse(next)))
        {
          if(!cinuse(next))
          {
            if(next == fm -> top)
            {
              size_t tsize = fm -> topsize += psize;
              fm -> top = p;
              p -> head = tsize | PINUSE_BIT;
              if(p == fm -> dv)
              {
                fm -> dv = 0;
                fm -> dvsize = 0;
              }
              if(should_trim(fm, tsize))
                sys_trim(fm, 0);
              goto postaction;
            }
            else if (next == fm -> dv)
            {
              size_t dsize = fm -> dvsize += psize;
              fm -> dv = p;
              set_size_and_pinuse_of_free_chunk(p, dsize);
              goto postaction;
            }
            else
            {
              size_t nsize = chunksize(next);
              psize += nsize;
              unlink_chunk(fm, next, nsize);
              set_size_and_pinuse_of_free_chunk(p, psize);
              if (p == fm -> dv)
              {
                fm -> dvsize = psize;
                goto postaction;
              }
            }
          }
          else
            set_free_with_pinuse(p, psize, next);
          insert_chunk(fm, p, psize);
          check_free_chunk(fm, p);
          goto postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
#if !FOOTERS
#undef fm
#endif
}

static void* hlcalloc(size_t n_elements, size_t elem_size)
{
  void* mem;
  size_t req = 0;
  if (n_elements != 0)
  {
    req = n_elements * elem_size;
    if(((n_elements | elem_size) & ~(size_t)0xffff) && (req / n_elements != elem_size))
      req = MAX_SIZE_T;
  }
  mem = hlmalloc(req);
  if (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  return mem;
}

static void* hlrealloc(void* oldmem, size_t bytes)
{
  return realloc(oldmem, bytes);
  
  if (oldmem == 0)
    return hlmalloc(bytes);
#ifdef REALLOC_ZERO_BYTES_FREES
  if (bytes == 0) {
    hlfree(oldmem);
    return 0;
  }
#endif
  else
  {
    struct malloc_state* m ;
    {
      //boost::mutex::scoped_lock lock(mutex_);
#if !FOOTERS
    m = gm;
#else
    m = get_mstate_for(mem2chunk(oldmem));
    if(!ok_magic(m))
    {
      USAGE_ERROR_ACTION(m, oldmem);
      return 0;
    }
#endif
    }
    
    return internal_realloc(m, oldmem, bytes);
  }
}

static void* hlmemalign(size_t alignment, size_t bytes)
{
  return internal_memalign(gm, alignment, bytes);
}

static void** hlindependent_calloc(size_t n_elements, size_t elem_size, void* chunks[])
{
  size_t sz = elem_size;
  return ialloc(gm, n_elements, &sz, 3, chunks);
}

static void** hlindependent_comalloc(size_t n_elements, size_t sizes[], void* chunks[])
{
  return ialloc(gm, n_elements, sizes, 0, chunks);
}

static void* hlvalloc(size_t bytes)
{
  size_t pagesz;
  init_mparams();
  pagesz = mparams.page_size;
  return hlmemalign(pagesz, bytes);
}

static void* hlpvalloc(size_t bytes)
{
  size_t pagesz;
  init_mparams();
  pagesz = mparams.page_size;
  return hlmemalign(pagesz, (bytes + pagesz - SIZE_T_ONE) & ~(pagesz - SIZE_T_ONE));
}

static int hlmalloc_trim(size_t pad)
{
  int result = 0;
  if(!PREACTION(gm))
  {
    result = sys_trim(gm, pad);
    POSTACTION(gm);
  }
  return result;
}

static size_t hlmalloc_footprint(void) { return gm -> footprint; }

static size_t hlmalloc_max_footprint(void) { return gm -> max_footprint; }

#if !NO_MALLINFO
static struct mallinfo hlmallinfo(void) { return internal_mallinfo(gm); }
#endif

static void hlmalloc_stats() { internal_malloc_stats(gm); }

static size_t hlmalloc_usable_size(void* mem)
{
  if (mem != 0)
  {
    mchunkptr p = mem2chunk(mem);
    if (cinuse(p))
      return chunksize(p) - overhead_for(p);
  }
  return 0;
}

static int hlmallopt(int param_number, int value) {
  return change_mparam(param_number, value);
}

#endif


#if MSPACES

static struct malloc_state* init_user_mstate(char* tbase, size_t tsize)
{
  size_t msize = pad_request(sizeof(struct malloc_state));
  mchunkptr mn;
  mchunkptr msp = align_as_chunk(tbase);
  struct malloc_state* m = (struct malloc_state*)(chunk2mem(msp));
  memset(m, 0, msize);
  INITIAL_LOCK(&m -> mutex);
  msp -> head = (msize|PINUSE_BIT|CINUSE_BIT);
  m -> seg.base = m -> least_addr = tbase;
  m -> seg.size = m -> footprint = m -> max_footprint = tsize;
  m -> magic = mparams.magic;
  m -> mflags = mparams.default_mflags;
  disable_contiguous(m);
  init_bins(m);
  mn = next_chunk(mem2chunk(m));
  init_top(m, mn, (size_t)((tbase + tsize) - (char*)mn) - TOP_FOOT_SIZE);
  check_top_chunk(m, m -> top);
  return m;
}

static void* create_mspace(size_t capacity, int locked)
{
  struct malloc_state* m = 0;
  size_t msize = pad_request(sizeof(struct malloc_state));
  init_mparams();

  if (capacity < (size_t) -(msize + TOP_FOOT_SIZE + mparams.page_size))
  {
    size_t rs = ((capacity == 0)? mparams.granularity : (capacity + TOP_FOOT_SIZE + msize));
    size_t tsize = granularity_align(rs);
    char* tbase = (char*)(CALL_MMAP(tsize));
    if (tbase != CMFAIL)
    {
      m = init_user_mstate(tbase, tsize);
      m -> seg.sflags = IS_MMAPPED_BIT;
      set_lock(m, locked);
    }
  }
  return (void*)m;
}

static void* create_mspace_with_base(void* base, size_t capacity, int locked)
{
  struct malloc_state* m = 0;
  size_t msize = pad_request(sizeof(struct malloc_state));
  init_mparams();

  if(capacity > msize + TOP_FOOT_SIZE &&
      capacity < (size_t) -(msize + TOP_FOOT_SIZE + mparams.page_size))
  {
    m = init_user_mstate((char*)base, capacity);
    m -> seg.sflags = EXTERN_BIT;
    set_lock(m, locked);
  }
  return (void*)m;
}

static size_t destroy_mspace(void* msp)
{
  size_t freed = 0;
  struct malloc_state* ms = (struct malloc_state*)msp;
  if (ok_magic(ms))
  {
    msegmentptr sp = &ms -> seg;
    while (sp != 0)
    {
      char* base = sp -> base;
      size_t size = sp -> size;
      size_t flag = sp -> sflags;
      sp = sp -> next;
      if((flag & IS_MMAPPED_BIT) && !(flag & EXTERN_BIT) &&
          CALL_MUNMAP(base, size) == 0)
        freed += size;
    }
  }
  else
    USAGE_ERROR_ACTION(ms,ms);
  return freed;
}

static void* mspace_malloc(void* msp, size_t bytes)
{
  struct malloc_state* ms = (struct malloc_state*)msp;
  if(!ok_magic(ms))
  {
    USAGE_ERROR_ACTION(ms,ms);
    return 0;
  }
  if(!PREACTION(ms))
  {
    void* mem;
    size_t nb;
    if (bytes <= MAX_SMALL_REQUEST)
    {
      size_t idx;
      size_t smallbits;
      nb = (bytes < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(bytes);
      idx = small_index(nb);
      smallbits = ms -> smallmap >> idx;

      if((smallbits & 0x3U) != 0)
      {
        mchunkptr b, p;
        idx += ~smallbits & 1;
        b = smallbin_at(ms, idx);
        p = b -> fd;
        assert(chunksize(p) == small_index2size(idx));
        unlink_first_small_chunk(ms, b, p, idx);
        set_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        goto postaction;
      }

      else if (nb > ms -> dvsize)
      {
        if (smallbits != 0)
        {
          mchunkptr b, p, r;
          size_t rsize;
          size_t i;
          size_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
          size_t leastbit = least_bit(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b -> fd;
          assert(chunksize(p) == small_index2size(i));
          unlink_first_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          if (SIZE_T_SIZE != 4 && rsize < MIN_CHUNK_SIZE)
            set_inuse_and_pinuse(ms, p, small_index2size(i));
          else
          {
            set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
            r = chunk_plus_offset(p, nb);
            set_size_and_pinuse_of_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          goto postaction;
        }

        else if (ms -> treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0)
        {
          check_malloced_chunk(ms, mem, nb);
          goto postaction;
        }
      }
    }
    else if (bytes >= MAX_REQUEST)
      nb = MAX_SIZE_T;
    else
    {
      nb = pad_request(bytes);
      if (ms -> treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0)
      {
        check_malloced_chunk(ms, mem, nb);
        goto postaction;
      }
    }

    if (nb <= ms -> dvsize)
    {
      size_t rsize = ms -> dvsize - nb;
      mchunkptr p = ms -> dv;
      if (rsize >= MIN_CHUNK_SIZE)
      {
        mchunkptr r = ms -> dv = chunk_plus_offset(p, nb);
        ms -> dvsize = rsize;
        set_size_and_pinuse_of_free_chunk(r, rsize);
        set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
      }
      else
      {
        size_t dvs = ms -> dvsize;
        ms -> dvsize = 0;
        ms -> dv = 0;
        set_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      goto postaction;
    }

    else if (nb < ms -> topsize)
    {
      size_t rsize = ms -> topsize -= nb;
      mchunkptr p = ms -> top;
      mchunkptr r = ms -> top = chunk_plus_offset(p, nb);
      r -> head = rsize | PINUSE_BIT;
      set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_top_chunk(ms, ms -> top);
      check_malloced_chunk(ms, mem, nb);
      goto postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
    POSTACTION(ms);
    return mem;
  }

  return 0;
}

static void mspace_free(void* msp, void* mem)
{
  if (mem != 0)
  {
    mchunkptr p  = mem2chunk(mem);
#if FOOTERS
    struct malloc_state* fm = get_mstate_for(p);
#else
    struct malloc_state* fm = (struct malloc_state*)msp;
#endif
    if(!ok_magic(fm))
    {
      USAGE_ERROR_ACTION(fm, p);
      return;
    }
    if(!PREACTION(fm))
    {
      check_inuse_chunk(fm, p);
      if (RTCHECK(ok_address(fm, p) && ok_cinuse(p)))
      {
        size_t psize = chunksize(p);
        mchunkptr next = chunk_plus_offset(p, psize);
        if(!pinuse(p))
        {
          size_t prevsize = p -> prev_foot;
          if((prevsize & IS_MMAPPED_BIT) != 0)
          {
            prevsize &= ~IS_MMAPPED_BIT;
            psize += prevsize + MMAP_FOOT_PAD;
            if (CALL_MUNMAP((char*)p - prevsize, psize) == 0)
              fm -> footprint -= psize;
            goto postaction;
          }
          else
          {
            mchunkptr prev = chunk_minus_offset(p, prevsize);
            psize += prevsize;
            p = prev;
            if (RTCHECK(ok_address(fm, prev)))
            {
              if (p != fm -> dv)
                unlink_chunk(fm, p, prevsize);
              else if((next -> head & INUSE_BITS) == INUSE_BITS)
              {
                fm -> dvsize = psize;
                set_free_with_pinuse(p, psize, next);
                goto postaction;
              }
            }
            else
              goto erroraction;
          }
        }

        if (RTCHECK(ok_next(p, next) && ok_pinuse(next)))
        {
          if(!cinuse(next))
          {
            if (next == fm -> top)
            {
              size_t tsize = fm -> topsize += psize;
              fm -> top = p;
              p -> head = tsize | PINUSE_BIT;
              if (p == fm -> dv)
              {
                fm -> dv = 0;
                fm -> dvsize = 0;
              }
              if (should_trim(fm, tsize))
                sys_trim(fm, 0);
              goto postaction;
            }
            else if (next == fm -> dv)
            {
              size_t dsize = fm -> dvsize += psize;
              fm -> dv = p;
              set_size_and_pinuse_of_free_chunk(p, dsize);
              goto postaction;
            }
            else
            {
              size_t nsize = chunksize(next);
              psize += nsize;
              unlink_chunk(fm, next, nsize);
              set_size_and_pinuse_of_free_chunk(p, psize);
              if (p == fm -> dv) {
                fm -> dvsize = psize;
                goto postaction;
              }
            }
          }
          else
            set_free_with_pinuse(p, psize, next);
          insert_chunk(fm, p, psize);
          check_free_chunk(fm, p);
          goto postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
}

static void* mspace_calloc(void* msp, size_t n_elements, size_t elem_size)
{
  void* mem;
  size_t req = 0;
  struct malloc_state* ms = (struct malloc_state*)msp;
  if(!ok_magic(ms))
  {
    USAGE_ERROR_ACTION(ms,ms);
    return 0;
  }
  if (n_elements != 0)
  {
    req = n_elements * elem_size;
    if(((n_elements | elem_size) & ~(size_t)0xffff) && (req / n_elements != elem_size))
      req = MAX_SIZE_T;
  }
  mem = internal_malloc(ms, req);
  if (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  return mem;
}

static void* mspace_realloc(void* msp, void* oldmem, size_t bytes)
{
  if (oldmem == 0)
    return mspace_malloc(msp, bytes);
#ifdef REALLOC_ZERO_BYTES_FREES
  if (bytes == 0)
  {
    mspace_free(msp, oldmem);
    return 0;
  }
#endif
  else
  {
#if FOOTERS
    mchunkptr p  = mem2chunk(oldmem);
    struct malloc_state* ms = get_mstate_for(p);
#else
    struct malloc_state* ms = (struct malloc_state*)msp;
#endif /* FOOTERS */
    if(!ok_magic(ms))
    {
      USAGE_ERROR_ACTION(ms,ms);
      return 0;
    }
    return internal_realloc(ms, oldmem, bytes);
  }
}

static void* mspace_memalign(void* msp, size_t alignment, size_t bytes)
{
  struct malloc_state* ms = (struct malloc_state*)msp;
  if(!ok_magic(ms))
  {
    USAGE_ERROR_ACTION(ms,ms);
    return 0;
  }
  return internal_memalign(ms, alignment, bytes);
}

static void** mspace_independent_calloc(void* msp, size_t n_elements, size_t elem_size, void* chunks[])
{
  size_t sz = elem_size;
  struct malloc_state* ms = (struct malloc_state*)msp;
  if(!ok_magic(ms))
  {
    USAGE_ERROR_ACTION(ms,ms);
    return 0;
  }
  return ialloc(ms, n_elements, &sz, 3, chunks);
}

static void** mspace_independent_comalloc(void* msp, size_t n_elements, size_t sizes[], void* chunks[])
{
  struct malloc_state* ms = (struct malloc_state*)msp;
  if(!ok_magic(ms))
  {
    USAGE_ERROR_ACTION(ms,ms);
    return 0;
  }
  return ialloc(ms, n_elements, sizes, 0, chunks);
}

static int mspace_trim(void* msp, size_t pad)
{
  int result = 0;
  struct malloc_state* ms = (struct malloc_state*)msp;
  if (ok_magic(ms))
  {
    if(!PREACTION(ms))
    {
      result = sys_trim(ms, pad);
      POSTACTION(ms);
    }
  }
  else
    USAGE_ERROR_ACTION(ms,ms);
  return result;
}

static void mspace_malloc_stats(void* msp)
{
  struct malloc_state* ms = (struct malloc_state*)msp;
  if (ok_magic(ms))
    internal_malloc_stats(ms);
  else
    USAGE_ERROR_ACTION(ms,ms);
}

static size_t mspace_footprint(void* msp)
{
  size_t result;
  struct malloc_state* ms = (struct malloc_state*)msp;
  if (ok_magic(ms))
    result = ms -> footprint;
  USAGE_ERROR_ACTION(ms,ms);
  return result;
}


static size_t mspace_max_footprint(void* msp)
{
  size_t result;
  struct malloc_state* ms = (struct malloc_state*)msp;
  if (ok_magic(ms))
    result = ms -> max_footprint;
  USAGE_ERROR_ACTION(ms,ms);
  return result;
}


#if !NO_MALLINFO
static struct mallinfo mspace_mallinfo(void* msp)
{
  struct malloc_state* ms = (struct malloc_state*)msp;
  if(!ok_magic(ms))
    USAGE_ERROR_ACTION(ms,ms);
  return internal_mallinfo(ms);
}
#endif

static int mspace_mallopt(int param_number, int value)
{
  return change_mparam(param_number, value);
}

#endif
}
;

//struct HLmemory::malloc_params HLmemory::mparams = HLmemory::malloc_params();

//struct HLmemory::malloc_state HLmemory::_gm_ = HLmemory::malloc_state();

#endif
