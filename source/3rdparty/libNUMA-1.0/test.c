#include <libNUMA.h>
#include <stdio.h>

static void
print_set (size_t len, const void *p)
{
  const unsigned long int *tp = p;
  while (len > 0)
    {
      unsigned long int v = *tp++;
      unsigned long int mask = 1ul;
      while (mask != 0)
	{
	  putchar_unlocked ('0' + ((v & mask) != 0));
	  mask <<= 1;
	}

      len -= sizeof (*tp);
    }
}

int
main (void)
{
  cpu_set_t s;

  printf ("NUMA_cpu_system_count = %d\n", NUMA_cpu_system_count ());
  printf ("NUMA_memnode_system_count = %d\n", NUMA_memnode_system_count ());

  printf ("NUMA_cpu_system_mask = ");
  if (NUMA_cpu_system_mask (sizeof (s), &s) < 0)
    puts ("<N/A>");
  else
    {
      print_set (sizeof (s), &s);
      putchar_unlocked ('\n');
    }

  printf ("NUMA_cpu_self_count = %d\n", NUMA_cpu_self_count ());

  printf ("NUMA_cpu_self_mask = ");
  if (NUMA_cpu_self_mask (sizeof (s), &s) < 0)
    puts ("<N/A>");
  else
    {
      print_set (sizeof (s), &s);
      putchar_unlocked ('\n');
    }

  /* Find one valid CPU.  */
  puts ("NUMA_cpu_system_mask");
  if (NUMA_cpu_system_mask (sizeof (s), &s) >= 0 && CPU_COUNT (&s) != 0)
    {
      size_t idx = 0;
      while (! CPU_ISSET (idx, &s))
	++idx;

      CPU_ZERO (&s);
      CPU_SET (idx, &s);

      size_t level = 0;
      while (1)
	{
	  cpu_set_t s2;
	  ssize_t n;
	  n = NUMA_cpu_level_mask (sizeof (s2), &s2, sizeof (s), &s, level);
	  if (n < 0)
	    {
	      printf ("NUMA_cpu_level_mask for level %zu failed\n", level);
	      break;
	    }
	  if (n < level)
	    break;

	  printf ("level %zu: ", level);
	  print_set (sizeof (s2), &s2);
	  putchar_unlocked ('\n');

	  ++level;
	}
    }
  //cpu_set_t allcpus[2];

  //CPU_ZERO(&allcpus[0]);
  //CPU_SET(0, &allcpus[0]);
  //CPU_ZERO(&allcpus[1]);
  //CPU_SET(1, &allcpus[1]);

  //memnode_set_t memnodeset[2];
  //NUMA_cpu_tomemnode(2, allcpus, 2, memnodeset);
  //for(int i = 0; i < 2; i++)
  //{
  //}

  memnode_set_t* memsetp;
  size_t memnode_size;
  memsetp = MEMNODE_ALLOC(1);
  memnode_size = MEMNODE_ALLOC_SIZE(1);

  MEMNODE_ZERO_S(memnode_size, memsetp);
  MEMNODE_SET_S(0, memnode_size, memsetp);
  cpu_set_t*  cpu_out;
  size_t cpu_size;
  cpu_out = CPU_ALLOC(16);
  cpu_size = CPU_ALLOC_SIZE(16);
  CPU_ZERO_S(cpu_size, cpu_out);
  NUMA_memnode_to_cpu(memnode_size, memsetp, cpu_size, cpu_out);
  for(int i = 0; i < 16; i++)
  {
      if(CPU_ISSET_S(i, cpu_size, cpu_out))
      {
          printf("cpu %d is set for memnode 0\n", i);
      }
  }

  cpu_set_t  cpu_out_2;
  //size_t cpu_size_2;
  //cpu_out_2 = CPU_ALLOC(16);
  //cpu_size_2 = CPU_ALLOC_SIZE(16);
  for(int i = 0; i < 8; ++i)
  {
      CPU_ZERO(&cpu_out_2);
      CPU_SET(i, &cpu_out_2);
      int n = NUMA_cpu_level_mask(cpu_size, cpu_out, sizeof(cpu_out_2), &cpu_out_2, 1);
      printf("cpu %d for level 5 return %d\n", i, n);
      for(int k = 0; k < 8; ++k)
      {
          if(CPU_ISSET_S(k, cpu_size, cpu_out))
          {
              printf("cpu %d for level 5 cpuset %d\n", i, k);
          }
      }
  }

  CPU_FREE(cpu_out);
  //CPU_FREE(cpu_out_2);

  return 0;
}
