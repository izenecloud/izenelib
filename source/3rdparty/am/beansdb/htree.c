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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "fnv1a.h"
#include "htree.h"
#include "codec.h"

const int MAX_KEY_LENGTH = 200;
const int BUCKET_SIZE = 16;
const int SPLIT_LIMIT = 32; 
const int MAX_DEPTH = 8;
static const int g_index[] = {0, 1, 17, 289, 4913, 83521, 1419857, 24137569, 410338673};

#define max(a,b) ((a)>(b)?(a):(b))
#define INDEX(it) (0x0f & (keyhash >> ((7 - node->depth - tree->depth) * 4)))
#define KEYLENGTH(it) ((it)->length-sizeof(Item)+ITEM_PADDING)
#define HASH(it) ((it)->hash * ((it)->ver>0))

// forward dec
static void add_item(HTree *tree, Node *node, Item *it, uint32_t keyhash, bool enlarge);
static void remove_item(HTree *tree, Node *node, Item *it, uint32_t keyhash);
static void split_node(HTree *tree, Node *node);
static void merge_node(HTree *tree, Node *node);
static void update_node(HTree *tree, Node *node);

inline uint32_t get_pos(HTree *tree, Node *node)
{
    return (node - tree->root) - g_index[(int)node->depth];
}

inline Node *get_child(HTree *tree, Node *node, int b)
{
    int i = g_index[node->depth + 1] + (get_pos(tree, node) << 4) + b;
    return tree->root + i;
}

inline Data* get_data(Node *node)
{
    if (node->compressed) {
        char wbuf[QLZ_SCRATCH_DECOMPRESS];
        Data *d = malloc(qlz_size_decompressed((char*)node->data));
        qlz_decompress((char*)node->data, (char*)d, wbuf);
        return d;
    } else {
        return node->data;
    }
}

static int saved = 0;

inline void set_data(HTree *tree, Node *node, Data *data)
{
    if (data != node->data) {
        if (node->data) free(node->data);
        if (tree->compress) {
            int need = data->used + 400;
            char *cbuf = need <= sizeof(tree->cbuf) ? tree->cbuf : malloc(need);
            size_t size = qlz_compress((char*)data, cbuf, data->used, tree->wbuf);
            saved += data->size - size;
            free(data);
            data = malloc(size);
            memcpy(data, cbuf, size);
            node->compressed = 1;
            if (cbuf != tree->cbuf) free(cbuf);
        }
        node->data = data;
    }
}

inline void free_data(Node *node, Data *data)
{
    if (data != node->data) {
        free(data);
    }
}

inline uint32_t key_hash(HTree *tree, Item* it)
{
    char buf[255];
    int n = dc_decode(tree, buf, it->name, KEYLENGTH(it));
    return fnv1a(buf, n);
}

static Item* create_item(HTree *tree, const char* name, uint32_t pos, uint16_t hash, int32_t ver)
{
    Item *it = (Item*)tree->buf;
    it->pos = pos;
    it->ver = ver;
    it->hash = hash;
    int n = dc_encode(tree,it->name, name, strlen(name));
    it->length = sizeof(Item) + n - ITEM_PADDING;
    return it;
}

static void enlarge_pool(HTree *tree)
{
    int i;
    int old_size = tree->pool_size;
    int new_size = g_index[tree->height + 1];
    
    tree->root = (Node*)realloc(tree->root, sizeof(Node) * new_size);
    memset(tree->root + old_size, 0, sizeof(Node) * (new_size - old_size));
    for (i=old_size; i<new_size; i++){
        tree->root[i].depth = tree->height;
    }

    tree->height ++;
    tree->pool_size = new_size;
}

static void clear(HTree *tree, Node *node)
{
    if (node->data) free(node->data);
    node->data = (Data*) malloc(64);
    node->data->size = 64;
    node->data->used = sizeof(Data);
    node->data->count = 0;
    
    node->is_node = 0;
    node->valid = 1;
    node->compressed = 0;
    node->count = 0;
    node->hash = 0;
}

