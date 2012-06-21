/* Copyright (C) 2007, 2012 Ulrich Drepper.
   This file is part of libNUMA.

   libNUMA is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 2 of the License.

   libNUMA is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.  */

#ifndef _LIBNUMA_H
#define _LIBNUMA_H	1

#include <sched.h>
#include <stdlib.h>


/* Size definition for memory node sets.  */
#define __MEMNODE_SETSIZE	128
#define __NMEMNODEBITS		(8 * sizeof (__memnode_mask))

/* Type for array elements in 'memnode_set_t'.  */
typedef unsigned long int __memnode_mask;

/* Basic access functions.  */
#define __MEMNODEELT(node)  ((node) / __NMEMNODEBITS)
#define __MEMNODEMASK(node) ((__memnode_mask) 1 << ((node) % __NMEMNODEBITS))

/* Data structure to describe memory node mask.  */
typedef struct
{
  __memnode_mask __bits[__MEMNODE_SETSIZE / __NMEMNODEBITS];
} memnode_set_t;


/* Access functions for memory node masks.  */
#if __GNUC_PREREQ (2, 91)
# define MEMNODE_ZERO_S(setsize, memnodesetp) \
  do __builtin_memset (memnodesetp, '\0', setsize); while (0)
#else
# define MEMNODE_ZERO_S(setsize, memnodesetp) \
  do {									      \
    size_t __i;								      \
    size_t __imax = (setsize) / sizeof (__memnode_mask);		      \
    memnode_set_t *__arr = (cpusetp);					      \
    for (__i = 0; __i < __imax; ++__i)					      \
      __arr->__bits[__i] = 0;						      \
  } while (0)
#endif
#define MEMNODE_SET_S(node, setsize, memnodesetp) \
  ({ size_t __node = (node);						      \
     __node < 8 * (setsize)						      \
     ? ((memnodesetp)->__bits[__MEMNODEELT (__node)]			      \
	|= __MEMNODEMASK (__node)) : 0; })
#define MEMNODE_CLR_S(node, setsize, memnodesetp) \
  ({ size_t __node = (node);						      \
     __node < 8 * (setsize)						      \
     ? ((memnodesetp)->__bits[__MEMNODEELT (__node)]			      \
	&= ~__MEMNODEMASK (__node)) : 0; })
#define MEMNODE_ISSET_S(node, setsize, memnodesetp) \
  ({ size_t __node = (node);						      \
     __node < 8 * (setsize)						      \
     ? (((memnodesetp)->__bits[__MEMNODEELT (__node)]			      \
	 & __MEMNODEMASK (__node))) != 0 : 0; })

#define MEMNODE_COUNT_S(setsize, memnodesetp) \
  NUMA_memnode_count (setsize, memnodesetp)

#if __GNUC_PREREQ (2, 91)
#  define MEMNODE_EQUAL_S(setsize, memnodesetp1, memnodesetp2) \
  (__builtin_memcmp (memnodesetp1, memnodesetp2, setsize) == 0)
#else
# define MEMNODE_EQUAL_S(setsize, memnodesetp1, memnodesetp2) \
  ({ memnode_set_t *__arr1 = (memnodesetp1);				      \
     memnode_set_t *__arr2 = (memnodesetp2);				      \
     size_t __imax = (setsize) / sizeof (__memnode_mask);		      \
     size_t __i;							      \
     for (__i = 0; __i < __imax; ++__i)					      \
       if (__arr1->__bits[__i] != __arr2->__bits[__i])			      \
	 break;								      \
     __i == __imax; })
#endif

#define __MEMNODE_OP_S(setsize, destset, srcset1, srcset2, op) \
  ({ memnode_set_t *__dest = (destset);					      \
     memnode_set_t *__arr1 = (srcset1);					      \
     memnode_set_t *__arr2 = (srcset2);					      \
     size_t __imax = (setsize) / sizeof (__cpu_mask);			      \
     size_t __i;							      \
     for (__i = 0; __i < __imax; ++__i)					      \
       __dest->__bits[__i] = __arr1->__bits[__i] op __arr2->__bits[__i];      \
     __dest; })
#define MEMNODE_AND_S(setsize, destset, srcset1, srcset2) \
  __MEMNODE_OP_S (setsize, destset, srcset1, srcset2, &)
#define MEMNODE_OR_S(setsize, destset, srcset1, srcset2) \
  __MEMNODE_OP_S (setsize, destset, srcset1, srcset2, |)
#define MEMNODE_XOR_S(setsize, destset, srcset1, srcset2) \
  __MEMNODE_OP_S (setsize, destset, srcset1, srcset2, ^)

#define MEMNODE_ALLOC_SIZE(count) \
  ((((count) + __NMEMNODEBITS - 1) / __NMEMNODEBITS) * 8)
#define MEMNODE_ALLOC(count) \
  ((memnode_set_t *) malloc (MEMNODE_ALLOC_SIZE (count)))
#define MEMNODE_FREE(memnodeset) \
  free (memnodeset)


__BEGIN_DECLS

/* Return current number of online CPUs in the system.  */
extern int NUMA_cpu_system_count (void) __THROW;
/* Set bits for all online CPUs in the system and return the number of
   bits set.  */
extern int NUMA_cpu_system_mask (size_t __destsize, cpu_set_t *__dest) __THROW;

/* Return current number of CPUs the calling thread is allowed to run on.  */
extern int NUMA_cpu_self_count (void) __THROW;
/* Set bits for all CPUs the calling thread is allowed to run on and
   return the number of bits set.  */
extern int NUMA_cpu_self_mask (size_t __destsize, cpu_set_t *__dest) __THROW;

/* Return index of CPUS currently used by the calling thread.  */
extern int NUMA_cpu_self_current_idx (void) __THROW;
/* Set bit for the CPU currently used by the calling thread.  */
extern int NUMA_cpu_self_current_mask (size_t __destsize, cpu_set_t *__dest)
  __THROW;

/* Return CPUs up to LEVEL from CPU in SRC.  */
extern ssize_t NUMA_cpu_level_mask (size_t __destsize, cpu_set_t *__dest,
				    size_t __srcsize, const cpu_set_t *__src,
				    unsigned int __level);


/* Return current number of online memory nodes in the system.  */
extern int NUMA_memnode_system_count (void) __THROW;
/* Set bits for all online memory nodes in the system and return the
   number of bits set.  */
extern int NUMA_memnode_system_mask (size_t __destsize, memnode_set_t *__dest)
  __THROW;

/* Set bits for all memory nodes locale to any CPU the calling thread
   is currently allowed to is return the number of bits set.  */
extern int NUMA_memnode_self_mask (size_t __destsize, memnode_set_t *__dest)
  __THROW;

/* Return index of memory node of currently used CPU.  */
extern int NUMA_memnode_self_current_idx (void) __THROW;
/* Set bit for memory node of currently used CPU.  */
extern int NUMA_memnode_self_current_mask (size_t __destsize,
					   memnode_set_t *__dest) __THROW;

/* Set bits for all memory nodes which are local to any of the CPUs
   indicated by bits set in CPUSET.  Return -1 on failure, otherwise
   the number of bits set in MEMNODESET.  */
extern int NUMA_cpu_to_memnode (size_t __cpusetsize, const cpu_set_t *__cpuset,
				size_t __memnodesize,
				memnode_set_t *__memnodeset) __THROW;
/* Set bits for all CPUs which are local to any of the memory nodes
   indicated by bits set in MEMNODESET.  Return -1 on failure, otherwise
   the number of bits set in CPUSET.  */
extern int NUMA_memnode_to_cpu (size_t __memnodesize,
				const memnode_set_t *__memnodeset,
				size_t __cpusetsize, cpu_set_t *__cpuset)
  __THROW;

/* Count the number of bits set for memory nodes in SET.  */
extern int NUMA_memnode_count (size_t __setsize, const memnode_set_t *__set)
  __THROW;

/* Return the index of the memory node which contains (or would
   contain) the memory page pointed to by ADDR.  Returns the index of
   the memory node or -1 in case of a failure.  */
extern int NUMA_mem_get_node_idx (void *__addr) __THROW;
/* In DEST set bits for all the memory nodes which contain (or would
   contain) any of the memory pages allocated for the address range
   [ADDR,ADDR+SIZE).  Returns the number of bits set in DEST or -1 in
   case of a failure.  */
extern int NUMA_mem_get_node_mask (void *__addr, size_t __size,
				   size_t __destsize, memnode_set_t *__dest)
  __THROW;


__END_DECLS

#endif	/* libNUMA.h */
