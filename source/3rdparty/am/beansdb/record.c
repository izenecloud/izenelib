#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "record.h"
//#include "crc32.c"
#include "quicklz.h"
//#include "fnv1a.h"

const int PADDING = 256;
const int32_t COMPRESS_FLAG = 0x00010000;
const int32_t CLIENT_COMPRESS_FLAG = 0x00000010;
const float COMPRESS_RATIO_LIMIT = 0.7;
const int TRY_COMPRESS_SIZE = 1024 * 10;

uint32_t crc32 (uint32_t crc, unsigned char *buf, size_t len);

uint32_t gen_hash(char *buf, int len)
{
    uint32_t hash = len * 97;
    if (len <= 1024){
        hash += fnv1a(buf, len);
    }else{
        hash += fnv1a(buf, 512);
        hash *= 97;
        hash += fnv1a(buf + len - 512, 512);
    }
    return hash;
}

typedef struct hint_record {
    uint32_t ksize:8;
    uint32_t pos:24;
    int32_t version;
    uint16_t hash;
    char name[2]; // allign
} HintRecord;

const int NAME_IN_RECORD = 2;

struct param {
    int size;
    int curr;
    char* buf;
};

void collect_items(Item* it, void* param)
{
    int length = sizeof(HintRecord) + strlen(it->name) + 1 - NAME_IN_RECORD;
    struct param *p = (struct param *)param;
    if (p->size - p->curr < length) {
        p->size *= 2;
        p->buf = (char*)realloc(p->buf, p->size);
    }
    
    HintRecord *r = (HintRecord*)(p->buf + p->curr);
    r->ksize = strlen(it->name);
    r->pos = it->pos >> 8;
    r->version = it->ver;
    r->hash = it->hash;
    memcpy(r->name, it->name, r->ksize + 1);
    
    p->curr += length;
}

void write_file(char *buf, int size, const char* path)
{
    char tmp[255];
    sprintf(tmp, "%s.tmp", path);
    FILE *hf = fopen(tmp, "wb");
    if (NULL==hf){
        fprintf(stderr, "open %s failed\n", tmp);
        return;
    }
    int n = fwrite(buf, 1, size, hf); 
    fclose(hf);

    if (n == size) {
        unlink(path);
        rename(tmp, path);
    }else{
        fprintf(stderr, "write to %s failed \n", tmp);
    }
}

void build_hint(HTree* tree, const char* hintpath)
{
    struct param p;
    p.size = 1024 * 1024;
    p.curr = 0;
    p.buf = malloc(p.size);
    
    ht_visit(tree, collect_items, &p);
    ht_destroy(tree);    

    // compress
    if (strcmp(hintpath + strlen(hintpath) - 4, ".qlz") == 0) {
        char* wbuf = malloc(QLZ_SCRATCH_COMPRESS);
        char* dst = malloc(p.size + 400);
        int dst_size = qlz_compress(p.buf, dst, p.curr, wbuf);
        free(p.buf);
        p.curr = dst_size;
        p.buf = dst;
        free(wbuf);
    }

    write_file(p.buf, p.curr, hintpath);
    free(p.buf);
}