static void add_item(HTree *tree, Node *node, Item *it, uint32_t keyhash, bool enlarge)
{
    while (node->is_node) {
        node->valid = 0;
        node = get_child(tree, node, INDEX(it));
    }

    Data *data = get_data(node);
    Item *p = data->head;
    int i;
    for (i=0; i<data->count; i++){
        if (it->length == p->length && 
                memcmp(it->name, p->name, KEYLENGTH(it)) == 0){
            node->hash += (HASH(it) - HASH(p)) * keyhash;
            node->count += it->ver > 0;
            node->count -= p->ver > 0;
            memcpy(p, it, sizeof(Item));
            set_data(tree, node, data);
            return;
        }
        p = (Item*)((char*)p + p->length);
    }

    if (data->size < data->used + it->length){
        int size = max(data->used + it->length, data->size + 64);
        int pos = (char*)p-(char*)data;
        Data *new_data = (Data*) malloc(size);
        memcpy(new_data, data, data->used);
        free_data(node, data);
        data = new_data;
        data->size = size;
        p = (Item *)((char*)data + pos);
    }
    
    memcpy(p, it, it->length);
    data->count ++;
    data->used += it->length;
    node->count += it->ver > 0;
    node->hash += keyhash * HASH(it);
    set_data(tree, node, data);
    
    if (node->count > SPLIT_LIMIT){
        if (node->depth == tree->height - 1){
            if (enlarge && node->count > SPLIT_LIMIT * 4){
                int pos = node - tree->root;
                enlarge_pool(tree);
                node = tree->root + pos; // reload
                split_node(tree, node);
            }
        }else{
            split_node(tree, node);
        }
    }
}

static void split_node(HTree *tree, Node *node)
{
    Node *child = get_child(tree, node, 0);
    int i;
    for (i=0; i<BUCKET_SIZE; i++){
        clear(tree, child+i);
    }
    
    Data *data = get_data(node);
    Item *it = data->head;
    for (i=0; i<data->count; i++) {
        int32_t keyhash = key_hash(tree,it);
        add_item(tree, child + INDEX(it), it, keyhash, false);
        it = (Item*)((char*)it + it->length);
    }
   
    free_data(node, data);
    free(node->data);
    node->data = NULL;
    
    node->is_node = 1;
    node->valid = 0;
}

static void remove_item(HTree *tree, Node *node, Item *it, uint32_t keyhash)
{
    while (node->is_node) {
        node->valid = 0;
        node = get_child(tree, node, INDEX(it));
    }
    
    Data *data = get_data(node);
    if (data->count == 0) return ;
    Item *p = data->head;
    int i;
    for (i=0; i<data->count; i++){
        if (it->length == p->length && 
                memcmp(it->name, p->name, KEYLENGTH(it)) == 0){
            data->count --;
            data->used -= p->length;
            node->count -= p->ver > 0;
            node->hash -= keyhash * HASH(p);
            memcpy(p, (char*)p + p->length, 
                    data->size - ((char*)p - (char*)data) - p->length);
            set_data(tree, node, data);
            return;
        }
        p = (Item*)((char*)p + p->length);
    }
    free_data(node, data);
}

static void merge_node(HTree *tree, Node *node)
{
    clear(tree, node);

    Node* child = get_child(tree, node, 0);
    int i, j;
    for (i=0; i<BUCKET_SIZE; i++){
        Data *data = get_data(child+i); 
        Item *it = data->head;
        int count = (child+i)->count;
        for (j=0; j < count; j++){
            if (it->ver > 0) {
                add_item(tree, node, it, key_hash(tree,it), false);
            } // drop deleted items, ver < 0
            it = (Item*)((char*)it + it->length);
        }
        free_data(child+i, data);
        clear(tree, child + i);
    }
}

static void update_node(HTree *tree, Node *node)
{
    if (node->valid) return ;
    
    int i;
    node->hash = 0;
    if (node->is_node){
        Node *child = get_child(tree, node, 0);
        node->count = 0;
        for (i=0; i<BUCKET_SIZE; i++){
            update_node(tree, child+i);
            node->count += child[i].count;
        }
        for (i=0; i<BUCKET_SIZE; i++){
            if (node->count > 128){
                node->hash *= 97;               
            }
            node->hash += child[i].hash;
        }
    }
    node->valid = 1;
    
    // merge nodes
    if (node->count <= SPLIT_LIMIT) {
        merge_node(tree, node);
    }
}

