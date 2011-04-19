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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include "htree.h"
#include "hstore.h"
#include "bitcask.h"

#define NUM_OF_MUTEX 97
const int APPEND_FLAG  = 0x00000100;
const int INCR_FLAG    = 0x00000204;

struct t_hstore {
    int height;
    time_t before;
    int op_start, op_end, op_limit; // for optimization
    Bitcask** bitcasks;
    pthread_mutex_t locks[NUM_OF_MUTEX];
};

inline int get_index(HStore *store, char *key)
{
    if (store->height == 0) return 0;
    uint32_t h = fnv1a(key, strlen(key));
    return h >> ((8 - store->height) * 4);
}

inline pthread_mutex_t* get_mutex(HStore *store, char *key)
{
    uint32_t i = fnv1a(key, strlen(key)) % NUM_OF_MUTEX;
    return &store->locks[i];
}

HStore* hs_open(const char *path, int height, time_t before)
{
    if (NULL == path) return NULL;
    if (0 != access(path, F_OK) && 0 != mkdir(path, 0750)){
        fprintf(stderr, "mkdir %s failed\n", path);
        return NULL;
    }
    if (height < 0 || height > 3) {
        fprintf(stderr, "invalid db height: %d\n", height);
        return NULL; 
    }
    if (before != 0){
        if (before<0) {
            fprintf(stderr, "invalid time:%ld\n", before);
            return NULL;
        }else{
            fprintf(stderr, "serve data modified before %s\n", ctime(&before));
        }
    }
    if (height > 1){
        // try to mkdir
        HStore *s = hs_open(path, height-1, 0);
        if (s == NULL){
            return NULL;
        }
        hs_close(s);
    }

    int i, j, count = 1 << (height * 4);
    HStore *store = (HStore*) malloc(sizeof(HStore));
    store->height = height;
    store->before = before;
    store->op_start = 0;
    store->op_end = 0;
    store->op_limit = 0;
    store->bitcasks = (Bitcask**) malloc(sizeof(Bitcask*) * count);
    memset(store->bitcasks, 0, sizeof(Bitcask*) * count);
    for (i=0; i<NUM_OF_MUTEX; i++) {
        pthread_mutex_init(&store->locks[i], NULL);
    }

    char format[255], buf[255], format2[255], buf2[255];
    for (i=0; i<count; i++){
        switch(height){
            case 0: sprintf(buf, "%s/", path); break;
            case 1: sprintf(buf, "%s/%x/", path, i); break;
            case 2: sprintf(buf, "%s/%x/%x/", path, i>>4, i & 0xf); break;
            case 3: sprintf(buf, "%s/%x/%x/%x/", path, i>>8, (i>>4)&0xf, i&0xf); break;
        }
        Bitcask* bc = bc_open(buf, height, before);
        if (bc == NULL){
            return NULL;
        }
        store->bitcasks[i] = bc;
    }

    return store;
}

void hs_flush(HStore *store, int limit)
{
    if (!store) return;
    if (store->before > 0) return;
    int i, count = 1 << (store->height * 4);
    for (i=0; i<count; i++){
        bc_flush(store->bitcasks[i], limit);
    }
}

void hs_close(HStore *store)
{
    if (!store) return;
    int i, count = 1 << (store->height * 4);
    for (i=0; i<count; i++){
        bc_close(store->bitcasks[i]);
    }
    free(store->bitcasks);
    free(store);
}

static uint16_t hs_get_hash(HStore *store, char *pos, uint32_t *count)
{
    if (strlen(pos) >= store->height){
        pos[store->height] = 0;
        int index = strtol(pos, NULL, 16);
        return bc_get_hash(store->bitcasks[index], "@", count);
    }else{
        int i, hash=0;
        *count = 0;
        char pos_buf[255];
        for (i=0; i<16; i++){
            int h,c;
            sprintf(pos_buf, "%s%x", pos, i);
            h = hs_get_hash(store, pos_buf, &c);
            hash *= 97;
            hash += h;
            *count += c;
        }
        return hash;
    }
}

static char* hs_list(HStore *store, char *key)
{
    int pos = strlen(key);
    if (pos >= store->height){
        char buf[10] = {0};
        memcpy(buf, key, store->height);
        int index = strtol(buf, NULL, 16);
        return bc_list(store->bitcasks[index], key + store->height);
    }else{
        int i, bsize = 1024, used = 0;
        char *buf = malloc(bsize);
        for (i=0; i < 16; i++) {
            char pos_buf[255];
            if (strlen(key)){
                sprintf(pos_buf, "%s%x", key, i);
            }else{
                sprintf(pos_buf, "%x", i);
            }
            uint32_t hash, count;
            hash = hs_get_hash(store, pos_buf, &count);
            used += snprintf(buf + used, bsize - used, "%x/ %u %u\n", i, hash & 0xffff, count);
        }
        return buf;
    }
}

