/*-----------------------------------------------------------------------------
 *  serialize.hpp - A compression serializer for LZ-End
 *
 *  Coding-Style:
 *      emacs) Mode: C, tab-width: 8, c-basic-offset: 8, indent-tabs-mode: nil
 *      vi) tabstop: 8, expandtab
 *
 *  Authors:
 *      Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#ifndef IZENELIB_AM_SUCCINCT_LZEND_SERIALIZE_HPP
#define IZENELIB_AM_SUCCINCT_LZEND_SERIALIZE_HPP

#include <stdint.h>
#include <vector>

#include <zlib.h>
#include <glog/logging.h>

#include "io/BitsReader.hpp"
#include "io/BitsWriter.hpp"

using namespace std;

NS_IZENELIB_AM_BEGIN
namespace succinct
{
namespace lzend
{

#define VEC_TO_ARRAY(vec, p) \
        ({\
            for (uint64_t i = 0;\
                    i < vec.size(); i++)\
                    p[i] = vec[i];\
         })

#define ARRAY_TO_VEC(p, vec, size)\
        ({\
            for (uint64_t i = 0;\
                    i < size; i++)\
                    vec.push_back(p[i]);\
         })

class compressor
{
public:
    static void zlib_encode(
        const vector<uint8_t>& vec,
        uint64_t& size, uint8_t *addr)
    {
        uLongf wsize;

        uint8_t *temp = new uint8_t[vec.size()];
        VEC_TO_ARRAY(vec, temp);

        /* Compressed with gzip */
        if (compress2(addr, &wsize, temp,
                      vec.size(), 9) != Z_OK)
            throw "Can't compress input data";

        size = static_cast<uint64_t>(wsize);

        //delete[] temp;
    }

    static void zlib_decode(
        vector<uint8_t>& vec,
        uint8_t *addr, uint64_t size,
        uint64_t len)
    {
        uLongf wsize;

        uint8_t *temp = new uint8_t[len];

        /* Uncompressed with gzip */
        uncompress(temp, &wsize, addr, size);

        ARRAY_TO_VEC(temp, vec, len);

        delete[] temp;
    }

    static void int_encode(
        const vector<uint32_t>& vec,
        uint64_t& size, uint32_t *addr)
    {
        uint32_t *temp = new uint32_t[vec.size()];

        VEC_TO_ARRAY(vec, temp);

        BitsWriter *wt = new BitsWriter(addr);
        size = wt->N_DeltaArray(temp, vec.size());

        delete[] temp;
        delete wt;
    }

    static void int_decode(
        vector<uint32_t>& vec,
        uint32_t *addr, uint64_t size,
        uint64_t len)
    {
        uint32_t *temp = new uint32_t[len];

        BitsReader *rd = new BitsReader(addr);
        rd->N_DeltaArray(temp, len);

        ARRAY_TO_VEC(temp, vec, len);

        delete[] temp;
    }
};


}
}
NS_IZENELIB_AM_END

#endif

