#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include "bitcask.h"
#include "htree.h"
#include "record.h"

#define MAX_BUCKET_COUNT 256

const uint32_t MAX_RECORD_SIZE = 50 * 1024 * 1024; // 50M
const uint32_t MAX_BUCKET_SIZE = (uint32_t)1024 * 1024 * 1024 * 2; // 2G
const uint32_t WRITE_BUFFER_SIZE = 1024 * 1024 * 4; // 4M

const char DATA_FILE[] = "%s/%03d.data";
const char HINT_FILE[] = "%s/%03d.hint.qlz";
const char NEW_DATA_FILE[] = "%s/%03d.data.new";
const char NEW_HINT_FILE[] = "%s/%03d.hint.new.qlz";

struct bitcask_t {
    char*  path;
    int    depth;
    HTree* tree;
    int    curr;
    HTree* curr_tree;
    char   *write_buffer;
    int    wbuf_size;
    int    wbuf_start_pos;
    int    wbuf_curr_pos;
    pthread_mutex_t flush_lock;
    pthread_mutex_t buffer_lock;
    pthread_mutex_t write_lock;
};

Bitcask* bc_open(const char *path, int depth, time_t before)
{
    if (path == NULL || depth > 4) return NULL;
     if (0 != access(path, F_OK) && 0 != mkdir(path, 0750)){
        fprintf(stderr, "mkdir %s failed\n", path);
        return NULL;
    }
    Bitcask* bc = (Bitcask*)malloc(sizeof(Bitcask));
    memset(bc, 0, sizeof(Bitcask));    
    bc->path = strdup(path);
    bc->depth = depth;
    bc->tree = ht_new(depth);
    bc->curr_tree = ht_new(depth);
    bc->wbuf_size = 1024 * 4;
    bc->write_buffer = malloc(bc->wbuf_size);
    pthread_mutex_init(&bc->buffer_lock, NULL);
    pthread_mutex_init(&bc->write_lock, NULL);
    pthread_mutex_init(&bc->flush_lock, NULL);

    char datapath[255], hintpath[255];
    int i=0;
    for (i=0; i<MAX_BUCKET_COUNT; i++) {
        sprintf(datapath, DATA_FILE, path, i);
        FILE* f = fopen(datapath, "rb");
        if (NULL == f) break;
        fclose(f);
        
        sprintf(hintpath, HINT_FILE, path, i);
        struct stat st;
        if (before == 0){
            if (0 == lstat(hintpath, &st)){
                scanHintFile(bc->tree, i, hintpath, NULL);
            }else{
                scanDataFile(bc->tree, i, datapath, hintpath);                
            }
        }else{
            if (0 == lstat(hintpath, &st) && 
                (st.st_mtime < before || 0 == lstat(datapath, &st) && st.st_mtime < before)){
                scanHintFile(bc->tree, i, hintpath, NULL); 
            }else{
                scanDataFileBefore(bc->tree, i, datapath, before);
            }
        }
    }
    bc->curr = i;
//    ht_optimize(bc->tree);

    return bc;
}

/*
 * bc_close() is not thread safe, should stop other threads before call it.
 * */
void bc_close(Bitcask *bc)
{
    int i=0;
    pthread_mutex_lock(&bc->write_lock);
    bc_flush(bc, 0);
    
    if (NULL != bc->curr_tree) {
        char buf[255];
        sprintf(buf, HINT_FILE, bc->path, bc->curr);
        build_hint(bc->curr_tree, buf);
        bc->curr_tree = NULL;
    }
    bc->curr = 0;
    ht_destroy(bc->tree);
    free(bc->path);
    free(bc->write_buffer);
    free(bc);
}

void update_items(Item *it, void *args)
{
    HTree *tree = (HTree*) args;
    Item *p = ht_get(tree, it->name);
    if (!p) {
        fprintf(stderr, "Bug, item missed after optimized\n");
        return;
    }
    if (it->pos != p->pos && (it->pos & 0xff) == (p->pos & 0xff) ) {
        ht_add(tree, p->name, it->pos, p->hash, p->ver);
    }
}

void bc_optimize(Bitcask *bc, int limit)
{
    int i;
    for (i=0; i < bc->curr; i++) {
        char data[255], hint[255];
        sprintf(data, DATA_FILE, bc->path, i);
        sprintf(hint, HINT_FILE, bc->path, i);
        
        HTree *cur_tree = optimizeDataFile(bc->tree, i, data, hint, limit);
        if (NULL == cur_tree) continue;

        pthread_mutex_lock(&bc->write_lock);
        ht_visit(cur_tree, update_items, bc->tree);
        pthread_mutex_unlock(&bc->write_lock);

        build_hint(cur_tree, hint);
    }
}

