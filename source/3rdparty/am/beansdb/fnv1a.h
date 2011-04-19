#ifndef __FNV1A_H__
#define __FNV1A_H__

#define FNV_32_PRIME 0x01000193
#define FNV_32_INIT 0x811c9dc5
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int uint32_t;

inline uint32_t fnv1a(const char *key, int key_len)
{
  uint32_t h = FNV_32_INIT;
  int i;

  for (i=0; i<key_len; i++) {
      h ^= (uint32_t)key[i];
      h *= FNV_32_PRIME;
  }

  return h;
}
#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif
