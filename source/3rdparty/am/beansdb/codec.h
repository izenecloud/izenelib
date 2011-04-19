#ifndef __CODEC_H__
#define __CODEC_H__

#include "htree.h"

#ifdef __cplusplus
extern "C" {
#endif

void dc_init(HTree* htree, const char *path);
int dc_encode(HTree* htree, char* buf, const char *src, int len);
int dc_decode(HTree* htree, char* buf, const char *src, int len);
#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif

