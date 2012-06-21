#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <numaif.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/param.h>

#include "libNUMA.h"


int
NUMA_memnode_system_mask (size_t destsize, memnode_set_t *dest)
{
  /* Iterate over the node information.  */
  DIR *dir = opendir ("/sys/devices/system/node");
  if (dir == NULL)
    return -1;

  MEMNODE_ZERO_S (destsize, dest);

  int result = 0;
  struct dirent64 *d;
  while ((d = readdir64 (dir)) != NULL)
    /* NB: the sysfs has d_type support.  */
    if (d->d_type == DT_DIR && strncmp (d->d_name, "node", 4) == 0)
      {
	char *endp;
	unsigned long int nodeidx = strtoul (d->d_name + 4, &endp, 10);
	if (nodeidx != ULONG_MAX && endp != d->d_name + 4 && *endp == '\0')
	  {
	    if (dest != NULL && MEMNODE_SET_S (nodeidx, destsize, dest) == 0)
	      {
		result = -1;
		errno = ERANGE;
		break;
	      }
	    ++result;
	  }
      }

  closedir (dir);

  return result;
}


int
NUMA_memnode_system_count (void)
{
  return NUMA_memnode_system_mask (0, NULL);
}


int
NUMA_memnode_self_mask (size_t destsize, memnode_set_t *dest)
{
  // XXX When the kernel knows about the special use of 0 use it instead.
  pid_t pid = getpid ();

 /* Try with the default cpu_set_t first.  */
  cpu_set_t s;
  if (sched_getaffinity (pid, sizeof (s), &s) < 0)
    {
      cpu_set_t *ds = NULL;
      size_t dslen = sizeof (s);
      int result = -1;

      while (errno == EINVAL)
	{
	  /* Don't worry about overflows here.  malloc will return
	     ENOMEM long before that.  */
	  dslen *= 2;
	  cpu_set_t *newds = realloc (ds, dslen);
	  if (newds == NULL)
	    {
	      free (ds);
	      return -1;
	    }
	  ds = newds;

	  if (sched_getaffinity (getpid (), dslen, ds) >= 0)
	    {
	      result = NUMA_cpu_to_memnode (dslen, ds, destsize, dest);
	      break;
	    }
	}

      free (ds);

      return result;
    }

  return NUMA_cpu_to_memnode (sizeof (s), &s, destsize, dest);
}


int
NUMA_memnode_self_current_idx (void)
{
  int node;

#ifdef __x86_64__
  /* getcpu is a vsyscall.  */
  register unsigned long int rax __asm__ ("rax") = 0xffffffffff600800ul;
  register long int rcx __asm ("rcx");
  register void *rdx __asm ("rdx") = NULL;
  register void *rdi __asm ("rdi") = NULL;
  register void *rsi __asm ("rsi") = &node;
  register long int r8 __asm ("r8");
  register long int r9 __asm ("r9");
  register long int r10 __asm ("r10");
  register long int r11 __asm ("r11");
  asm volatile ("call *%%rax"
		: "=a" (rax), "=c" (rcx), "=d" (rdx), "=D" (rdi), "=S" (rsi),
		  "=r" (r8), "=r" (r9), "=r" (r10), "=r" (r11)
		: "2" (rdx), "3" (rdi), "4" (rsi));
  if (rax != 0)
    {
      errno = -rax;
      return -1;
    }
#else
  if (syscall (__NR_getcpu, NULL, &node, NULL) < 0)
    return -1;
#endif

  return node;
}


int
NUMA_memnode_self_current_mask (size_t destsize, memnode_set_t *dest)
{
  int idx = NUMA_memnode_self_current_idx ();
  if (idx < 0)
    return -1;

  MEMNODE_SET_S (idx, destsize, dest);

  return idx;
}


int
NUMA_cpu_to_memnode (size_t cpusetsize, const cpu_set_t *cpuset,
		     size_t memnodesize, memnode_set_t *memnodeset)
{
  /* Iterate over the node information and check whether any of the
     local CPUs is listed in CPUSET.  */
  DIR *dir = opendir ("/sys/devices/system/node");
  if (dir == NULL)
    return -1;

  MEMNODE_ZERO_S (memnodesize, memnodeset);

  int result = 0;
  int dfd = dirfd (dir);
  struct dirent64 *d;
  while ((d = readdir64 (dir)) != NULL)
    /* NB: the sysfs has d_type support.  */
    if (d->d_type == DT_DIR && strncmp (d->d_name, "node", 4) == 0)
      {
	char *endp;
	unsigned long int nodeidx = strtoul (d->d_name + 4, &endp, 10);
	if (nodeidx == ULONG_MAX || endp == d->d_name + 4 || *endp != '\0')
	  continue;

	/* Now read this directory for the cpu* links.  */
	int fd = openat (dfd, d->d_name, O_RDONLY|O_DIRECTORY);
	if (fd == -1)
	  continue;

	DIR *dir2 = fdopendir (fd);
	if (dir2 == NULL)
	  {
	    close (fd);
	    continue;
	  }

	struct dirent64 *d2;
	while ((d2 = readdir64 (dir2)) != NULL)
	  if (d2->d_type == DT_LNK && strncmp (d2->d_name, "cpu", 3) == 0)
	    {
	      unsigned long int nr = strtoul (d2->d_name + 3, &endp, 10);
	      if (nr != ULONG_MAX && endp != d2->d_name + 3 && *endp == '\0'
		  && CPU_ISSET_S (nr, cpusetsize, cpuset))
		{
		  /* The CPU is in the set.  Mark the memory node as used.  */
		  if (MEMNODE_SET_S (nodeidx, memnodesize, memnodeset) == 0)
		    {
		      closedir (dir2);
		      errno = ERANGE;
		      result = 1;
		      goto out;
		    }

		  ++result;
		  break;
		}
	    }

	closedir (dir2);
      }

 out:
  closedir (dir);

  return result;
}


