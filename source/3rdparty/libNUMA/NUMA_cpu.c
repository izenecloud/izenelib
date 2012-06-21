#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#include "libNUMA.h"



static int
read_sysfs (size_t destsize, cpu_set_t *dest)
{
  DIR *dir = opendir ("/sys/devices/system/cpu");
  if  (dir == NULL)
    return -1;

  int dfd = dirfd (dir);
  int count = 0;
  struct dirent64 *d;

  while ((d = readdir64 (dir)) != NULL)
    /* NB: the sysfs has d_type support.  */
    if (d->d_type == DT_DIR && strncmp (d->d_name, "cpu", 3) == 0)
      {
	char *endp;
	unsigned long int nr = strtoul (d->d_name + 3, &endp, 10);
	if (nr != ULONG_MAX && endp != d->d_name + 3 && *endp == '\0')
	  {
	    /* Try reading the online file.  */
	    char oname[_D_ALLOC_NAMLEN (d) + sizeof "/online"];
	    strcpy (stpcpy (oname, d->d_name), "/online");

	    int fd = openat (dfd, oname, O_RDONLY);
	    if (fd >= 0)
	      {
		char buf[1];
		ssize_t n = read (fd, buf, sizeof (buf));
		close (fd);
		if (n > 0)
		  {
		    if (buf[0] == '1')
		      {
			if (dest != NULL
			    && CPU_SET_S (nr, destsize, dest) == 0)
			  {
			    /* The bitmask is too small.  */
			    errno = ERANGE;
			    return -1;
			  }

			++count;
		      }
		  }
		else
		  goto cannot_read;
	      }
	    else
	      {
		/* If we cannot read the online file we have to assume
		   the CPU is online.  */
	      cannot_read:
		if (dest != NULL && CPU_SET_S (nr, destsize, dest) == 0)
		  {
		    /* The bitmask is too small.  */
		    errno = ERANGE;
		    return -1;
		  }

		++count;
	      }
	  }
      }

  closedir (dir);

  return count;
}


/* Return current number of online CPUs in the system.  */
int
NUMA_cpu_system_count (void)
{
  /* We try reading the /sys/devices/system/cpu directory and look at
     the individual online files.  */
  int count = read_sysfs (0, NULL);
  if (count >= 0)
    return count;

  /* Simply return the value sysconf reports.  */
  // XXX At some point when glibc has this functionality for long
  // XXX enough this is all that is needed.
  return sysconf (_SC_NPROCESSORS_ONLN);
}


/* Set bits for all online CPUs in the system and return the number of
   bits set.  */
int
NUMA_cpu_system_mask (size_t destsize, cpu_set_t *dest)
{
  /* First clear the old content.  */
  CPU_ZERO_S (destsize, dest);

  /* We try reading the /sys/devices/system/cpu directory and look at
     the individual online files.  */
  int count = read_sysfs (destsize, dest);
  if (count >= 0 || errno == ERANGE)
    return count;

  /* The sysfs is not available.  Use /proc/cpuinfo.  */
  FILE *fp = fopen ("/proc/cpuinfo", "r");
  if (fp == NULL)
    /* Note: errno is set to ENOENT or EPERM or something like this.  */
    return -1;

  count = 0;
  char *line = NULL;
  size_t len = 0;
  while (!feof_unlocked (fp))
    {
      ssize_t n = getline (&line, &len, fp);
      if (n <= 0)
	break;

      unsigned long int nr;
      if (sscanf (line, "processor : %lu", &nr) == 1)
	{
	  if (CPU_SET_S (nr, destsize, dest) == 0)
	    {
	      /* The bitmask is too small.  */
	      errno = ERANGE;
	      return -1;
	    }

	  ++count;
	}
    }

  return count;
}


/* Return current number of CPUs the thread is allowed to run on.  */
int
NUMA_cpu_self_count (void)
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
	      result = CPU_COUNT_S (dslen, ds);
	      break;
	    }
	}

      free (ds);

      return result;
    }

  return CPU_COUNT (&s);
}


/* Set bits for all CPUs the thread is allowed to run on.  */
int
NUMA_cpu_self_mask (size_t destsize, cpu_set_t *dest)
{
  if (sched_getaffinity (getpid (), destsize, dest) < 0)
    return -1;

  return CPU_COUNT_S (destsize, dest);
}


/* Return index of currently used CPU.  */
int
NUMA_cpu_self_current_idx (void)
{
  return sched_getcpu ();
}


/* Set bit for currently used CPU.  */
int
NUMA_cpu_self_current_mask (size_t destsize, cpu_set_t *dest)
{
  int idx = sched_getcpu ();
  if (idx < 0)
    return -1;

  CPU_ZERO_S (destsize, dest);

  return CPU_SET_S (idx, destsize, dest) == 0 ? -1 : 0;
}