static Item* get_item_hash(HTree* tree, Node* node, Item* it, uint32_t keyhash)
{
    while (node->is_node) {
        node = get_child(tree, node, INDEX(it));
    }
    
    Data *data = get_data(node);
    Item *p = data->head, *r = NULL;
    int i;
    for (i=0; i<data->count; i++){
        if (it->length == p->length && 
                memcmp(it->name, p->name, KEYLENGTH(it)) == 0){
            r = p;
            break;
        }
        p = (Item*)((char*)p + p->length);
    }
    free_data(node, data);
    return r;
}

inline int hex2int(char b)
{
    if (('0'<=b && b<='9') || ('a'<=b && b<='f')) {
        return (b>='a') ?  (b-'a'+10) : (b-'0');
    }else{
        return -1;
    }
}

static uint16_t get_node_hash(HTree* tree, Node* node, const char* dir, 
    int *count)
{
    if (node->is_node && strlen(dir) > 0){
        char i = hex2int(dir[0]);
        if (i >= 0) {
            return get_node_hash(tree, get_child(tree, node, i), dir+1, count);
        }else{
            if(count) *count = 0;
            return 0;
        }
    }
    update_node(tree, node);
    if (count) *count = node->count;
    return node->hash;
}

static char* list_dir(HTree *tree, Node* node, const char* dir)
{
    int dlen = strlen(dir); 
    while (node->is_node && dlen > 0){
        int b = hex2int(dir[0]);
        if (b >=0 && b < 16) {
            node = get_child(tree, node, b);
            dir ++;
            dlen --;
        }else{
            return NULL;
        }
    }
    
    int bsize = 4096;
    char *buf = (char*) malloc(bsize);
    memset(buf, 0, bsize);
    int n = 0, i, j;
    if (node->is_node) {
        update_node(tree, node);

        Node *child = get_child(tree, node, 0);
        if (node->count > 128) {
            for (i=0; i<BUCKET_SIZE; i++) {
                Node *t = child + i;
                n += snprintf(buf + n, bsize - n, "%x/ %u %u\n", 
                            i, t->hash, t->count);
            }           
        }else{
            for (i=0; i<BUCKET_SIZE; i++) {
                char *r = list_dir(tree, child + i, "");
                if (bsize - n < strlen(r)) {
                    buf = (char*)realloc(buf, bsize * 2);
                    bsize *= 2;
                }
                n += sprintf(buf + n, "%s", r);
                free(r);
            }                       
        }
    }else{
        Data *data = get_data(node); 
        Item *it = data->head;
        char pbuf[20], name[255];
        for (i=0; i<data->count; i++, it = (Item*)((char*)it + it->length)){
            if (dlen > 0){
                sprintf(pbuf, "%08x", key_hash(tree,it));
                if (memcmp(pbuf + tree->depth + node->depth, dir, dlen) != 0){
                    continue;
                }
            }
            int l = dc_decode(tree,name, it->name, KEYLENGTH(it));
            n += snprintf(buf+n, bsize-n-1, "%s %u %d\n", name, it->hash, it->ver);
            if (bsize - n < 200) {
                buf = (char*)realloc(buf, bsize * 2);
                bsize *= 2;
            }
        }
        free_data(node, data);
    }
    return buf;
}

static void visit_node(HTree *tree, Node* node, fun_visitor visitor, void* param)
{
    int i;
    if (node->is_node){
        Node *child = get_child(tree, node, 0);
        for (i=0; i<BUCKET_SIZE; i++){
            visit_node(tree, child+i, visitor, param);
        }
    }else{
        Data *data = get_data(node);
        Item *p = data->head;
        Item *it = (Item*)tree->buf;
        for (i=0; i<data->count; i++){
            memcpy(it, p, sizeof(Item));
            dc_decode(tree,it->name, p->name, KEYLENGTH(p));
            it->length = sizeof(Item) + strlen(it->name) - ITEM_PADDING;
            visitor(it, param);
            p = (Item*)((char*)p + p->length);
        }
        free_data(node, data);
    }    
}

static optimize_node(HTree *tree, Node* node)
{
    int i;
    if (node->is_node){
        Node *child = get_child(tree, node, 0);
        for (i=0; i<BUCKET_SIZE; i++){
            optimize_node(tree, child+i);
        }
    }else{
        if (!node->compressed && node->data->size > 64) {
            Data *data = get_data(node);
            node->data = NULL;
            set_data(tree, node, data);
        }
    }    
}

/*
 * API
 */