void scanHintFile(HTree* tree, int bucket, const char* path, const char* new_path)
{
    char *addr;
    int fd;
    struct stat sb;
    size_t length;
    
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "open %s failed\n", path);
        return;     
    }

    if (fstat(fd, &sb) == -1 || sb.st_size == 0){
        close(fd);
        return ;
    }
    
    addr = (char*) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED){
        fprintf(stderr, "mmap failed %s\n", path);
        close(fd);
        return;
    }
  
    char *start = addr, *end = addr + sb.st_size;
    if (strcmp(path + strlen(path) - 4, ".qlz") == 0) {
        char wbuf[QLZ_SCRATCH_DECOMPRESS];
        int size = qlz_size_decompressed(addr);
        start = malloc(size);
        int vsize = qlz_decompress(addr, start, wbuf);
        if (vsize < size) {
            fprintf(stderr, "decompress %s failed: %d < %d, remove it\n", path, vsize, size);
            unlink(path);
            exit(1);
        }
        end = start + vsize;
    }
    
    if (new_path != NULL) {
        if (strcmp(new_path + strlen(new_path) - 4, ".qlz") == 0) {
            char* wbuf = malloc(QLZ_SCRATCH_COMPRESS);
            char* dst = malloc(sb.st_size + 400);
            int dst_size = qlz_compress(start, dst, end - start, wbuf);
            write_file(dst, dst_size, new_path);
            free(dst);
            free(wbuf);
        } else {
            write_file(start, end - start, new_path);
        }
    }
    
    char *p = start;
    while (p < end) {
        HintRecord *r = (HintRecord*) p;
        p += sizeof(HintRecord) - NAME_IN_RECORD + r->ksize + 1;
        if (p > end){
            fprintf(stderr, "scan %s: unexpected end, need %ld byte\n", path, p - end);
            break;
        }
        uint32_t pos = (r->pos << 8) | (bucket & 0xff);
        if (strlen(r->name) == r->ksize) {
            ht_add(tree, r->name, pos, r->hash, r->version);
        }else{
            fprintf(stderr, "scan %s: key length not match %d\n", path, r->ksize);
        }
    }
    
    munmap(addr, sb.st_size);
    if (start != addr ) free(start);
    close(fd);
}


char* record_value(DataRecord *r)
{
    char *res = r->value;
    if (res == r->key + r->ksz + 1) {
        // value was alloced in record
        res = malloc(r->vsz);
        memcpy(res, r->value, r->vsz);
    }
    return res;
}

void free_record(DataRecord *r)
{
    if (r == NULL) return;
    if (r->value != NULL && r->free_value) free(r->value);
    free(r);
}

void compress_record(DataRecord *r)
{
    int ksz = r->ksz, vsz = r->vsz; 
    int n = sizeof(DataRecord) - sizeof(char*) + ksz + vsz;
    if (n > PADDING && (r->flag & (COMPRESS_FLAG|CLIENT_COMPRESS_FLAG)) == 0) {
        char *wbuf = malloc(QLZ_SCRATCH_COMPRESS);
        char *v = malloc(vsz + 400);
        if (wbuf == NULL || v == NULL) return ;
        int try_size = vsz > TRY_COMPRESS_SIZE ? TRY_COMPRESS_SIZE : vsz; 
        int vsize = qlz_compress(r->value, v, try_size, wbuf);
        if (try_size < vsz && vsize < try_size * COMPRESS_RATIO_LIMIT){
            try_size = vsz;
            vsize = qlz_compress(r->value, v, try_size, wbuf);
        }
        free(wbuf);
        
        if (vsize > try_size * COMPRESS_RATIO_LIMIT || try_size < vsz) {
            free(v);
            return;
        }
        
        if (r->free_value) {
            free(r->value);
        }
        r->value = v;
        r->free_value = true;
        r->vsz = vsize;
        r->flag |= COMPRESS_FLAG;
    }
}

DataRecord* decompress_record(DataRecord *r)
{
    if (r->flag & COMPRESS_FLAG) {
        char scratch[QLZ_SCRATCH_DECOMPRESS];
        int csize = qlz_size_compressed(r->value);
        if (csize != r->vsz) {
            fprintf(stderr, "broken compressed data: %d != %d, flag=%x\n", csize, r->vsz, r->flag);
            goto DECOMP_END;
        }
        int size = qlz_size_decompressed(r->value);
        char *v = malloc(size);
        if (v == NULL) {
            fprintf(stderr, "malloc(%d)\n", size);
            goto DECOMP_END;
        }
        int ret = qlz_decompress(r->value, v, scratch);
        if (ret < size) {
            fprintf(stderr, "decompress %s failed: %d < %d\n", r->key, ret, size);
            goto DECOMP_END;
        }
        if (r->free_value) {
            free(r->value);
        }
        r->value = v;
        r->free_value = true;
        r->vsz = size;
        r->flag &= ~COMPRESS_FLAG;
    }
    return r;

DECOMP_END:
    free_record(r); 
    return NULL;
}

