#ifndef __BITCASK_H__
#define __BITCASK_H__

#include <stdbool.h>
#include <stdint.h>

#include "record.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct bitcask_t Bitcask;

Bitcask*   bc_open(const char *path, int depth, time_t before);
void       bc_flush(Bitcask *bc, int limit);
void       bc_close(Bitcask *bc);
void       bc_merge(Bitcask *bc);
void       bc_optimize(Bitcask *bc, int limit);
DataRecord* bc_get(Bitcask *bc, const char* key);
bool       bc_set(Bitcask *bc, const char* key, char* value, int vlen, int flag, int version);
bool       bc_delete(Bitcask *bc, const char* key);
uint16_t   bc_get_hash(Bitcask *bc, const char * pos, int *count);
char*      bc_list(Bitcask *bc, const char* pos);
uint32_t   bc_count(Bitcask *bc, uint32_t* curr);
#ifdef __cplusplus
};  /* end of extern "C" */
#endif
	
#endif
