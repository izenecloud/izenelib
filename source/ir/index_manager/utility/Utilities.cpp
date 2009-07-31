#include <ir/index_manager/utility/Utilities.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

int64_t Utilities::currentTimeMillis()
{
    //todo
    struct timeval tstruct;
    //if (gettimeofday(&tstruct, NULL) < 0)
    {
        return 0;
    }

    return (((uint64_t) tstruct.tv_sec) * 1000) + tstruct.tv_usec / 1000;
}

bool Utilities::dirExists(const char* path)
{
    struct stat64 buf;
    int32_t ret = stat64(path,&buf);
    return ( ret == 0);
}

char* Utilities::getBuffer(string& s,size_t len)
{
    s.resize(len,'\0');
    return (char*)s.data();
}

size_t Utilities::utf8towcs(wchar_t * wcs,size_t wlen,const char * utf, size_t ulen)
{
    char *sp = (char*)utf;
    wchar_t *rp = wcs;
    size_t len = 0;

    while ( ( (!rp) || (rp < wcs + wlen) ) && (sp < utf + ulen) )
    {
        int r = utf8towc(rp,sp,6);
        if ( r == -1 )
            return 0;
        sp += r;
        if (rp)
            rp++;
        len ++;
    }
    return len;
}

#define UTF8_GET(Result, Chars, Count, Mask, Len)	\
        (Result) = (Chars)[0] & (Mask); 				\
        for ((Count) = 1; (Count) < (Len); ++(Count))	\
            {											\
                if (((Chars)[(Count)] & 0xc0) != 0x80)	\
                {										\
                    (Result) = -1;						\
                    break;								\
                }										\
                (Result) <<= 6; 						\
                (Result) |= ((Chars)[(Count)] & 0x3f);	\
            }

size_t Utilities::utf8towc(wchar_t *wc, const char *utf, size_t ulen)
{
    int i, mask = 0;
    int result;
    unsigned char c = (unsigned char) *utf;
    size_t len = 0;

    if (c < 128)
    {
        len = 1;
        mask = 0x7f;
    }
    else if ((c & 0xe0) == 0xc0)
    {
        len = 2;
        mask = 0x1f;
    }
    else if ((c & 0xf0) == 0xe0)
    {
        len = 3;
        mask = 0x0f;
    }
    else if ((c & 0xf8) == 0xf0)
    {
        len = 4;
        mask = 0x07;
    }
    else if ((c & 0xfc) == 0xf8)
    {
        len = 5;
        mask = 0x03;
    }
    else if ((c & 0xfe) == 0xfc)
    {
        len = 6;
        mask = 0x01;
    }
    else
        return 0;
    if (ulen < len)
        return 0;
    UTF8_GET (result, utf,i, mask, (int)len);
    if (wc)
        *wc = (wchar_t)result;
    return len;
}


size_t Utilities::_wcstombs(char *mbs,size_t mlen,const wchar_t *wcs, size_t wlen)
{
    if (!wcs || wlen <= 0)
        return 0;
    mbstate_t state;
    memset( &state, 0, sizeof(state) );
    wchar_t const* tmp = wcs;
    return wcsrtombs(mbs,&tmp,mlen,&state);

}

size_t Utilities::_mbstowcs(wchar_t *wcs, size_t wlen,const char *mbs, size_t mlen)
{
    mbstate_t state;
    memset( &state, 0, sizeof(state) );
    char const* tmp = mbs;
    return mbsrtowcs(wcs,&tmp,wlen,&state);
}

size_t Utilities::wctoutf8(char * utf, const wchar_t wc)
{
    wchar_t c = wc;
    uint8_t len = 0;
    int first;
    int i;

    if (c < 0x80)
    {
        first = 0;
        len = 1;
    }
    else if (c < 0x800)
    {
        first = 0xc0;
        len = 2;
    }
    else if (c < 0x10000)
    {
        first = 0xe0;
        len = 3;
    }
    else if (c < 0x200000)
    {
        first = 0xf0;
        len = 4;
    }
    else if (c < 0x4000000)
    {
        first = 0xf8;
        len = 5;
    }
    else
    {
        first = 0xfc;
        len = 6;
    }

    if (utf)
    {
        for (i = len - 1; i > 0; --i)
        {
            utf[i] = (char)((c & 0x3f) | 0x80);
            c >>= 6;
        }
        utf[0] = c | first;
    }

    return len;
}

size_t Utilities::wcstoutf8(char * utf,size_t ulen, const wchar_t * wcs, size_t wlen)
{
    char *p = utf;
    size_t i = 0;
    if (p)
    {
        while ( (p < utf + ulen) && (i < wlen) )
            p += wctoutf8(p,wcs[i++]);
        return p - utf;
    }
    else
    {
        size_t len = 0;
        while ( i < wlen)
            len += wctoutf8(NULL,wcs[i++]);
        return len;
    }
}