DataRecord* decode_record(char* buf, int size)
{
    DataRecord *r = (DataRecord *) (buf - sizeof(char*));
    int ksz = r->ksz, vsz = r->vsz;
    if (ksz < 0 || ksz > 200 || vsz < 0 || vsz > 100 * 1024 * 1024){
        fprintf(stderr, "invalid ksz=: %d, vsz=%d\n", ksz, vsz);
        return NULL;
    }
    int need = sizeof(DataRecord) - sizeof(char*) + ksz + vsz;
    if (size < need) {
        fprintf(stderr, "not enough data in buffer: %d < %d\n", size, need);
        return NULL;
    }
    // CRC check ?

    DataRecord *r2 = (DataRecord *) malloc(need + 1 + sizeof(char*));
    memcpy(r2, r, sizeof(DataRecord) + ksz);
    r2->key[ksz] = 0; // c str    
    r2->free_value = false;
    r2->value = r2->key + ksz +1;
    memcpy(r2->value, r->key + ksz, vsz);
        
    return decompress_record(r2);
}

DataRecord* read_record(FILE *f, bool decomp)
{
    DataRecord *r = (DataRecord*) malloc(PADDING + sizeof(char*));
    if (NULL == r) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }
    r->value = NULL;
   
    if (fread(&r->crc, 1, PADDING, f) != PADDING) {
        fprintf(stderr, "read record faied\n");         
        goto READ_END;
    }

    int ksz = r->ksz, vsz = r->vsz;
    if (ksz < 0 || ksz > 200 || vsz < 0 || vsz > 100 * 1024 * 1024){
        fprintf(stderr, "invalid ksz=: %d, vsz=%d\n", ksz, vsz);
        goto READ_END;
    }
   
    int read_size = PADDING - (sizeof(DataRecord) - sizeof(char*)) - ksz;
    if (vsz < read_size) {
        r->value = r->key + ksz + 1;
        memmove(r->value, r->key + ksz, vsz);
    }else{
        r->value = malloc(vsz);
        if (NULL == r->value) {
            fprintf(stderr, "malloc failed\n");
            goto READ_END;
        }
        memcpy(r->value, r->key + ksz, read_size);
        int need = vsz - read_size;
        int ret = 0;
        if (need > 0 && need != (ret=fread(r->value + read_size, 1, need, f))) {
            r->key[ksz] = 0; // c str    
            fprintf(stderr, "read record %s faied: %d < %d @%ld\n", r->key, ret, need, ftell(f)); 
            goto READ_END;
        }
    }
    r->key[ksz] = 0; // c str    

    uint32_t crc = crc32(0, (char*)(&r->tstamp), 
                    sizeof(DataRecord) - sizeof(char*) - sizeof(uint32_t) + ksz);
    crc = crc32(crc, r->value, vsz);
    if (crc != r->crc){
        fprintf(stderr, "%s @%ld crc32 check failed %d != %d\n", r->key, ftell(f), crc, r->crc);
        goto READ_END;
    }

    r->free_value = r->value != r->key + ksz + 1;
    if (decomp) {
        r = decompress_record(r);
    }
    return r;
    
READ_END:
    free_record(r);
    return NULL; 
}

char* encode_record(DataRecord *r, int *size)
{
    compress_record(r);

    int m, n;
    int ksz = r->ksz, vsz = r->vsz;
    int hs = sizeof(char*); // over header
    m = n = sizeof(DataRecord) - hs + ksz + vsz;
    if (n % PADDING != 0) {
        m += PADDING - (n % PADDING);
    }

    char *buf = malloc(m);

    DataRecord *data = (DataRecord*)(buf - hs);
    memcpy(&data->crc, &r->crc, sizeof(DataRecord)-hs);
    memcpy(data->key, r->key, ksz);
    memcpy(data->key + ksz, r->value, vsz);
    data->crc = crc32(0, (char*)&data->tstamp, n - sizeof(uint32_t));
    
    *size = m;    
    return buf;
}