HTree* ht_new(int depth)
{
    HTree *tree = (HTree*)malloc(sizeof(HTree));
    if (!tree) return NULL;
    memset(tree, 0, sizeof(HTree));
    tree->depth = depth;   
    tree->height = 1;
    tree->compress = false;
    tree->dict = NULL;
    tree->rdict = NULL;

    int pool_size = g_index[tree->height];
    Node *root = (Node*)malloc(sizeof(Node) * pool_size);
    if (!root){
        free(tree);
        return NULL;
    }
    memset(root, 0, sizeof(Node) * pool_size);

    // init depth
    int i,j;
    for (i=0; i<tree->height; i++){
        for (j=g_index[i]; j<g_index[i+1]; j++){
            root[j].depth = i;
        }
    }

    tree->root = root;
    tree->pool_size = pool_size;
    clear(tree, tree->root);

    pthread_mutex_init(&tree->lock, NULL);
    dc_init(tree,NULL);
    
    return tree;
}

void ht_destroy(HTree *tree)
{
    if (!tree) return;

    pthread_mutex_lock(&tree->lock);

    int i;
    for(i=0; i<tree->pool_size; i++){
        if (tree->root[i].data) free(tree->root[i].data);
    }
    free(tree->root);
    free(tree->dict);
    free(tree->rdict);
    free(tree);
}

inline uint32_t keyhash(const char *s)
{
    return fnv1a(s, strlen(s));
}

void ht_add(HTree *tree, const char* name, uint32_t pos, uint16_t hash, int32_t ver)
{
    if (!tree || !name) return;
    if (name[0] <= 32 || strlen(name) > MAX_KEY_LENGTH){
        fprintf(stderr, "bad key\n");
        return;
    }

    pthread_mutex_lock(&tree->lock);
    Item *it = create_item(tree, name, pos, hash, ver);
    add_item(tree, tree->root, it, keyhash(name), true);
    pthread_mutex_unlock(&tree->lock);
}

void ht_remove(HTree* tree, const char *name)
{
    if (!tree || !name) return;
    if (name[0] <= 32 || strlen(name) > MAX_KEY_LENGTH){
        fprintf(stderr, "bad key\n");
        return;
    }

    pthread_mutex_lock(&tree->lock);
    Item *it = create_item(tree, name, 0, 0, 0);
    remove_item(tree, tree->root, it, keyhash(name));
    pthread_mutex_unlock(&tree->lock);
}

Item* ht_get(HTree* tree, const char* key)
{
    if (!tree || !key || key[0] <= 0 || strlen(key) > MAX_KEY_LENGTH) {
        return NULL;
    }
    
    pthread_mutex_lock(&tree->lock);
    Item *it = create_item(tree, key, 0, 0, 0);
    Item *r = get_item_hash(tree, tree->root, it, keyhash(key));
    if (r != NULL){
        Item *rr = (Item*)malloc(sizeof(Item) + strlen(key) + 1);
        memcpy(rr, r, sizeof(Item));
        memcpy(rr->name, key, strlen(key) + 1);
        r = rr; // r is in node->Data block 
    }
    pthread_mutex_unlock(&tree->lock);
    return r;   
}

uint32_t ht_get_hash(HTree* tree, const char* key, int* count)
{
    if (!tree || !key || strlen(key) > 8) {
        if(count) *count = 0;
        return 0;
    }
    
    uint32_t hash = 0;
    pthread_mutex_lock(&tree->lock);

    update_node(tree, tree->root);

    if (key[0] == '@'){
        hash = get_node_hash(tree, tree->root, key+1, count);
    }
    pthread_mutex_unlock(&tree->lock);
    return hash;
}

char* ht_list(HTree* tree, const char* dir)
{
    if (!tree || !dir || strlen(dir) > 8) return NULL;

    pthread_mutex_lock(&tree->lock);
    char* r = list_dir(tree, tree->root, dir);
    pthread_mutex_unlock(&tree->lock);

    return r;
}

void ht_optimize(HTree *tree)
{
    pthread_mutex_lock(&tree->lock);
    tree->compress = true;
    optimize_node(tree, tree->root);
    pthread_mutex_unlock(&tree->lock);

    fprintf(stderr, "%dM bytes saved\n", saved >> 20);
}

void ht_visit(HTree *tree, fun_visitor visitor, void *param)
{
    pthread_mutex_lock(&tree->lock);
    visit_node(tree, tree->root, visitor, param);
    pthread_mutex_unlock(&tree->lock);  
}
