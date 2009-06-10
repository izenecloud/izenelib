/**
* @file        Utilities.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Misc utility functions
*/
#ifndef UTILITIES_H
#define UTILITIES_H

#include <ir/index_manager/utility/system.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

static size_t POW_TABLE[] =
{
    1,2,4,8,16,32,64,128,256,512,
    1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,
    1048576,2097152,4194304,8388608,16777216,33554432,67108864,134217728,268435456,536870912,
    1073741824,2147483648
};

class Utilities
{
public:
    Utilities();
    ~Utilities();
public:
    static int64_t currentTimeMillis();

    static bool dirExists(const char* path);

    static char* getBuffer(std::string& s,size_t len);

    static size_t utf8towcs(wchar_t * wcs,size_t wlen,const char * utf, size_t ulen);

    static size_t utf8towc(wchar_t *wc, const char *utf, size_t ulen);

    static size_t _wcstombs(char *mbs,size_t mlen,const wchar_t *wcs, size_t wlen);

    static size_t _mbstowcs(wchar_t *wcs, size_t wlen,const char *mbs, size_t mlen);

    static size_t wctoutf8(char * utf, const wchar_t wc);

    static size_t wcstoutf8(char * utf,size_t ulen, const wchar_t * wcs, size_t wlen);

    static inline size_t LOG2_UP(size_t val);

    static inline size_t LOG2_DOWN(size_t val);
};

//////////////////////////////////////////////////////////////////////////
///
inline size_t Utilities::LOG2_UP(size_t val)
{
    for (int i = 0;i<31;i++)
    {
        if (val <= POW_TABLE[i] )
            return i;
    }
    return 0;
}
inline size_t Utilities::LOG2_DOWN(size_t val)
{
    for (int i = 1;i<31;i++)
    {
        if (val == POW_TABLE[i] )
            return i;
        else if (val < POW_TABLE[i])
            return i-1;
    }
    return 0;
}


}

NS_IZENELIB_IR_END

#endif