int write_record(FILE *f, DataRecord *r) 
{
    int size;
    char *data = encode_record(r, &size);
    if (fwrite(data, 1, size, f) < size){
        fprintf(stderr, "write %d byte failed\n", size);
        free(data);
        return -1;
    }
    free(data);
    return 0;
}

void scanDataFile(HTree* tree, int bucket, const char* path, const char* hintpath)
{
    if (bucket < 0 || bucket > 255) return;
    
    FILE *df = fopen(path, "rb");
    if (NULL==df){
        fprintf(stderr, "open %s failed\n", path);
        return;
    }
    fprintf(stderr, "scan datafile %s \n", path);
    
    HTree *cur_tree = ht_new(0);
    fseek(df, 0, SEEK_END);
    uint32_t total = ftell(df);
    fseek(df, 0, SEEK_SET);
    uint32_t pos = 0;
    while (pos < total) {
        DataRecord *r = read_record(df, true);
        if (r != NULL) {
            uint16_t hash = gen_hash(r->value, r->vsz);
            if (r->version > 0){
                ht_add(tree, r->key, pos | bucket, hash, r->version);            
            }else{
                ht_remove(tree, r->key);
            }
            ht_add(cur_tree, r->key, pos | bucket, hash, r->version);
            free_record(r);
        }
        
        pos = ftell(df);
        if (pos % PADDING != 0){
            int left = PADDING - (pos % PADDING);
            fseek(df, left, SEEK_CUR);
            pos += left;
        }
    }
    fclose(df);
    build_hint(cur_tree, hintpath);
}

void scanDataFileBefore(HTree* tree, int bucket, const char* path, time_t before)
{
    if (bucket < 0 || bucket > 255) return;
    
    FILE *df = fopen(path, "rb");
    if (NULL == df){
        fprintf(stderr, "open %s failed\n", path);
        return;
    }
    fprintf(stderr, "scan datafile %s before %ld\n", path, before);

    fseek(df, 0, SEEK_END);
    uint32_t total = ftell(df);
    fseek(df, 0, SEEK_SET);
    uint32_t pos = 0;
    while (pos < total) {
        DataRecord *r = read_record(df, true);
        if (r != NULL) {
            if (r->tstamp >= before ){
                break;
            }
            if (r->version > 0){
                uint16_t hash = gen_hash(r->value, r->vsz);
                ht_add(tree, r->key, pos | bucket, hash, r->version);            
            }else{
                ht_remove(tree, r->key);
            }
            free_record(r);
        }
        
        pos = ftell(df);
        if (pos % PADDING != 0){
            int left = PADDING - (pos % PADDING);
            fseek(df, left, SEEK_CUR);
            pos += left;
        }
    }
    
    fclose(df);
}


static int count_deleted_record(HTree* tree, int bucket, const char* path, int *total)
{
    char *addr;
    int fd;
    struct stat sb;
    size_t length;
    
    *total = 0;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "open %s failed\n", path);
        return 0; 
    }

    if (fstat(fd, &sb) == -1 || sb.st_size == 0){
        close(fd);
        return 0;
    }

    addr = (char*) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED){
        fprintf(stderr, "mmap failed %s\n", path);
        close(fd);
        return 0;
    }
  
    char *start = addr, *end = addr + sb.st_size;
    if (strcmp(path + strlen(path) - 4, ".qlz") == 0) {
        char wbuf[QLZ_SCRATCH_DECOMPRESS];
        int size = qlz_size_decompressed(addr);
        start = malloc(size);
        int vsize = qlz_decompress(addr, start, wbuf);
        if (vsize < size) {
            fprintf(stderr, "decompress %s failed: %d < %d, remove it\n", path, vsize, size);
            unlink(path);
            return 0;
        }
        end = start + vsize;
    }
    
    char *p = start;
    int deleted = 0;
    while (p < end) {
        HintRecord *r = (HintRecord*) p;
        p += sizeof(HintRecord) - NAME_IN_RECORD + r->ksize + 1;
        if (p > end){
            fprintf(stderr, "scan %s: unexpected end, need %ld byte\n", path, p - end);
            break;
        }
        (*total) ++;
        Item *it = ht_get(tree, r->name);
        if (it == NULL || it->pos != ((r->pos << 8) | bucket)) {
            deleted ++;
        }
        if (it) free(it);
    }
    
    munmap(addr, sb.st_size);
    if (start != addr) free(start);
    close(fd);

    return deleted;
}

