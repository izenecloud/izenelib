#ifndef P4_H_GUARD
#define p4_H_GUARD

#include <stdlib.h>
#include "s16head.hpp"
#include "unpack.hpp"


#define FRAC 0.10
#define S 16
#define PCHUNK 128


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

static void pack(uint32_t *v, uint32_t b, uint32_t n, uint32_t *w);

static int detailed_p4_encode(uint32_t **w, uint32_t *p, uint32_t num , uint32_t *chunk_size, uint32_t *exception_n)
{
    uint32_t b = cnum[num];
    int p_low;
    uint32_t e_n = 0;

    uint32_t* out = (uint32_t*)malloc(sizeof(uint32_t) * PCHUNK * 2);
    uint32_t* ex = (uint32_t*)malloc(sizeof(uint32_t) * PCHUNK * 2);
    uint32_t* po = (uint32_t*)malloc(sizeof(uint32_t) * PCHUNK * 2);

    uint32_t *_pp, *_ww;

    if (b == 32)
    {
        (*w)[0] = b << 10;
        ++(*w);
        for (uint32_t i = 0; i < PCHUNK ; ++i)  (*w)[i] = p[i];
        *w += PCHUNK;
        *chunk_size = 1 + BLOCK_SIZE;

        free(out);
        free(ex);
        free(po);
        return 0;
    }

    for (uint32_t i = 0; i < PCHUNK ; ++i)
    {
        if ( p[i] >= 1U << b)    //exception
        {
            p_low = p[i] & ((1 << b) - 1);
            out[i] = p_low;
            ex[e_n] = p[i] >> b;
            po[++e_n] = i;
        }
        else
            out[i] = p[i];
    }

    if (1)    // force to pass every time
    {
        /*get the gap of position*/
        for (int j = e_n - 1; j > 0; --j)
        {
            po[j] = po[j] - po[j-1] ;
            po[j] --;
        }

        uint32_t s = b * PCHUNK >> 5;
        (*w)[0] = (num << 10) + e_n;      // record b and number of exceptions into this value, in the other version we pick this value out and did not count it
        ++(*w);
        for (uint32_t i = 0; i < s; ++i)  (*w)[i] = 0;
        pack(out, b, PCHUNK , *w);
        *w += s;

        uint32_t *all_array = (uint32_t*)malloc(sizeof(uint32_t) * PCHUNK * 4) ;
        for (uint32_t j = 0; j < e_n; ++j)
        {
            all_array[j] = po[j];
            all_array[e_n+j] =ex[j];
        }
        for (_pp = all_array, _ww = *w; _pp < &all_array[2 * e_n];)
            s16_encode(&_ww, &_pp, &(all_array[2*e_n]) - _pp);

        *chunk_size = 1 + s + (_ww - *w) ;

        *w += _ww - *w ;

        *exception_n = e_n;

        free(out);
        free(ex);
        free(po);
        free(all_array);
        return e_n;

    }
}


static void pack(uint32_t *v, uint32_t b, uint32_t n, uint32_t *w)
{
    for (uint32_t bp = 0, i = 0; i < n; ++i, bp += b)
    {
        uint32_t wp = bp >> 5;
        int s = 32 - b - (bp & 31);
        if (s >= 0)
            w[wp] |= v[i] << s;
        else
        {
            s = -s;
            w[wp] |= v[i] >> s;
            w[wp + 1] = v[i] << (32 - s);
        }
    }
}

/*modified p4decode */
static const uint32_t *detailed_p4_decode(uint32_t *_p, const uint32_t *_w, uint32_t *all_array, bool delta, bool reverse)
{

    int flag = _w[0];
    _w++;

    uint32_t *_ww,*_pp;
    uint32_t b = flag >> 10 & 31;
    uint32_t e_n = flag & 1023;

    unpack[b](_p, _w);

    b = cnum[b];
    _w += (b * BLOCK_SIZE) >> 5;
    uint32_t _k = 0;
    uint32_t psum = 0;
    if (e_n != 0)
    {
        for (_pp = all_array, _ww = (uint32_t *)_w; _pp < &all_array[e_n * 2];)
        {
            S16_DECODE(_ww, _pp);
        }

        _w += _ww - _w;
        psum = all_array[0];

        for (uint32_t i = 0; i < e_n; ++i)
        {
            _p[psum] += all_array[e_n + i] << b;
            psum += all_array[i + 1] + 1;
        }
    }

    if (delta)
    {
        if (reverse)
        {
            for (int i = BLOCK_SIZE - 2; i >= 0; --i)
            {
                if (_p[i] == 0) continue;
                _p[i] += _p[i + 1];
            }
        }
        else
        {
            for (uint32_t i = 1; i < BLOCK_SIZE && _p[i] != 0; ++i)
            {
                _p[i] += _p[i - 1];
            }
        }
    }

    return(_w);
}

}

NS_IZENELIB_IR_END

#endif