char *hs_get(HStore *store, char *key, int *vlen, uint32_t *flag)
{
    if (!key || !store) return NULL;

    if (key[0] == '@'){
        char *r = hs_list(store, key+1);
        if (r) *vlen = strlen(r);
        *flag = 0;
        return r;
    }
    
    bool info = false;
    if (key[0] == '?'){
        info = true;
        key ++;
    }
    int index = get_index(store, key);
    DataRecord *r = bc_get(store->bitcasks[index], key);
    if (r == NULL){
        return NULL;
    }
    
    char *res = NULL;
    if (info){
        res = malloc(256);
        uint16_t hash = 0;
        if (r->version > 0){
            hash = gen_hash(r->value, r->vsz);
        }
        *vlen = snprintf(res, 255, "%d %u %u %u %u", r->version, 
            hash, r->flag, r->vsz, r->tstamp);
        *flag = 0;
    }else if (r->version > 0){
        res = record_value(r);
        r->value = NULL;
        *vlen = r->vsz;
        *flag = r->flag;
    }
    free_record(r);
    return res;
}

bool hs_set(HStore *store, char *key, char* value, int vlen, uint32_t flag, int ver)
{
    if (!store || !key || key[0] == '@') return false;
    if (store->before > 0) return false;
    
    int index = get_index(store, key);
    return bc_set(store->bitcasks[index], key, value, vlen, flag, ver);
}

bool hs_append(HStore *store, char *key, char* value, int vlen)
{
    if (!store || !key || key[0] == '@') return false;
    if (store->before > 0) return false;
    
    pthread_mutex_t *lock = get_mutex(store, key);
    pthread_mutex_lock(lock);

    int suc = false;
    int rlen = 0, flag = APPEND_FLAG;
    char *body = hs_get(store, key, &rlen, &flag);
    if (body != NULL && flag != APPEND_FLAG) {
        fprintf(stderr, "try to append %s with flag=%x\n", key, flag);
        goto APPEND_END;
    }
    body = realloc(body, rlen + vlen);
    memcpy(body + rlen, value, vlen);
    suc = hs_set(store, key, body, rlen + vlen, flag, 0); // TODO: use timestamp
    
APPEND_END:    
    if (body != NULL) free(body);
    pthread_mutex_unlock(lock);
    return suc;
}

int64_t hs_incr(HStore *store, char *key, int64_t value)
{
    if (!store || !key || key[0] == '@') return 0;
    if (store->before > 0) return 0;
    
    pthread_mutex_t *lock = get_mutex(store, key);
    pthread_mutex_lock(lock);

    int64_t result = 0;
    int rlen = 0, flag = INCR_FLAG;
    char buf[25];
    char *body = hs_get(store, key, &rlen, &flag);
    
    if (body != NULL) {
        if (flag != INCR_FLAG || rlen > 22) {
            fprintf(stderr, "try to incr %s but flag=0x%x, len=%d", key, flag, rlen);
            goto INCR_END; 
        }
        result = strtoll(body, NULL, 10);
        if (result == 0 && errno == EINVAL) {
            fprintf(stderr, "incr %s failed: %s\n", key, buf);
            goto INCR_END;
        }
    }

    result += value;
    if (result < 0) result = 0;
    rlen = sprintf(buf, "%lld", (long long int) result); 
    if (!hs_set(store, key, buf, rlen, INCR_FLAG, 0)) { // use timestamp later
        result = 0; // set failed
    }

INCR_END:
    pthread_mutex_unlock(lock);
    if (body != NULL) free(body);
    return result;
}

void* do_optimize(void *arg)
{
    HStore *store = (HStore *) arg;
    fprintf(stderr, "start to optimize from %d to %d\n", 
        store->op_start, store->op_end);
    for (; store->op_start < store->op_end; store->op_start ++) {
        bc_optimize(store->bitcasks[store->op_start], store->op_limit);
    }
    store->op_start = store->op_end = 0;
    return NULL;
}

bool hs_optimize(HStore *store, int limit)
{
    if (store->before > 0) return false;
    bool processing = store->op_start < store->op_end;
    if (processing) {
        store->op_start = store->op_end = 0;
    }else{
        pthread_t id;
        store->op_limit = limit;
        store->op_start = 0;
        store->op_end = 1 << (store->height * 4);
        pthread_create(&id, NULL, do_optimize, store);
    }
    
    return true;
}

bool hs_delete(HStore *store, char *key)
{
    if (!key || !store) return false;
    if (store->before > 0) return false;

    int index = get_index(store, key);
    return bc_delete(store->bitcasks[index], key);
}

uint64_t hs_count(HStore *store, uint64_t *curr)
{
    uint64_t total = 0, curr_total = 0;
    int i, j, count = 1 << (store->height * 4);
    for (i=0; i<count; i++) {
        uint32_t curr = 0;
        total += bc_count(store->bitcasks[i], &curr);
        curr_total += curr;
    }
    
    if (NULL != curr)  *curr = curr_total;
    return total;
}
