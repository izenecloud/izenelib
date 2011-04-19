#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "codec.h"
//#include "fnv1a.h"

const size_t DICT_SIZE = 128;
const size_t RDICT_SIZE = 128 * 10;

static int dict_used = 1;

void dc_init(HTree* htree, const char* path)
{
    if (htree->dict != NULL) return ;
    
    htree->dict = (Fmt**)malloc(sizeof(char*) * DICT_SIZE);
    memset(htree->dict, 0, sizeof(Fmt*) * DICT_SIZE);
    dict_used = 1;
    
    htree->rdict = (int*)malloc(sizeof(int) * RDICT_SIZE);
    memset(htree->rdict, 0, sizeof(int) * RDICT_SIZE);

    pthread_mutex_init(&htree->dict_lock, NULL);
}

int dc_encode(HTree* htree, char* buf, const char* src, int len)
{
    if (src == NULL || buf == NULL){
        return 0;
    }
    Fmt** dict = htree->dict;
    int* rdict = htree->rdict;
    pthread_mutex_t* lock = &htree->dict_lock;
	
    if (len > 6 && len < 50 && src[0] > 0){
        int m=0;
        char fmt[255];
		bool hex[20];
        char num[20][10];
		const char *p=src, *q=src + len;
		char *dst=fmt;
        while(p<q){
            if (*p == '%' || *p == '@' || *p == ':'  // not supported format
                || *p == '0'){    // no leading '0'
                goto RET;
            }
            if (*p >= '0' && *p <= '9' || *p >= 'a' && *p <= 'f'){
                char *nd = num[m];
				hex[m] = false;
                while(p < q && (*p >= '0' && *p <= '9' || *p >= 'a' && *p <= 'f')) {
					if (*p >= 'a' && *p <= 'f') hex[m] = true;
                    *nd ++ = *p ++;
                }
                *nd = 0;
                if (hex[m] && nd - num[m] >= 4){
					if (nd - num[m] > 8){
						goto RET;
					}
	                *dst ++ = '%';
	                *dst ++ = 'x';
                    m ++;					
				} else if (!hex[m] && nd - num[m] >= 1) {
                    if (nd - num[m] > 10 || nd - num[m] == 10 && num[m][0] >= '2'){
                        goto RET; // larger than int32: 2,415,919,103=0x8fffffff 
                    }
	                *dst ++ = '%';
	                *dst ++ = 'd';
                    m ++;                    
                }else{
                    memcpy(dst, num[m], nd - num[m]);
                    dst += nd - num[m];
                }
            }else{
                *dst ++ = * p++;
            }
        }
        *dst = 0; // ending 0
        int flen = dst - fmt;
        if (m > 0 && m < 4){
            uint32_t h = fnv1a(fmt, flen) % RDICT_SIZE;
            if (rdict[h] == 0){
                pthread_mutex_lock(&lock);
                if (dict_used < DICT_SIZE) {
                    dict[dict_used] = (Fmt*) malloc(sizeof(Fmt) + flen + 1);
                    dict[dict_used]->nargs = m;
                    memcpy(dict[dict_used]->fmt, fmt, flen + 1);
                    fprintf(stderr, "new fmt %d: %s <= %s\n", dict_used, fmt, src);
                    rdict[h] = dict_used;
                    dict_used ++ ;
                    pthread_mutex_unlock(lock);
                    goto FOUND;
                }
                pthread_mutex_unlock(lock);
                fprintf(stderr, "not captched fmt: %s <= %s\n", fmt, src);
				rdict[h] = -1; // not again
            }else if (rdict[h] > 0 && dict[rdict[h]] != NULL 
						&& strcmp(fmt, dict[rdict[h]]->fmt) == 0){
FOUND:          buf[0] = - rdict[h];
                int32_t *args = (int32_t*)(buf + 1), i;
                for (i=0; i<m; i++){
                    args[i] = hex[i] ? strtol(num[i], NULL, 16) : atoi(num[i]);
                }
                return 1 + m * sizeof(int32_t);
            }
        }
    }
RET:
    memcpy(buf, src, len);
    return len;
}

int dc_decode(HTree* htree, char* buf, const char* src, int len)
{
    if (buf == NULL || src == NULL || len == 0){
        return 0;
    }

    Fmt** dict = htree->dict;
    int* rdict = htree->rdict;

    if (src[0] < 0){
        int idx = -src[0];
        Fmt *f = dict[idx];
        assert(f!=NULL);
        assert(len == f->nargs * 4 + 1);
        int32_t* args = (int32_t*)(src + 1);
        int rlen = 0;
        switch(f->nargs){
            case 1: rlen = sprintf(buf, f->fmt, args[0]); break;
            case 2: rlen = sprintf(buf, f->fmt, args[0], args[1]); break;
            case 3: rlen = sprintf(buf, f->fmt, args[0], args[1], args[2]); break;
            default: assert(0);    
        }
        return rlen;
    }
RET1:
    memcpy(buf, src, len);
	buf[len] = 0;
    return len;
}