int
NUMA_memnode_to_cpu (size_t memnodesize, const memnode_set_t *memnodeset,
		     size_t cpusetsize, cpu_set_t *cpuset)
{
  /* Iterate over the node information and set the bits for all the
     CPUs in memory nodes listed in MEMNODESET.  */
  DIR *dir = opendir ("/sys/devices/system/node");
  if (dir == NULL)
    return -1;

  CPU_ZERO_S (cpusetsize, cpuset);

  int result = 0;
  int dfd = dirfd (dir);
  struct dirent64 *d;
  while ((d = readdir64 (dir)) != NULL)
    /* NB: the sysfs has d_type support.  */
    if (d->d_type == DT_DIR && strncmp (d->d_name, "node", 4) == 0)
      {
	char *endp;
	unsigned long int nodeidx = strtoul (d->d_name + 4, &endp, 10);
	if (nodeidx == ULONG_MAX || endp == d->d_name + 4 || *endp != '\0'
	    || ! MEMNODE_ISSET_S (nodeidx, memnodesize, memnodeset))
	  continue;

	/* Now read this directory for the cpu* links.  */
	int fd = openat (dfd, d->d_name, O_RDONLY|O_DIRECTORY);
	if (fd == -1)
	  continue;

	DIR *dir2 = fdopendir (fd);
	if (dir2 == NULL)
	  {
	    close (fd);
	    continue;
	  }

	struct dirent64 *d2;
	while ((d2 = readdir64 (dir2)) != NULL)
	  if (d2->d_type == DT_LNK && strncmp (d2->d_name, "cpu", 3) == 0)
	    {
	      unsigned long int nr = strtoul (d2->d_name + 3, &endp, 10);
	      if (nr != ULONG_MAX && endp != d2->d_name + 3 && *endp == '\0')
		{
		  /* Precaution: maybe a CPU is listed for more than
		     one node.  We do not want to count it twice.  */
		  if (! CPU_ISSET_S (nr, cpusetsize, cpuset)
		      && CPU_SET_S (nr, cpusetsize, cpuset) == 0)
		    {
		      closedir (dir2);
		      errno = ERANGE;
		      result = 1;
		      goto out;
		    }

		  ++result;
		}
	    }

	closedir (dir2);
      }

 out:
  closedir (dir);

  return result;
}


int
NUMA_memnode_count (size_t setsize, const memnode_set_t *set)
{
  int s = 0;
  const __memnode_mask *p = set->__bits;
  const __memnode_mask *end = &set->__bits[setsize / sizeof (__memnode_mask)];

  while (p < end)
    {
      __memnode_mask l = *p++;

      if (l == 0)
	continue;

#if LONG_BIT > 32
      l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
      l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
      l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
      l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
      l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
      l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
#else
      l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
      l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
      l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
      l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
      l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
#endif

      s += l;
    }

  return s;
}


int
NUMA_mem_get_node_idx (void *addr)
{
  int res;
  if (syscall (__NR_get_mempolicy, &res, NULL, (size_t) 0, addr,
	       (unsigned long int) MPOL_F_ADDR) < 0)
    return -1;

  return res;
}


int
NUMA_mem_get_node_mask (void *addr, size_t size, size_t destsize,
			memnode_set_t *dest)
{
  /* We have to iterate over the entire address range.  This can be
     wasteful in case we deal with large pages.  But there is no
     alternative so far.  */
  size_t ps = sysconf (_SC_PAGESIZE);

  int result = 0;
  while (size > 0)
    {
      int res;
      if (syscall (__NR_get_mempolicy, &res, NULL, (size_t) 0, addr,
		   (unsigned long int) MPOL_F_ADDR) < 0)
	return -1;

      if (dest && !MEMNODE_ISSET_S (res, destsize, dest))
	{
	  if (MEMNODE_SET_S (res, destsize, dest) == 0)
	    {
	      errno = ERANGE;
	      return -1;
	    }

	  ++result;
	}

      addr = (char *) addr + ps;
      size -= MIN (size, ps);
    }

  return result;
}
