/**
 * @file util/bzip.h
 * @author Peisheng Wang
 * @date Created 2010-02-02
 * 
 * @brief bzip compression used in tokyocabinet
 *
 */

#ifndef BZIP_H_
#define BZIP_H_

#include <types.h>
#include <bzlib.h>
#include <cstdlib>

NS_IZENELIB_UTIL_BEGIN

#define BZIPBUFSIZ     8192

//note that, return pointer must be freed at last to prevent from memory leakage

static inline char *_tc_bzcompress(const char *ptr, int size, int *sp){
  assert(ptr && size >= 0 && sp);
  bz_stream zs;
  zs.bzalloc = NULL;
  zs.bzfree = NULL;
  zs.opaque = NULL;
  if(BZ2_bzCompressInit(&zs, 9, 0, 0) != BZ_OK) return NULL;
  int asiz = size + 16;
  if(asiz < BZIPBUFSIZ) asiz = BZIPBUFSIZ;
  char *buf;
  if(!(buf = (char*)malloc(asiz))){
    BZ2_bzCompressEnd(&zs);
    return NULL;
  }
  char obuf[BZIPBUFSIZ];
  int bsiz = 0;
  zs.next_in = (char *)ptr;
  zs.avail_in = size;
  zs.next_out = obuf;
  zs.avail_out = BZIPBUFSIZ;
  int rv;
  while((rv = BZ2_bzCompress(&zs, BZ_FINISH)) == BZ_FINISH_OK){
    int osiz = BZIPBUFSIZ - zs.avail_out;
    if(bsiz + osiz > asiz){
      asiz = asiz * 2 + osiz;
      char *swap;
      if(!(swap = (char*)realloc(buf, asiz))){
        free(buf);
        BZ2_bzCompressEnd(&zs);
        return NULL;
      }
      buf = swap;
    }
    memcpy(buf + bsiz, obuf, osiz);
    bsiz += osiz;
    zs.next_out = obuf;
    zs.avail_out = BZIPBUFSIZ;
  }
  if(rv != BZ_STREAM_END){
    free(buf);
    BZ2_bzCompressEnd(&zs);
    return NULL;
  }
  int osiz = BZIPBUFSIZ - zs.avail_out;
  if(bsiz + osiz + 1 > asiz){
    asiz = asiz * 2 + osiz;
    char *swap;
    if(!(swap = (char*)realloc(buf, asiz))){
      free(buf);
      BZ2_bzCompressEnd(&zs);
      return NULL;
    }
    buf = swap;
  }
  memcpy(buf + bsiz, obuf, osiz);
  bsiz += osiz;
  buf[bsiz] = '\0';
  *sp = bsiz;
  BZ2_bzCompressEnd(&zs);
  return buf;
}


static inline char *_tc_bzdecompress(const char *ptr, int size, int *sp){
  assert(ptr && size >= 0 && sp);
  bz_stream zs;
  zs.bzalloc = NULL;
  zs.bzfree = NULL;
  zs.opaque = NULL;
  if(BZ2_bzDecompressInit(&zs, 0, 0) != BZ_OK) return NULL;
  int asiz = size * 2 + 16;
  if(asiz < BZIPBUFSIZ) asiz = BZIPBUFSIZ;
  char *buf;
  if(!(buf = (char*)malloc(asiz))){
    BZ2_bzDecompressEnd(&zs);
    return NULL;
  }
  char obuf[BZIPBUFSIZ];
  int bsiz = 0;
  zs.next_in = (char *)ptr;
  zs.avail_in = size;
  zs.next_out = obuf;
  zs.avail_out = BZIPBUFSIZ;
  int rv;
  while((rv = BZ2_bzDecompress(&zs)) == BZ_OK){
    int osiz = BZIPBUFSIZ - zs.avail_out;
    if(bsiz + osiz >= asiz){
      asiz = asiz * 2 + osiz;
      char *swap;
      if(!(swap = (char*)realloc(buf, asiz))){
        free(buf);
        BZ2_bzDecompressEnd(&zs);
        return NULL;
      }
      buf = swap;
    }
    memcpy(buf + bsiz, obuf, osiz);
    bsiz += osiz;
    zs.next_out = obuf;
    zs.avail_out = BZIPBUFSIZ;
  }
  if(rv != BZ_STREAM_END){
    free(buf);
    BZ2_bzDecompressEnd(&zs);
    return NULL;
  }
  int osiz = BZIPBUFSIZ - zs.avail_out;
  if(bsiz + osiz >= asiz){
    asiz = asiz * 2 + osiz;
    char *swap;
    if(!(swap = (char*)realloc(buf, asiz))){
      free(buf);
      BZ2_bzDecompressEnd(&zs);
      return NULL;
    }
    buf = swap;
  }
  memcpy(buf + bsiz, obuf, osiz);
  bsiz += osiz;
  buf[bsiz] = '\0';
  *sp = bsiz;
  BZ2_bzDecompressEnd(&zs);
  return buf;
}

NS_IZENELIB_UTIL_END

#endif /*BZIP_H_*/
