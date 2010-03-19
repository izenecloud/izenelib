#ifndef PERSIST_STL_H
#define PERSIST_STL_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <list>
#include <deque>

#include "persist.h"

NS_IZENELIB_AM_BEGIN


template<class T>
class mapped_list : public std::list<T, izenelib::am::allocator<T> >
{
};

template<class C, class Traits = std::char_traits<C> >
class basic_string : public std::basic_string<C, Traits, izenelib::am::allocator<C> >
{
    void operator=(const std::basic_string<C> &s);
public:
    basic_string() { }
    basic_string(const C*c) : std::basic_string<C, Traits, izenelib::am::allocator<C> >(c) { }

    basic_string(const std::basic_string<C> &s)
    {
        assign(s.begin(), s.end());
    }
    
    basic_string(char* p, size_t sz)
    :std::basic_string<C, Traits, izenelib::am::allocator<C> >(p,sz)
    {
        
    }

    basic_string &operator=(const C *s)
    {
        assign(s);
        return *this;
    }
    // TODO: Other constructors
};

//typedef basic_string<char> string;
//typedef basic_string<wchar_t> wstring;

typedef basic_string<char> mapped_string;
typedef basic_string<wchar_t> mapped_wstring;


template<class T>
class mapped_vector : public std::vector<T, izenelib::am::allocator<T> >
{
    // TODO: constructors
};

template<class T, class L = std::less<T> >
class mapped_set : public std::set<T, L, izenelib::am::allocator<T> >
{
};

template<class T, class L = std::less<T> >
class mapped_multiset : public std::multiset<T, L, izenelib::am::allocator<T> >
{
};

template<class T, class V, class L = std::less<T> >
class mapped_map : public std::map<T, V, L, izenelib::am::allocator<std::pair<T,V> > >
{
};

template<class T, class V, class L = std::less<T> >
class mapped_multimap : public std::multimap<T, V, L, izenelib::am::allocator<std::pair<T,V> > >
{
};

// fixed_string is a fixed-length string
template<int N, class C=char, class L=unsigned char>
class fixed_string
{
    L len;      // The actual length of the string
    C str[N+1]; // The string, with 1 for a null-terminator
public:
    typedef C *iterator;
    typedef const C *const_iterator;

    fixed_string()
    {
        clear();
    }
    fixed_string(const_iterator d)
    {
        assign(d);
    }

    template<int M, class L2>
    fixed_string(const fixed_string<M,C,L2> &b)
    {
        assign(b.begin(), b.end());
    }

    fixed_string<N,C,L> &operator=(const_iterator d)
    {
        assign(d);
        return *this;
    }

    template<int M,class L2>
    fixed_string<N,C,L> &operator=(const fixed_string<M,C,L2> &b)
    {
        assign(b.begin(), b.end());
        return *this;
    }

    iterator begin()
    {
        return str;
    }
    const_iterator begin() const
    {
        return str;
    }

    iterator end()
    {
        return str+len;
    }
    const_iterator end() const
    {
        return str+len;
    }

    const_iterator c_str() const
    {
        return begin();
    }

    size_t size() const
    {
        return len;
    }

    void clear()
    {
        len=0;
        str[len]=0;
    }

    void assign(const_iterator a)
    {
        for (len=0; len<N && *a; ++a, ++len)
            str[len] = *a;

        str[len]=0;  // Zero-terminate
    }

    void assign(const_iterator a, const_iterator b)
    {
        for (len=0; len<N && a!=b; ++a, ++len)
            str[len] = *a;

        str[len]=0; // Zero-terminate
    }

    C &operator[](size_t n)
    {
        return str[n];
    }
    const C&operator[](size_t n) const
    {
        return str[n];
    }

    template<int M,class L2>
    bool operator==(const fixed_string<M,C,L2> &b) const
    {
        if (size() != b.size()) return false;

        for (unsigned p=0; p<size(); ++p)
        {
            if (str[p] != b[p]) return false;
        }
        return true;
    }

    bool operator==(const_iterator b) const
    {
        for (unsigned p=0; p<=size(); ++p)
        {
            if (str[p] != b[p]) return false;
        }

        return true;
    }

    template<int M,class L2>
    bool operator<(const fixed_string<M,C,L2> &b) const
    {
        for (const C* pa = begin(), *pb = b.begin(); *pb; ++pa, ++pb)
        {
            if (*pa < *pb) return true;
            else if (*pa > *pb) return false;
        }

        return false;
    }
};

NS_IZENELIB_AM_END

#endif