DataRecord* bc_get(Bitcask *bc, const char* key)
{
    Item *item = ht_get(bc->tree, key);
    if (NULL == item) return NULL;
    if (item->ver < 0){
        free(item);
        return NULL;
    }
    
    int bucket = item->pos & 0xff;
    uint32_t pos = item->pos & 0xffffff00;
    if (bucket > bc->curr) {
        fprintf(stderr, "BUG: invalid bucket %d > %d\n", bucket, bc->curr);
        ht_remove(bc->tree, key);
        free(item);
        return NULL;
    }

    DataRecord* r = NULL;
    if (bucket == bc->curr) {
        pthread_mutex_lock(&bc->buffer_lock);
        if (bucket == bc->curr && pos >= bc->wbuf_start_pos){
            int p = pos - bc->wbuf_start_pos;
            r = decode_record(bc->write_buffer + p, bc->wbuf_curr_pos - p);
        }
        pthread_mutex_unlock(&bc->buffer_lock);
        
        if (r != NULL){
            free(item);
            return r;
        }
    }
        
    char data[255];
    sprintf(data, DATA_FILE, bc->path, bucket);
    FILE *f = fopen(data, "rb");
    if (NULL == f){
        goto GET_END;
    }
    
    if (0 != fseek(f, pos, SEEK_SET)){
        fprintf(stderr, "IOError: seek file %d to %d failed\n", bucket, pos);
        goto GET_END;
    }
    
    r = read_record(f, true);
    if (NULL == r){
        fprintf(stderr, "Bug: get %s failed in %s %d %d\n", key, bc->path, bucket, pos);        
    }else{
         // check key
        if (strcmp(key, r->key) != 0){
            fprintf(stderr, "Bug: record %s is not expected %s\n", r->key, key);
            free_record(r);
            r = NULL;
        } 
    }
GET_END:
    if (NULL == r)
        ht_remove(bc->tree, key);
    if (f != NULL) fclose(f);
    free(item);
    return r;
}

struct build_thread_args {
    HTree *tree;
    char *path;
};

void* build_thread(void *param)
{
    struct build_thread_args *args = (struct build_thread_args*) param;
    build_hint(args->tree, args->path);
    free(args->path);
    free(param);
    return NULL;
}

void bc_flush(Bitcask *bc, int limit)
{
    if (bc->curr >= MAX_BUCKET_COUNT) {
        fprintf(stderr, "reach max bucket count\n");
        exit(1);
    }
    
    pthread_mutex_lock(&bc->flush_lock);
    if (bc->wbuf_curr_pos > limit * 1024) {
        char buf[255];
        sprintf(buf, DATA_FILE, bc->path, bc->curr);
        FILE *f = fopen(buf, "ab");
        if (f == NULL) {
            fprintf(stderr, "open file %s for flushing failed.\n", buf);
            exit(1);
        }
        // check file size
        int last_pos = ftell(f);
        if (last_pos != bc->wbuf_start_pos) {
            fprintf(stderr, "last pos not match: %d != %d\n", last_pos, bc->wbuf_start_pos);
            exit(1);
        }
        
        int n = fwrite(bc->write_buffer, 1, bc->wbuf_curr_pos, f);
        
        pthread_mutex_lock(&bc->buffer_lock);
        if (n < bc->wbuf_curr_pos) {
            memmove(bc->write_buffer, bc->write_buffer + n, bc->wbuf_curr_pos - n);
        }
        bc->wbuf_start_pos += n;
        bc->wbuf_curr_pos -= n;
        if (bc->wbuf_curr_pos == 0 && bc->wbuf_size < WRITE_BUFFER_SIZE) {
            bc->wbuf_size *= 2;
            free(bc->write_buffer);
            bc->write_buffer = malloc(bc->wbuf_size);
        }
        if (bc->wbuf_start_pos + bc->wbuf_size > MAX_BUCKET_SIZE) {
            if (bc->wbuf_curr_pos > 0) {
                if (fwrite(bc->write_buffer, 1, bc->wbuf_curr_pos, f) < bc->wbuf_curr_pos){
                    fprintf(stderr, "write to %s failed\n", buf);
                    exit(1);
                }
            }
            // build in new thread
            char datapath[255];
            sprintf(datapath, HINT_FILE, bc->path, bc->curr);
            struct build_thread_args *args = (struct build_thread_args*)malloc(
                    sizeof(struct build_thread_args));
            args->tree = bc->curr_tree;
            args->path = strdup(datapath);
            pthread_t build_ptid;
            pthread_create(&build_ptid, NULL, build_thread, args);
            // next bucket
            bc->curr ++;
            bc->curr_tree = ht_new(bc->depth);
            bc->wbuf_start_pos = 0;
            bc->wbuf_curr_pos = 0;
        }
        pthread_mutex_unlock(&bc->buffer_lock);
        
        fclose(f);
    }
    pthread_mutex_unlock(&bc->flush_lock);
}