static int
add_to_mask (size_t destsize, cpu_set_t *dest, const char *fname,
	     char **line, size_t *linelen)
{
  FILE *fp = fopen (fname, "r");
  if (fp == NULL)
    return 0;

  /* Should be only one line.  */
  int result = 0;
  if (getline (line, linelen, fp) > 0)
    {
      /* NB: line includes the terminating newline.  Every eight
	 digits there is a comma.  So, we divide by 9 and get the
	 number of represented nibbles.  */
      size_t tsidx = (strlen (*line) / 9) * 32;
      char *cp = *line;
      while (tsidx >= 4)
	{
	  if (*cp != ',')
	    {
	      if (*cp != '0')
		{
		  size_t val = isdigit (*cp) ? *cp - '0' : *cp - 'a' + 10;
		  for (size_t bit = 0; bit < 4; ++bit)
		    if ((val & (1 << bit)) != 0)
		      {
			if (CPU_SET_S (tsidx - 4 + bit, destsize, dest) == 0)
			  {
			    errno = ERANGE;
			    result = -1;
			    goto out;
			  }
		      }
		}

	      tsidx -= 4;
	    }

	  ++cp;
	}
    }

 out:
  fclose (fp);

  return result;
}


/* Return CPUs up to LEVEL from CPU in SRC.  */
ssize_t
NUMA_cpu_level_mask (size_t destsize, cpu_set_t *dest, size_t srcsize,
		     const cpu_set_t *src, unsigned int level)
{
  if (srcsize > destsize)
    {
      errno = ERANGE;
      return -1;
    }

  /* Always add the original set.  */
  CPU_ZERO_S (destsize, dest);
  memcpy (dest, src, srcsize);

  if (level == 0)
    return 0;

  char *line = NULL;
  size_t linelen = 0;

  /* Read the memory node information, if necessary.  */
  struct memnode
  {
    size_t idx;
    size_t ncpus;
    size_t ndists;
    struct memnode *next;
    size_t *cpus;
    size_t distances[0];
  } *allnodes = NULL;
  size_t maxdist_allowed = 0;

  if (level > 2)
    {
      DIR *dir = opendir ("/sys/devices/system/node");
      if (dir != NULL)
	{
	  size_t ndistsmax = 4;
	  size_t ncpusmax = 4;
	  int dfd = dirfd (dir);
	  struct dirent64 *d;

	  while ((d = readdir64 (dir)) != NULL)
	    /* NB: the sysfs has d_type support.  */
	    if (d->d_type == DT_DIR && strncmp (d->d_name, "node", 4) == 0)
	      {
		char *endp;
		unsigned long int nr = strtoul (d->d_name + 4, &endp, 10);
		if (nr != ULONG_MAX && endp != d->d_name + 4 && *endp == '\0')
		  {
		    /* Read the distance file.  */
		    char dname[sizeof ("node") + 3 * sizeof (long int)
			       + sizeof "/distance"];
		    strcpy (stpcpy (dname, d->d_name), "/distance");

		    int fd = openat (dfd, dname, O_RDONLY);
		    if (fd == -1)
		      {
			/* We cannot determine the node distances.
			   Fail altogether.  */
			allnodes = NULL;
			break;
		      }

		    if (line == NULL)
		      {
			linelen = 512;
			line = malloc (linelen);
			if (line == NULL)
			  {
			    allnodes = NULL;
			    break;
			  }
		      }

		    size_t remlen = linelen;
		    char *remp = line;
		    ssize_t n;

		    while ((n = read (fd, remp, remlen)) == (ssize_t) remlen)
		      {
			char *newp = realloc (line, linelen + 512);
			if (newp == NULL)
			  break;

			line = newp;
			remp = newp + (remp + remlen - line);
			remlen = 512;
			linelen += 512;
		      }

		    close (fd);

		    if (n == -1 || n == remlen)
		      {
			/* We could not read the file.  */
			allnodes = NULL;
			break;
		      }

		    size_t ndisttot = ndistsmax;
		    struct memnode *node = alloca (sizeof (*allnodes)
						   + (ndisttot
						      * sizeof (size_t)));
		    node->idx = nr;
		    size_t ndistcur = 0;

		    char *savep = NULL;
		    char *tok = strtok_r (line, " \n", &savep);
		    while (tok != NULL)
		      {
			if (ndistcur == ndisttot)
			  {
			    ndisttot *= 8;
			    struct memnode *newp
			      = alloca (sizeof (*allnodes)
					+ (ndisttot * sizeof (size_t)));
			    memcpy (newp->distances, node->distances,
				    ndistcur * sizeof (size_t));
			    node = newp;
			  }

			node->distances[ndistcur++] = atol (tok);

			tok = strtok_r (NULL, " \n", &savep);
		      }

		    node->ndists = ndistcur;
		    ndistsmax = MAX (ndistsmax, ndistcur);


		    int dfd2 = openat (dfd, d->d_name, O_RDONLY|O_DIRECTORY);
		    DIR *dir2 = fdopendir (dfd2);
		    if (dfd2 == -1 || dir2 == NULL)
		      {
			/* We cannot determine the processors which
			   belong to the node.  Fail altogether.  */
			if (dfd2 != -1)
			  close (dfd2);
			allnodes = NULL;
			break;
		      }

		    size_t ncpustot = ncpusmax;
		    node->cpus = alloca (ncpustot * sizeof (*node->cpus));
		    size_t ncpuscur = 0;

		    struct dirent64 *d2;
		    while ((d2 = readdir64 (dir2)) != NULL)
		      if (d2->d_type == DT_LNK
			  && strncmp (d2->d_name, "cpu", 3) == 0)
			{
			  char *endp;
			  unsigned long int nr = strtoul (d2->d_name + 3,
							  &endp, 10);
			  if (nr != ULONG_MAX && endp != d2->d_name + 3
			      && *endp == '\0')
			    {
			      if (ncpuscur == ncpustot)
				{
				  ncpustot *= 8;
				  size_t *newp = alloca (ncpustot
							 * sizeof (size_t));
				  node->cpus = memcpy (newp, node->cpus,
						       ncpuscur
						       * sizeof (size_t));
				}

			      node->cpus[ncpuscur++] = nr;
			    }
			}

		    closedir (dir2);

		    node->ncpus = ncpuscur;
		    ncpusmax = MAX (ncpusmax, ncpuscur);

		    node->next = allnodes;
		    allnodes = node;
		  }
	      }

	  closedir (dir);

	  /* Determine the values of the first LEVEL-2 distances.  */
	  size_t *mindists = alloca ((level - 2) * sizeof (size_t));
	  for (size_t cnt = 0; cnt < level - 2; ++cnt)
	    mindists[cnt] = ~((size_t) 0);
	  struct memnode *runp = allnodes;
	  while (runp != NULL)
	    {
	      for (size_t cnt = 0; cnt < runp->ndists; ++cnt)
		if (cnt != runp->idx)
		  {
		    size_t inner = level - 2;
		    while (inner > 0
			   && mindists[inner - 1] >= runp->distances[cnt])
		      --inner;

		    if (inner < level - 2
			&& mindists[inner] != runp->distances[cnt])
		      {
			memmove (&mindists[inner + 1],
				 &mindists[inner],
				 (level - 2 - (inner + 1)) * sizeof (size_t));
			mindists[inner] = runp->distances[cnt];
		      }
		}

	      runp = runp->next;
	    }

	  /* Determine the number of levels in the system.  */
	  while (level > 3 && mindists[level - 3])
	    --level;

	  maxdist_allowed = mindists[level - 3];
	}

      /* If we cannot provide the information for levels > 2 then
	 fail at this point.  */
      if (allnodes == NULL)
	{
	  errno = ENOENT;
	  return -1;
	}
    }

  /* Iterate over all processors in SRC.  */
  int result = level;
  size_t srccnt = CPU_COUNT_S (srcsize, src);
  size_t idx = 0;
  while (srccnt > 0)
    {
      if (CPU_ISSET_S (idx, srcsize, src))
	{
	  /* Always add thread siblings if we get here.  */
	  static const char tsfname[] = "\
/sys/devices/system/cpu/cpu%zu/topology/thread_siblings";
	  static const char csfname[] = "\
/sys/devices/system/cpu/cpu%zu/topology/core_siblings";
	  char fname[MAX (sizeof (tsfname), sizeof (csfname))
		     + 3 * sizeof (size_t)];

	  /* No need to read this file if we also read the core_siblings
	     file since all the bits set in core_siblings are also set
	     in thread_siblings.  */
	  snprintf (fname, sizeof (fname), level == 1 ? tsfname : csfname, idx);
	  int r = add_to_mask (destsize, dest, fname, &line, &linelen);
	  if (r < 0)
	    {
	      result = -1;
	      goto errout;
	    }

	  /* Add all CPUs on nodes which are connected to the node of
	     CPU IDX with a distance of MAXNODE_ALLOWED or less.  First
	     find the node this CPU is on.  */
	  struct memnode *runp = allnodes;
	  while (runp != NULL)
	    for (size_t cnt = 0; cnt < runp->ncpus; ++cnt)
	      if (idx == runp->cpus[cnt])
		{
		  /* Iterate over the distances.  For those not bigger
		     than the maximum add all CPUs.  */
		  for (cnt = 0; cnt < runp->ndists; ++cnt)
		    if (cnt != runp->idx
			&& runp->distances[cnt] <= maxdist_allowed)
		      {
			struct memnode *runp2 = allnodes;
			while (runp2 != NULL)
			  if (runp2->idx == cnt)
			    {
			      /* Add all CPUs.  */
			      for (size_t inner = 0; inner < runp2->ncpus;
				   ++inner)
				if (CPU_SET_S (runp2->cpus[inner],
					       destsize, dest) == 0)
				  {
				    errno = ERANGE;
				    result = -1;
				    goto errout;
				  }
			      break;
			    }
			  else
			    runp2 = runp2->next;
		      }

		  goto out;
		}
	out:

	  --srccnt;
	}
      ++idx;
    }

 errout:
  free (line);

  return result;
}
