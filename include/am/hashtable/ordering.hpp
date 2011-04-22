#ifndef ORDERING_HPP
#define ORDERING_HPP

#include <string>

using namespace std;

////////////////////////////////////////////////////////////////////////////
//  Some default hashing functions
////////////////////////////////////////////////////////////////////////////
inline unsigned int hash(register const char * s)
{
    unsigned int h;
    for (h = 0; *s; ) h += h + *s++;
    return h;
}

inline unsigned int hash(string ss)
{
    unsigned int h;
    const char* s = ss.c_str();
    for (h = 0; *s; ) h += h + *s++;
    return h;
}
inline unsigned int hash(char c)
{
    return c;
}
inline unsigned int hash(unsigned char c)
{
    return c;
}
inline unsigned int hash(int i)
{
    return i;
}
inline unsigned int hash(unsigned int i)
{
    return i;
}
inline unsigned int hash(short i)
{
    return i;
}
inline unsigned int hash(unsigned short i)
{
    return i;
}
inline unsigned int hash(long i)
{
    return i;
}
inline unsigned int hash(unsigned long i)
{
    return i;
}
inline unsigned int hash(float d)
{
    return (unsigned int)d;
}
inline unsigned int hash(double d)
{
    return (unsigned int)d;
}
extern unsigned int hash(const char *);

////////////////////////////////////////////////////////////////////////////
//  Some default equality functions
////////////////////////////////////////////////////////////////////////////
inline bool equal(char i, char j)
{
    return i == j;
}
inline bool equal(unsigned char i, unsigned char j)
{
    return i == j;
}
inline bool equal(short i, short j)
{
    return i == j;
}
inline bool equal(unsigned short i, unsigned short j)
{
    return i == j;
}
inline bool equal(int i, int j)
{
    return i == j;
}
inline bool equal(unsigned int i, unsigned int j)
{
    return i == j;
}
inline bool equal(long i, long j)
{
    return i == j;
}
inline bool equal(unsigned long i, unsigned long j)
{
    return i == j;
}
inline bool equal(float a, float b)
{
    return a == b;
}
inline bool equal(double a, double b)
{
    return a == b;
}
inline bool equal(const char * a, const char * b)
{
    return strcmp(a,b) == 0;
}

inline bool equal(string a, string b)
{
    return a.compare(b);
}
////////////////////////////////////////////////////////////////////////////
//  Some default ordering functions
////////////////////////////////////////////////////////////////////////////
inline bool less(long i, long j)
{
    return i < j;
}
inline bool less(short i, short j)
{
    return i < j;
}
inline bool less(int i, int j)
{
    return i < j;
}
inline bool less(double a, double b)
{
    return a < b;
}
inline bool less(const char * a, const char * b)
{
    return strcmp(a,b) < 0;
}
inline bool lesseq(long i, long j)
{
    return i <= j;
}
inline bool lesseq(double a, double b)
{
    return a <= b;
}
inline bool lesseq(const char * a, const char * b)
{
    return strcmp(a,b) <= 0;
}
inline int compare(int i, int j)
{
    return i - j;
}
inline int compare(short i, short j)
{
    return i - j;
}
inline int compare(long i, long j)
{
    return i - j;
}
inline int compare(double a, double b)
{
    return a == b ? 0 : (a > b ? 1 : 0);
}
inline int compare(float a, float b)
{
    return a == b ? 0 : (a > b ? 1 : 0);
}
inline int compare(const char * a, const char * b)
{
    return strcmp(a,b);
}

#endif
