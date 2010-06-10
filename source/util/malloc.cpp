/* Prototypes for __malloc_hook, __free_hook */
#include <malloc.h>
#include <stdio.h>
#include <stdexcept>
/* Prototypes for our hooks.  */
static void my_malloc_init_hook (void);
static void my_realloc_init_hook (void);

static void *my_malloc_hook (size_t, const void *);
static void *my_realloc_hook (void *ptr, size_t size,
                              const void *caller);

void *(*old_malloc_hook) __MALLOC_PMT ((size_t __size,
					     __const __malloc_ptr_t));
void *(*old_realloc_hook) __MALLOC_PMT ((void *__ptr, size_t __size,
					      __const __malloc_ptr_t));

/* Override initializing hook from the C library. */
void (*__malloc_initialize_hook) (void) = my_malloc_init_hook;
void (*__realloc_initialize_hook) (void) = my_realloc_init_hook;
     
static void
my_malloc_init_hook (void)
{
  old_malloc_hook = __malloc_hook;
  __malloc_hook = my_malloc_hook;
}
     
static void
my_realloc_init_hook (void)
{
  old_realloc_hook = __realloc_hook;
  __realloc_hook = my_realloc_hook;
}
     
static void *
my_malloc_hook (size_t size, const void *caller)
{
  void *result;
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  /* Call recursively */
  result = malloc (size);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  //printf ("malloc (%u) returns %p\n", (unsigned int) size, result);

  if (!result)
    throw std::runtime_error("[ERROR]:Out of memory.");
  return result;
}
     
static void *
my_realloc_hook (void* ptr, size_t size, const void *caller)
{
  void *result;
  /* Restore all old hooks */
  __realloc_hook = old_realloc_hook;
  /* Call recursively */
  result = realloc (ptr, size);
  /* Save underlying hooks */
  old_realloc_hook = __realloc_hook;
  /* printf might call malloc, so protect it too. */
  //printf ("realloc (%u) returns %p\n", (unsigned int) size, result);
  /* Restore our own hooks */
  __realloc_hook = my_realloc_hook;
  
  if (!result)
    throw std::runtime_error("[ERROR]:Out of memory.");
  return result;
}