bool bc_set(Bitcask *bc, const char* key, char* value, int vlen, int flag, int version)
{
    if (version < 0 && vlen > 0 || vlen > MAX_RECORD_SIZE){
        fprintf(stderr, "invalid set cmd \n");
        return false;
    }

    bool suc = false;
    pthread_mutex_lock(&bc->write_lock);
    
    int oldv = 0, ver = version;
    Item *it = ht_get(bc->tree, key);
    if (it != NULL) {
        oldv = it->ver;
    }
    
    if (version == 0 && oldv > 0){ // replace
        ver = oldv + 1;
    } else if (version == 0 && oldv <= 0){ // add
        ver = 1;
    } else if (version < 0 && oldv <= 0) { // delete, not exist
        goto SET_FAIL;
    } else if (version == -1) { // delete
        ver = - abs(oldv) - 1;
    } else if (abs(version) <= abs(oldv)) { // sync
        goto SET_FAIL;
    } else { // sync
        ver = version;
    }
    
    uint16_t hash = gen_hash(value, vlen);
    if (ver < 0) hash = 0;

    if (NULL != it && hash == it->hash) {
        DataRecord *r = bc_get(bc, key);
        if (r != NULL && r->flag == flag && vlen  == r->vsz
             && memcmp(value, r->value, vlen) == 0) {
            if (version != 0){
                // update version
                ht_add(bc->tree, key, it->pos, it->hash, ver);
                if (it->pos & 0xff == bc->curr){
                    if (bc->curr_tree == NULL) {
                        fprintf(stderr, "BUG: curr_tree should not be NULL\n");
                    }else{
                        ht_add(bc->curr_tree, key, it->pos, it->hash, ver);
                    }
                }
            }
            suc = true;
            free_record(r);
            goto SET_FAIL;
        }
    }
    
    int klen = strlen(key);
    DataRecord *r = malloc(sizeof(DataRecord) + klen);
    r->ksz = klen;
    memcpy(r->key, key, klen);
    r->vsz = vlen;
    r->value = value;
    r->free_value = false;
    r->flag = flag;
    r->version = ver;
    r->tstamp = time(NULL);

    int rlen;
    char *rbuf = encode_record(r, &rlen);
    if (rbuf == NULL || (rlen & 0xff) != 0){
        fprintf(stderr, "encode_record() failed with %d\n", rlen);
        if (rbuf != NULL) free(rbuf);
        goto SET_FAIL; 
    }

    pthread_mutex_lock(&bc->buffer_lock);
    if (bc->wbuf_curr_pos + rlen > bc->wbuf_size) {
        pthread_mutex_unlock(&bc->buffer_lock);
        bc_flush(bc, 0);
        pthread_mutex_lock(&bc->buffer_lock);
    }
    // record maybe larger than buffer
    while (bc->wbuf_curr_pos + rlen > bc->wbuf_size) {
        bc->wbuf_size *= 2;
        bc->write_buffer = realloc(bc->write_buffer, bc->wbuf_size);
    }
    memcpy(bc->write_buffer + bc->wbuf_curr_pos, rbuf, rlen);
    int pos = (bc->wbuf_start_pos + bc->wbuf_curr_pos) | bc->curr;
    bc->wbuf_curr_pos += rlen;
    pthread_mutex_unlock(&bc->buffer_lock);
   
    ht_add(bc->tree, key, pos, hash, ver);
    ht_add(bc->curr_tree, key, pos, hash, ver);
    suc = true;
    free(rbuf);
    free_record(r);

SET_FAIL:
    pthread_mutex_unlock(&bc->write_lock);
    if (it != NULL) free(it);
    return suc;
}

bool bc_delete(Bitcask *bc, const char* key)
{
    return bc_set(bc, key, "", 0, 0, -1);
}

uint16_t bc_get_hash(Bitcask *bc, const char * pos, int *count)
{
    return ht_get_hash(bc->tree, pos, count);
}

char* bc_list(Bitcask *bc, const char* pos)
{
    return ht_list(bc->tree, pos);
}

uint32_t   bc_count(Bitcask *bc, uint32_t* curr)
{
    uint32_t total = 0;
    ht_get_hash(bc->tree, "@", &total);
    if (NULL != curr && NULL != bc->curr_tree) {
        ht_get_hash(bc->curr_tree, "@", curr);
    }
    return total;
}