HTree* optimizeDataFile(HTree* tree, int bucket, const char* path, const char* hintpath, int limit) 
{
    if (limit > 0) {
        int all = 0;
        int deleted = count_deleted_record(tree, bucket, hintpath, &all);
        if (deleted <= all * 0.1 && deleted < limit) {
            fprintf(stderr, "only %d records deleted in %d, skip %s\n", deleted, all, path);
            return NULL;
        }
    }

    FILE *df = fopen(path, "rb");
    if (NULL==df){
        fprintf(stderr, "open %s failed\n", path);
        return NULL;
    }
    char tmp[255];
    sprintf(tmp, "%s.tmp", path);
    FILE *new_df = fopen(tmp, "wb");
    if (NULL==new_df){
        fprintf(stderr, "open %s failed\n", tmp);
        fclose(df);
        return NULL;
    }
    
    HTree *cur_tree = ht_new(0);
    fseek(df, 0, SEEK_END);
    uint32_t total = ftell(df);
    fseek(df, 0, SEEK_SET);
    uint32_t pos = 0;
    int deleted = 0;
    while (pos < total) {
        DataRecord *r = read_record(df, false);
        if (r != NULL) {
            Item *it = ht_get(tree, r->key);
            if (it && it->pos  == (pos | bucket) && (limit > 0 || r->version > 0)) {
                if (r->version != it->ver ) {
                    fprintf(stderr, "BUG: version in HTree and datafile not match: %d != %d\n", 
                        it->ver, r->version);
                }
                uint32_t new_pos = ftell(new_df);
                uint16_t hash = it->hash;
                ht_add(cur_tree, r->key, new_pos | bucket, hash, r->version);
                if (write_record(new_df, r) != 0) {
                    ht_destroy(cur_tree);
                    fclose(df);
                    fclose(new_df);
                    return NULL;
                }
            }else{
                deleted ++;
            }
            if (it) free(it);
            free_record(r);
        }
        pos = ftell(df);
        if (pos % PADDING != 0){
            int left = PADDING - (pos % PADDING);
            fseek(df, left, SEEK_CUR);
            pos += left;
        }
    }
    uint32_t deleted_bytes = ftell(df) - ftell(new_df);
    fclose(df);
    fclose(new_df);
    
    unlink(hintpath);
    unlink(path);
    rename(tmp, path);
    fprintf(stderr, "optimize %s complete, %d records deleted, %d bytes came back\n", 
            path, deleted, deleted_bytes);
    return cur_tree;
}

void visit_record(const char* path, RecordVisitor visitor, void *arg1, void *arg2, bool decomp)
{
    FILE *df = fopen(path, "rb");
    if (NULL==df){
        fprintf(stderr, "open %s failed\n", path);
        return;
    }
    fprintf(stderr, "scan datafile %s \n", path);
    
    fseek(df, 0, SEEK_END);
    uint32_t total = ftell(df);
    fseek(df, 0, SEEK_SET);
    uint32_t pos = 0;
    while (pos < total) {
        DataRecord *r = read_record(df, decomp);
        if (r != NULL) {
            bool cont = visitor(r, arg1, arg2);
            if (cont) break;
        }
        
        pos = ftell(df);
        if (pos % PADDING != 0){
            int left = PADDING - (pos % PADDING);
            fseek(df, left, SEEK_CUR);
            pos += left;
        }
    }
    fclose(df);
}
