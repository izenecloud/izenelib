/*
 *  Beansdb - A high available distributed key-value storage system:
 *
 *      http://beansdb.googlecode.com
 *
 *  Copyright 2009 Douban Inc.  All rights reserved.
 *
 *  Use and distribution licensed under the BSD license.  See
 *  the LICENSE file for full text.
 *
 *  Authors:
 *      Davies Liu <davies.liu@gmail.com>
 */
#ifndef __HTREE_H__
#define __HTREE_H__

#include <stdbool.h>
#include <stdint.h>

#include "quicklz.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct t_item Item;
struct t_item {
    uint32_t pos;
    int32_t  ver;
    uint16_t hash;
    uint8_t  length;
    char     name[1];
};

#define ITEM_PADDING 1

typedef struct t_data Data;
struct t_data {
    int size;
    int used;
    int count;
    Item head[0];
};

typedef struct t_node Node;
struct t_node {
    uint16_t is_node:1;
    uint16_t valid:1;
    uint16_t depth:4;
    uint16_t compressed:1;
    uint16_t flag:9;
    uint16_t hash;
    uint32_t count;
    Data *data;
};

typedef struct {
    int nargs;
    char fmt[0];
} Fmt;

struct t_hash_tree {
    int depth;
    int height;
    Node *root;
    int pool_size;
    pthread_mutex_t lock;
    char buf[512];

    bool compress;
    char wbuf[QLZ_SCRATCH_COMPRESS];
    char cbuf[1024 * 10];

    Fmt** dict;
    int* rdict;
    pthread_mutex_t dict_lock;
};


typedef struct t_hash_tree HTree;
typedef void (*fun_visitor) (Item *it, void *param);

uint32_t fnv1a(const char *key, int key_len);

HTree*   ht_new(int depth);
void     ht_destroy(HTree *tree);
void     ht_add(HTree *tree, const char* key, uint32_t pos, uint16_t hash, int32_t ver);
void     ht_remove(HTree *tree, const char *key);
Item*    ht_get(HTree *tree, const char *key);
uint32_t ht_get_hash(HTree *tree, const char *key, int *count);
char*    ht_list(HTree *tree, const char *dir);
void     ht_visit(HTree *tree, fun_visitor visitor, void *param);
void     ht_optimize(HTree *tree);
#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif /* __HTREE_H__ */
