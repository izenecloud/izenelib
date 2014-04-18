/**
   @file vector_string.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef VECTOR_STRING_HPP
#define VECTOR_STRING_HPP

#include "converters.h"
#include "types.h"
#include "algo.hpp"

#include <util/hashFunction.h>

// #ifdef __cplusplus
// extern "C"
// {
// #endif
#include "hlmalloc.h"
// #ifdef __cplusplus
// }
// #endif

#include <stdio.h>
#include <cstring>
#include <iterator>
#include <vector>
#include <ostream>
#include <utility>
#include <stdarg.h>


#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#ifndef MS_WIN
#include <am/util/Wrapper.h>
#include <util/izene_serialization.h>
#endif
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
# include <ext/atomicity.h>
#else
# include <bits/atomicity.h>
#endif
//#include <boost/thread.hpp>
#include "ustr_types.h"
using namespace iconvLibrary;

NS_IZENELIB_UTIL_BEGIN

#ifndef ConvertFunction_
#define ConvertFunction_
/**
 * @brief a class which contains converting function offered by iconv library.
 * @details
 * This class contains pointer of converting functions which is described in the iconv library.
 */
class ConvertFunction
{
public:
    int (*convertToUCS)(unsigned int* outputChar,
                        const unsigned char* inputChar, int bufferSize);
    int (*convertFromUCS)(unsigned char* outputChar, unsigned int inputChar,
                          int bufferSize);
};
#endif
/**
 *@class vector_string
 * This string is just like std::string. It adopts vector structure. Its appending, iterator, substr are all faster than std::string's.
 * For mutable string, its operator[] is slower than std::string's. If you use unicode, the template parameter
 * CHAR_TYPE should be uint16_t. If you don't want copy-on-write, just set COPY_ON_WRITE 0. Parameter APPEND_RATE controls
 * the appending rate when it need to be appended. It should be an integer less than 100, repersent how much percent.
 *@brief A string structed as vector.
 **/
template <
class CHAR_TYPE = UCS2Char,
      int COPY_ON_WRITE = 0,
      uint8_t APPEND_RATE = 50
      >
class vector_string
{
public:
    typedef CHAR_TYPE value_type;
    typedef CHAR_TYPE CharT;
    typedef _Atomic_word  ReferT;
    typedef vector_string<CHAR_TYPE, COPY_ON_WRITE, APPEND_RATE> SelfT;
    typedef uint32_t size_t;

    static const size_t npos;// = -1;

    class const_iterator;

    /**
       @class iterator
       @brief it's for iteration along the string.
     */
    class iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {

        friend class const_iterator;
    public:
        iterator(SelfT* obj=NULL, CharT* p=NULL) : p_(p), obj_(obj) {}
        ~iterator() {}

        // The assignment and relational operators are straightforward
        iterator& operator = (const iterator& other)
        {
            obj_ = other.obj_;
            p_ = other.p_;
            return(*this);
        }

        bool operator==(const iterator& other)const
        {
            return(p_ == other.p_);
        }

        bool operator!=(const iterator& other)const
        {
            return(p_ != other.p_);
        }

        bool operator < (const iterator& other)const
        {
            return(p_ < other.p_);
        }

        bool operator > (const iterator& other)const
        {
            return(p_ > other.p_);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        iterator& operator++()
        {
            if (p_ != NULL)
            {
                p_++;

            }
            return(*this);
        }

        iterator operator++(int)
        {
            iterator tmp(*this);
            p_++;
            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        iterator& operator--()
        {
            if (p_ != NULL)
            {
                p_--;

            }
            return(*this);
        }

        iterator operator--(int)
        {
            iterator tmp(*this);
            p_--;
            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        CharT& operator*()
        {
            if (obj_->is_refered())
            {
                uint64_t i = p_ - obj_->str_;
                (*obj_).assign_self();
                p_ = obj_->str_ + i;
            }

            return(*p_);
        }

        iterator operator + (size_t i)const
        {
            iterator tmp(*this);
            tmp.p_ += i;
            return tmp;
        }

        iterator operator - (size_t i)const
        {
            iterator tmp(*this);
            tmp.p_ -= i;
            return tmp;
        }

        uint64_t operator - (const CharT* s) const
        {
            return p_ -s;
        }

    protected:
        CharT* p_;
        SelfT* obj_;
    };


    /**
       @class const_iterator
     */
    class const_iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {
    public:
        const_iterator(const CharT* p=NULL) : p_(p) {}
        const_iterator(const iterator& other)
        {
            p_ = other.p_;
        }

        ~const_iterator() {}

        // The assignment and relational operators are straightforward
        const_iterator& operator=(const const_iterator& other)
        {
            p_ = other.p_;
            return(*this);
        }

        // The assignment and relational operators are straightforward
        const_iterator& operator = (const iterator& other)
        {
            p_ = other.p_;
            return(*this);
        }

        bool operator==(const const_iterator& other) const
        {
            return(p_ == other.p_);
        }

        bool operator < (const const_iterator& other) const
        {
            return(p_ < other.p_);
        }

        bool operator > (const const_iterator& other) const
        {
            return(p_ > other.p_);
        }

        bool operator!=(const const_iterator& other) const
        {
            return(p_ != other.p_);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_iterator& operator++()
        {
            if (p_ != NULL)
            {
                p_++;

            }
            return(*this);
        }

        const_iterator operator++(int)
        {
            const_iterator tmp(*this);
            p_++;
            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_iterator& operator--()
        {
            if (p_ != NULL)
            {
                p_--;

            }
            return(*this);
        }

        const_iterator operator--(int)
        {
            const_iterator tmp(*this);
            p_--;
            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        const CharT& operator*() const
        {
            return(*p_);
        }

        size_t operator - (const CharT* s) const
        {
            return p_ -s;
        }

        const_iterator operator + (size_t i)const
        {
            const_iterator tmp(*this);
            tmp.p_ += i;

            return tmp;
        }

        const_iterator operator - (size_t i)const
        {
            const_iterator tmp(*this);
            tmp.p_ -= i;
            return tmp;
        }

    private:
        const CharT* p_;
    };


    /**
       @class reverse_iterator
       @brief iterator for reverse iteration.
     */
    class const_reverse_iterator;
    class reverse_iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {

        friend class const_reverse_iterator;
    public:
        reverse_iterator(SelfT* obj=NULL, CharT* p=NULL) : p_(p), obj_(obj) {}
        ~reverse_iterator() {}

        // The assignment and relational operators are straightforward
        reverse_iterator& operator=(const reverse_iterator& other)
        {
            p_ = other.p_;
            return(*this);
        }

        bool operator==(const reverse_iterator& other)const
        {
            return(p_ == other.p_);
        }

        bool operator!=(const reverse_iterator& other)const
        {
            return(p_ != other.p_);
        }

        bool operator > (const reverse_iterator& other)const
        {
            return(p_ < other.p_);
        }

        bool operator < (const reverse_iterator& other)const
        {
            return(p_ > other.p_);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        reverse_iterator& operator++()
        {
            if (p_ != NULL)
            {
                p_--;

            }
            return(*this);
        }

        reverse_iterator operator++(int)
        {
            reverse_iterator tmp(*this);
            p_--;
            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        reverse_iterator& operator--()
        {
            if (p_ != NULL)
            {
                p_++;

            }
            return(*this);
        }

        reverse_iterator operator--(int)
        {
            reverse_iterator tmp(*this);
            p_++;
            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        CharT& operator*()
        {
            if (obj_->is_refered())
            {
                uint64_t i = p_ - obj_->str_;
                (*obj_).assign_self();
                p_ = obj_->str_ + i;
            }

            return *p_;
        }

        reverse_iterator operator + (size_t i)
        {
            reverse_iterator tmp(*this);

            tmp.p_ -= i;

            return tmp;
        }

        reverse_iterator operator - (size_t i)
        {
            reverse_iterator tmp(*this);
            tmp.p_ += i;
            return tmp;
        }

        uint64_t operator - (const CharT* s)
        {
            return p_ -s;
        }

    private:
        CharT* p_;
        SelfT* obj_;
    };


    /**
       @class const_reverse_iterator
       @brief const iterator for reverse iteration.
     */
    class const_reverse_iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {
    public:
        const_reverse_iterator(const CharT* p=NULL) : p_(p) {}
        const_reverse_iterator(const reverse_iterator& other)
        {
            p_ = other.p_;
        }

        ~const_reverse_iterator() {}

        // The assignment and relational operators are straightforward
        const_reverse_iterator& operator=(const const_reverse_iterator& other)
        {
            p_ = other.p_;
            return(*this);
        }

        bool operator==(const const_reverse_iterator& other) const
        {
            return(p_ == other.p_);
        }

        bool operator!=(const const_reverse_iterator& other) const
        {
            return(p_ != other.p_);
        }

        bool operator > (const const_reverse_iterator& other)const
        {
            return(p_ < other.p_);
        }

        bool operator < (const const_reverse_iterator& other)const
        {
            return(p_ > other.p_);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_reverse_iterator& operator++()
        {
            if (p_ != NULL)
            {
                p_--;

            }
            return(*this);
        }

        const_reverse_iterator operator++(int)
        {
            const_reverse_iterator tmp(*this);
            p_--;
            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_reverse_iterator& operator--()
        {
            if (p_ != NULL)
            {
                p_++;

            }
            return(*this);
        }

        const_reverse_iterator operator--(int)
        {
            const_reverse_iterator tmp(*this);
            p_++;
            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        const CharT& operator*() const
        {
            return(*p_);
        }


        const_reverse_iterator operator + (size_t i)
        {
            const_reverse_iterator tmp(*this);
            tmp.p_ += i;
            return tmp;
        }

        const_reverse_iterator operator - (size_t i)
        {
            const_reverse_iterator tmp(*this);
            tmp.p_ -= i;
            return tmp;
        }

        uint64_t operator - (const CharT* s)
        {
            return p_ -s;
        }

    private:
        const CharT* p_;
    };


protected:
    mutable size_t length_;//!< String length.
    mutable char* p_; //!< A pointer pointing to string's buffer including reference counter at the first byte.
public:
    mutable CharT* str_;//!< The start position of buffer storing chars.
protected:
    mutable size_t max_size_;//!< The maximum length of chars the buffer can store.
    mutable bool is_attached_;//!< The buffer of string is attached or not.

    static const float append_rate_;
    //static boost::mutex mutex_;

    /**
       @brief add reference by 1
     */
    inline bool refer()
    {
        if (!COPY_ON_WRITE)
            return false;
        //lock
        if (is_attached_)
            return true;

        if (p_!=NULL)
        {
            //boost::mutex::scoped_lock lock(mutex_);
            //if (p_!=NULL)
            {
                if (*(ReferT*)p_==(ReferT)-1)
                {
                    char* p = (char*)HLmemory::hlmalloc(get_total_size(length_));
                    max_size_ = length_+1;
                    memcpy(p + sizeof (ReferT), str_, capacity());
                    str_ = (CharT*)(p+sizeof (ReferT));
                    str_[length_] = '\0';

                    p_ = p;
                    (*(ReferT*)p_) = 1;
                    is_attached_ = false;
                    return true;
                }
                __gnu_cxx::__atomic_add((ReferT*)p_, 1);//(*(ReferT*)p_)++;
            }

        }

        return true;
    }

    /**
       @brief reduce reference count by 1
     */
    inline void derefer() const
    {

        //str_ = NULL;
        //length_ = 0;
        //max_size_ = 0;

        if (!COPY_ON_WRITE)
        {
            if (p_!=NULL)
            {
                HLmemory::hlfree(p_);
                p_ = NULL;
            }

            return ;
        }

        //lock
        if (is_attached_)
        {
            is_attached_ =false;
            p_ = NULL;
            return;
        }

        if (p_!=NULL && *(ReferT*)p_ > 0)
        {
            //boost::mutex::scoped_lock lock(mutex_);
            //if (p_!=NULL && *(ReferT*)p_ > 0)
            //__gnu_cxx::__atomic_add((ReferT*)p_, -1);//  (*(ReferT*)p_)--;
            //if (*(ReferT*)p_== 0)
            if (__sync_add_and_fetch((ReferT*)p_, -1) <= 0)
            {
                //std::cout<<"free me!\n";
                HLmemory::hlfree(p_);
                p_ = NULL;
                //std::cout<<"alright\n";
            }
        }

    }

    /**
       @brief clear the reference count
     */
    inline void clear_reference()const
    {
        if (!COPY_ON_WRITE)
            return ;

        //lock
        if (is_attached_)
            return;

        if (p_!=NULL)
        {
            //boost::mutex::scoped_lock lock(mutex_);
            //if (p_!=NULL)
            (*(ReferT*)p_) = 1;
        }

    }

    /**
       @brief check if it's refered by other strings.
     */
    inline bool is_refered() const
    {
        if (!COPY_ON_WRITE)
            return false;

        //lock
        if (is_attached_)
            return true;

        if (p_!= NULL && *(ReferT*)p_ > 1)
        {
            //boost::mutex::scoped_lock lock(mutex_);
            //if (p_!= NULL && *(ReferT*)p_ > 1)
            return true;
        }

        return false;
    }

    /**
       @brief this would happen when writing a refered string.
     */
    inline void assign_self() const
    {
        if (!is_refered())
            return;

        char* p = (char*)HLmemory::hlmalloc(get_total_size(length_));
        max_size_ = length_+1;
        memcpy(p + sizeof (ReferT), str_, capacity());
        str_ = (CharT*)(p+sizeof (ReferT));
        str_[length_] = '\0';

        derefer();
        p_ = p;
        is_attached_ = false;
        clear_reference();
    }

    /**
       @brief get length of string char.
     */
    static inline size_t getLen(const CharT* s)
    {
        CharT e = '\0';
        size_t i = 0;

        while (s[i]!=e)
        {
            i++;
        }

        return i;
    }

    /**
       @param str_len number of charactors in string.
       @return the size needed to initialize a string of length str_len
     */
    inline size_t get_total_size(size_t str_len)const
    {
        return (str_len+1)*sizeof(CharT) + sizeof(ReferT);
    }

public:
    /**
     * @brief Content is initialized to an empty string.
     **/
    inline explicit vector_string()
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        // p_ = (char*)HLmemory::hlmalloc(get_total_size(1));
//     str_ = (CharT*)(p_+sizeof (ReferT));
//     memcpy(str_, s, sizeof(CharT));

//     max_size_ = length_ +1 ;
//     str_[length_] = '\0';

//     clear_reference();
    }

    /**
     *Content is initialized to a copy of the string object str.
     **/
    inline vector_string ( const SelfT& str )
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        assign(str);
    }

    /**
     * Content is initialized to a copy of a substring of str. The substring is the portion
     *f str that begins at the character position pos and takes up to n characters (it takes
     *less than n if the end of str is reached before).
     **/
    inline vector_string ( const SelfT& str, size_t pos, size_t n = npos )
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        assign(str, pos, n);
    }

    /**
     *Content is initialized to a copy of the string formed by the first n characters in
     *the array of characters pointed by s.
     **/
    inline vector_string ( const CharT * s, size_t n )
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        assign(n, s);
    }

    /**
     *Content is initialized to a copy of the string formed by the null-terminated character
     *sequence (C string) pointed by s. The length of the caracter sequence is determined by
     *the first occurrence of a null character (as determined by traits.length(s)). This
     *version can be used to initialize a string object using a string literal constant.
     **/
    inline vector_string ( const CharT * s )
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        assign(s);
    }

    /**
     *Content is initialized as a string formed by a repetition of character c, n times.
     **/
    inline vector_string ( size_t n, CharT c )
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        assign(n, c);
    }

    /**
     *Content is initialized as a string represented by vector 'v'.
     **/
    inline vector_string (const std::vector<CharT>& v)
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        assign(v);
    }

    /**
     *Content is initialized as a string represented std::string.
     **/
    //inline vector_string (const std::string& str)
    //    :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    //{
    //    assign(str);
    //}


    /**
     *If InputIterator is an integral type, behaves as the sixth constructor version
     *(the one right above this) by typecasting begin and end to call it:
     *
     *string(static_cast<size_t>(begin),static_cast<char>(end));
     *In any other case, the parameters are taken as iterators, and the content is initialized
     *with the values of the elements that go from the element referred by iterator begin to
     *the element right before the one referred by iterator end.
     **/
    template<class InputIterator>
    inline vector_string (InputIterator begin, InputIterator end)
        :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
    {
        assign(begin, end);
    }

    inline ~vector_string()
    {
        derefer();
    }

    /**
     * Get reference counter.
     **/
    inline ReferT getReferCnt() const
    {
        if (p_ != NULL)
            return *(ReferT*)p_;
        return 0;
    }

    /**
     *Sets a copy of the argument as the new content for the string object.
     *The previous content is dropped.
     *The assign member function provides a similar functionality with additional options.
     **/
    inline SelfT& operator= ( const SelfT& str )
    {
        if (p_ == str.p_)
            return *this;
        return this->assign(str);
    }

    inline SelfT& operator= ( const CharT* s )
    {
        return this->assign(s);
    }

    inline SelfT& operator= ( CharT c )
    {
        this->assign(1, c);
        return *this;
    }

    //******************Iterators********************
public:

    /**
     *An iterator to the beginning of the string.
     *The type of this iterator is either string::iterator member type or
     *string::const_iterator member type, which are compiler specific iterator
     *types suitable to iterate through the elements of a string object.
     **/
    inline const_iterator begin() const
    {
        return const_iterator(str_);
    }

    inline iterator begin()
    {
        return iterator(this, str_);
    }

    /**
     *An iterator past the end of the string.
     *The type of this iterator is either string::iterator member type
     *or string::const_iterator member type, which are compiler specific iterator
     *types suitable to iterate through the elements of a string object.
     **/
    inline const_iterator end() const
    {
        return const_iterator(str_+length_);
    }

    inline iterator end()
    {
        return iterator(this, str_+length_);
    }

    /**
     *A reverse iterator to the reverse beginning of the string (i.e., the last character).
     *The type of this iterator is either string::reverse_iterator member type or
     *string::const_reverse_iterator member type, which are compiler specific iterator types
     *suitable to perform a reverse iteration through the elements of a string object.
     **/
    inline reverse_iterator rbegin()
    {
        return reverse_iterator(this, str_+length_-1);
    }

    inline const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(str_+length_-1);
    }

    /**
     *A reverse iterator to the reverse end of the string (i.e., the element right
     *before its first character).
     *The type of this iterator is either string::reverse_iterator member type or
     *string::const_reverse_iterator member type, which are compiler specific iterator
     *types suitable to perform a reverse iteration through the elements of a string object.
     **/
    inline reverse_iterator rend()
    {
        return reverse_iterator(this, str_-1);
    }

    inline const_reverse_iterator rend() const
    {
        return const_reverse_iterator(str_-1);
    }


    //******************Capacity********************
public:

    /**
     *Returns a count of the number of bytes in the string.
     *string::length is an alias of string::size, sometimes, returning different value.
     **/
    inline size_t size() const
    {
        return length_ * sizeof(CharT);
    }

    /**
     *Returns a count of the number of characters in the string.
     **/
    inline size_t length() const
    {
        return length_;
    }

    /**
     *The maximum number of characters a string object can have as its content
     **/
    inline size_t max_size ( ) const
    {
        return (max_size_);
    }

    /**
     *Resizes the string content to n characters.
     *If n is smaller than the current length of the string, the content is reduced
     *to its first n characters, the rest being dropped.
     *If n is greater than the current length of the string, the content is expanded
     *by appending as many instances of the c character as needed to reach a size of n characters.
     *
     *The second version, actually calls: resize(n,char()), so when a string is resized to
     *a greater size without passing a second argument, the new character positions are filled
     *with the default value of a char, which is the null character.
     **/
    void resize (size_t n, CharT c)
    {
        assert (max_size_>=length_);

        if (n == length_ && n+1 == max_size_)
            return;

        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(n));
            if (max_size_ <= n)
                memcpy(p + sizeof (ReferT), str_, capacity());
            else
                memcpy(p + sizeof (ReferT), str_, n*sizeof(CharT));
            str_ = (CharT*)(p+sizeof (ReferT));

            for (size_t i=length_; i<n; i++)
                str_[i] = c;

            derefer();
            p_ = p;
            str_[n] = '\0';
            clear_reference();
            is_attached_ = false;
        }
        else
        {
            if (n!=max_size_)
            {
                p_ = (char*)HLmemory::hlrealloc(p_, get_total_size(n));
                str_ = (CharT*)(p_ + sizeof (ReferT));
            }
            for (size_t i=length_; i<n; i++)
                str_[i] = c;
            str_[n] = '\0';
        }

        length_ = n;
        max_size_ = n+1;
    }

    void resize ( size_t n )
    {
        assert (max_size_>=length_);

        if (n == max_size_)
        {
            length_ = n;
            return;
        }

        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(n));
            if (max_size_ <= n)
                memcpy(p +sizeof (ReferT), str_, capacity());
            else
                memcpy(p +sizeof (ReferT), str_, n*sizeof(CharT));
            str_ = (CharT*)(p+sizeof (ReferT));

            max_size_ = n;
            derefer();
            p_ = p;
            str_[n]='\0';
            clear_reference();
            is_attached_ = false;
        }
        else
        {
            p_ = (char*)HLmemory::hlrealloc(p_, get_total_size(n));
            str_ = (CharT*)(p_ + sizeof (ReferT));
            str_[n]='\0';
        }

        length_ = n;
        max_size_ = n+1;
    }

    /**
     *Returns the size of the allocated storage space in the string object.
     *Notice that the capacity is not necessarily equal to the number of characters
     *that conform the content of the string (this can be obtained with members
     *size or length), but the capacity of the allocated space, which is either
     *equal or greater than this content size.
     *
     *Notice also that this capacity does not suppose a limit to the length of the string.
     *If more space is required to accomodate content in the string object, the capacity
     *is automatically expanded, or can even be explicitly modified by calling member reserve.
     *
     *The real limit on the size a string object can reach is returned by member max_size.
     **/
    inline size_t capacity ( ) const
    {
        return max_size_ * sizeof(CharT);
    }

    /**
     *Requests that the capacity of the allocated storage space in the string be at least res_arg.
     *This can expand or shrink the size of the storage space in the string, although notice
     *that the resulting capacity after a call to this function is not necessarily equal to
     *res_arg but can be either equal or greater than res_arg, therefore shrinking requests may
     *or may not produce an actual reduction of the allocated space in a particular library implementation.
     *In any case, it never trims the string content (for that purposes, see resize or clear, which modify the content).
     **/
    void reserve ( size_t res_arg=0 )
    {
        if (res_arg<= capacity())
            return;

        max_size_ = res_arg/sizeof(CharT)+1;
        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(max_size_-1));
            if (p_ != NULL && str_!=NULL && length_>0)
            {
                memcpy(p + sizeof(ReferT), str_, length_*sizeof(CharT));
            }
            str_ = (CharT*)(p+sizeof (ReferT));

            derefer();
            p_ = p;
            is_attached_ = false;
        }
        else
        {
            if (p_==NULL)
                p_ = (char*)HLmemory::hlmalloc(get_total_size(max_size_-1));
            else
                p_ = (char*)HLmemory::hlrealloc(p_, get_total_size(max_size_-1));
            str_ = (CharT*)(p_+sizeof (ReferT));
        }

        if (max_size_-1 < length_)
            length_ = max_size_-1;

        str_[length_] = '\0';
        clear_reference();
    }

    /**
     *The string content is set to an empty string, erasing any previous content
     *and thus leaving its size at 0 characters.
     **/
    inline void clear()
    {
        derefer();
        length_ = max_size_ = 0;
        is_attached_ = false;
        str_ = NULL;
        p_ = NULL;
    }

    /**
     *Returns whether the string is empty, i.e. whether its size is 0.
     *This function does not modify the content of the string in any way.
     *To clear the content of the string, member clear can be used.
     **/
    inline bool empty ( ) const
    {
        return (length_==0);
    }

    //******************Element access********************
public:

    /**
     *Returns a reference the character at position pos in the string.
     *The function actually returns data()[ pos ].
     *The at member function has the same behavior as this operator function,
     *except that at also performs a range check.
     **/
    CharT& operator[] ( size_t pos )
    {
        assert(pos<length_);
        if (pos > length_)
        {
            if (pos>=max_size_)
            {
                reserve((pos+2)*sizeof(CharT));
                str_[pos] = 0;
                return str_[pos];
            }
            return str_[pos];
        }


        if (!COPY_ON_WRITE)
            return str_[pos];

        //lock
        bool f = *(ReferT*)p_ > 1 || is_attached_;
        if (!f)
            return str_[pos];

        char* p = (char*)HLmemory::hlmalloc(get_total_size(length_));
        memcpy(p + sizeof (ReferT), str_, length_*sizeof(CharT));
        str_ = (CharT*)(p+sizeof (ReferT));
        max_size_ = length_+1;
        str_[length_] = '\0';

        derefer();
        p_ = p;
        is_attached_ = false;
        clear_reference();


        return str_[pos];

    }

    inline const CharT& operator[] ( size_t pos ) const
    {
        assert(pos<length_);
        if (pos>=length_)
            return str_[length_];

        return str_[pos];
    }

    /**
     *Returns the character at position pos in the string.
     *This member function behaves as operator[] , except that at also performs
     *a range check, throwing an assertion.
     **/
    inline const CharT at ( size_t pos ) const
    {
        // if (pos>=length_)
//       return -1;
        assert(pos<length_);
        if (pos >= length_)
            return str_[length_];

        return str_[pos];
    }

    //******************Modifiers********************
public:
    /**
     *Appends a copy of the argument to the string.
     *The new string content is the content existing in the string object before
     *the call followed by the content of the argument.
     *
     *The append member function provides a similar functionality with additional options.
     **/
    inline SelfT& operator+= ( const SelfT& str )
    {
        return append(str);
    }

    /*!
      Append a buffer ended by '/0'
     */
    inline SelfT& operator+= ( const CharT* s )
    {
        size_t len = getLen(s);
        return append(s, len);
    }

    inline SelfT& operator+= ( CharT c )
    {
        return append(1, c);
    }

    /**
     *Appends a copy of str.
     **/
    SelfT& append ( const SelfT& str )
    {
        assert(this != &str);

        size_t app_len = str.length_;

        if (app_len == 0)
            return *this;

        size_t new_len = app_len+length_;
        size_t bufferSize = max_size_;

        if (bufferSize < new_len)
        {
            bufferSize = new_len<<1;
        }
        new_len = bufferSize;

        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(new_len));
            memcpy(p + sizeof(ReferT), str_, size());
            str_ = (CharT*)(p+sizeof (ReferT));
            memcpy(str_ + length_, str.str_, str.size());

            length_ = length_ + str.length();
            max_size_ = new_len+1;
            str_[length_] = '\0';

            derefer();
            p_ = p;
            clear_reference();

            return *this;

        }

        if (app_len + length_ + 1 > max_size_)
        {
            //std::cout<<get_total_size(new_len)<<std::endl;
            p_  = (char*)HLmemory::hlrealloc(p_, get_total_size(new_len));

            str_ = (CharT*)(p_ + sizeof (ReferT));
            max_size_ = new_len+1;
        }

        memcpy(&str_[length_], str.str_, app_len*sizeof(CharT));

        length_ +=  app_len;

        *(str_ + length_) = '\0';
        clear_reference();

        return *this;
    }

    /**
     *Appends a copy of a substring of str. The substring is the portion of str
     *that begins at the character position pos and takes up to n characters
     *(it takes less than n if the end of string is reached before).
     **/
    inline SelfT& append ( const SelfT& str, size_t pos, size_t n=npos )
    {
        return append(str.substr(pos, n));
    }

    /**
     *Appends a copy of the string formed by the first n characters in the array of characters pointed by s.
     **/
    SelfT& append ( const CharT* s, size_t n )
    {
        if (n == 0)
            return *this;

        size_t new_len = (size_t)((length_ + n)* append_rate_);
        // size_t new_len = length_ + n;
//     if (new_len+1>max_size_)
//       new_len += new_len>>1;

        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(new_len));
            memcpy(p + sizeof(ReferT), str_, size());
            str_ = (CharT*)(p+sizeof (ReferT));
            memcpy(str_ + length_, s, n * sizeof (CharT));

            length_ = length_ + n;
            max_size_ = new_len+1;
            str_[length_] = '\0';

            derefer();
            p_ = p;
            clear_reference();

            return *this;
        }

        if ( n + length_+1 > max_size_)
        {
            p_  = (char*)HLmemory::hlrealloc(p_, get_total_size(new_len));
            str_ = (CharT*)(p_ + sizeof (ReferT));
            max_size_ = new_len+1;
        }

        memcpy(str_ + length_, s, n*sizeof(CharT));
        length_ += n;
        str_[length_] = '\0';
        clear_reference();

        return *this;

    }

    /**
     *Appends a copy of the string formed by the null-terminated character
     *sequence (C string) pointed by s. The length of this character sequence
     *is determined by the first ocurrence of a null character (as determined
     *by traits.length(s)).
     **/
    inline SelfT& append ( const CharT* s )
    {
        size_t len = getLen(s);
        return append(s, len);
    }

    /**
     *Appends a string formed by the repetition n times of character c.
     **/
    SelfT& append ( size_t n, CharT c )
    {
        if (n==0)
            return *this;

        size_t new_len = (size_t)((length_ + n)* append_rate_);

        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(new_len));
            memcpy(p + sizeof(ReferT), str_, size());
            str_ = (CharT*)(p+sizeof (ReferT));
            for (size_t i=0; i<n; i++)
                str_[length_+i] = c;

            length_ += n;
            max_size_ = new_len+1;

            str_[length_] = '\0';

            derefer();
            p_ = p;
            clear_reference();

            return *this;
        }

        if ( n + length_+1 > max_size_)
        {
            if (p_!=NULL)
                p_  = (char*)HLmemory::hlrealloc(p_, get_total_size(new_len));
            else
                p_  = (char*)HLmemory::hlmalloc(get_total_size(new_len));

            str_ = (CharT*)(p_ + sizeof (ReferT));
            max_size_ = new_len+1;
        }

        for (size_t i=0; i<n; i++)
            str_[length_+i] = c;

        length_ += n;

        str_[length_] = '\0';
        clear_reference();

        return *this;
    }

    /**
     *If InputIterator is an integral type, behaves as the previous member
     *function version, effectively appending a string formed by the repetition
     *first times of the character equivalent to last.
     *In any other case, the parameters are considered iterators and the
     *content is appended by the elements that go from element referred
     *by iterator first to the element right before the one referred by iterator last.
     **/
    template <class InputIterator>
    SelfT& append ( InputIterator first, InputIterator last )
    {
        for (InputIterator i=first(); i!=last; i++)
            appand(1, *i);

        return *this;
    }

    /**
     *Appends a single character to the string content, increasing its size by one.
     *To append more than one character at a time, refer to either member
     *append or operator+= .
     **/
    inline void push_back ( CharT c )
    {
        append(1, c);
    }

    /**
     *Sets a copy of str as the new content.
     **/
    SelfT& assign ( const SelfT& str )
    {
        //std::cout<<getReferCnt()<<std::endl;
        //assert(this != &str);

        if (str.length()==0)
        {
            clear();
            return *this;
        }

        derefer();

        if (COPY_ON_WRITE)
        {
            p_ = str.p_;

            str_ = str.str_;
            length_ = str.length_;
            is_attached_ = str.is_attached_;
            max_size_ = str.max_size_;

            refer();

            assert(length_<=max_size_);
        }
        else
        {
            p_ = (char*)HLmemory::hlmalloc(get_total_size(str.length()));
            str_ = (CharT*)(p_ + sizeof (ReferT));
            length_ = str.length_;
            max_size_ = length_+1;
            memcpy(str_, str.str_, str.size());
            str_[length_] = '\0';

            is_attached_ = false;
            clear_reference();
        }

        return *this;
    }

    /**
     *Sets a copy of a substring of str as the new content. The substring
     *is the portion of str that begins at the character position pos and
     *takes up to n characters (it takes less than n if the end of str is
     *reached before).
     **/
    SelfT& assign ( const SelfT& str, size_t pos, size_t n )
    {
        assert(this != &str);

        if (n==0 || pos>=str.length())
        {
            clear();
            return *this;
        }

        derefer();
        max_size_ = length_ = 0;
        is_attached_ = false;

        if (n!=npos &&(n>= str.length()||n+pos >= str.length()))
            n = npos;

        if (n == npos)
            length_ = str.length_ - pos;
        else
            length_ = n;

        if (COPY_ON_WRITE)
        {
            p_ = str.p_;

            str_ = str.str_ + pos;
            max_size_ = str.max_size_ - pos;
            is_attached_ = str.is_attached_;
            refer();
        }
        else
        {
            p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
            str_ = (CharT*)(p_+sizeof (ReferT));
            max_size_ = length_+1;

            // for(size_t i=0; i<length_; i++)
//         str_[i] = str[i+pos];
            memcpy(str_, &str[pos], length_*sizeof(CharT));

            str_[length_] = '\0';
            is_attached_ = false;
            clear_reference();
        }

        return *this;
    }


    /**
     *Sets as the new content a copy of the string formed by the first
     *n characters of the array pointed by s.
     **/
    SelfT& assign ( size_t n, const CharT* s )
    {
        if (n==0)
        {
            clear();
            return *this;
        }

        derefer();

        p_ = (char*)HLmemory::hlmalloc(get_total_size(n));
        str_ = (CharT*)(p_+sizeof (ReferT));

        // for(size_t i=0; i<n; i++)
//       str_[i] = s[i];

        memcpy(str_, s, n*sizeof(CharT));

        length_ = n;

        is_attached_ = false;
        max_size_ = length_ +1 ;
        str_[length_] = '\0';

        clear_reference();

        return *this;
    }

    /**
     *Sets a copy of the string formed by the null-terminated character sequence
     *(C string) pointed by s as the new content. The length of the caracter sequence
     *is determined by the first ocurrence of a null character (as determined
     *by traits.length(s)).
     **/
    SelfT& assign ( const CharT* s )
    {
        assert (s !=NULL);

        derefer();

        length_ = getLen(s);
        if (length_==0)
        {
            length_ = max_size_ = 0;
            str_ =  NULL;
            p_ = NULL;
            is_attached_ = false;
            return *this;
        }

        p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
        str_ = (CharT*)(p_+sizeof (ReferT));

        // for(size_t i=0; i<length_; i++)
//       str_[i] = s[i];
        memcpy(str_, s, length_*sizeof(CharT));

        is_attached_ = false;
        max_size_ = length_ + 1;
        str_[length_] = '\0';
        clear_reference();

        return *this;
    }

    /**
     *Sets a string formed by a repetition of character c, n times, as the new content.
     **/
    SelfT& assign ( size_t n, CharT c )
    {
        if (n==0)
        {
            clear();
            return *this;
        }

        derefer();

        length_ = n;
        p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
        str_ = (CharT*)(p_+sizeof (ReferT));

        for(size_t i=0; i<length_; i++)
            str_[i] = c;

        is_attached_ = false;
        max_size_ = length_+1;
        str_[length_] = '\0';
        clear_reference();

        return *this;
    }

    /**
     *Set a string formated by a char vector.
     **/
    SelfT& assign (const std::vector<CharT>& v)
    {
        if (v.size() == 0)
        {
            clear();
            return *this;
        }

        derefer();

        length_ = v.size();
        p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
        str_ = (CharT*)(p_+sizeof (ReferT));

        for(size_t i=0; i<length_; i++)
            str_[i] = v[i];

        is_attached_ = false;
        max_size_ = length_+1;
        str_[length_] = '\0';
        clear_reference();

        return *this;
    }

    /**
     *Set a string formated by std string.
     **/
    //SelfT& assign (const std::string& str)
    //{
    //    if (str.length()==0)
    //    {
    //        clear();
    //        return *this;
    //    }

    //    derefer();

//  //  CharT* s = (CharT*)str.c_str();

    //    length_ = str.length()/sizeof(CharT);
    //    p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
    //    str_ = (CharT*)(p_+sizeof (ReferT));

    //    // for(size_t i=0; i<length_; i++)
//  //     str_[i] = s[i];

    //    memcpy(str_, str.c_str(), length_*sizeof(CharT));

    //    is_attached_ = false;
    //    max_size_ = length_+1;
    //    str_[length_] = '\0';
    //    clear_reference();

    //    return *this;
    //}

    /**
     *If InputIterator is an integral type, behaves as the previous member
     *function version, effectively setting as the new content a string formed
     *by the repetition first times of the character equivalent to last.
     *In any other case, the content is set to the values of the elements that
     *go from element referred to by iterator first to the element right before
     *the one referred to by iterator last.
     **/
    template <class InputIterator>
    SelfT& assign ( InputIterator first, InputIterator last )
    {
        derefer();
        InputIterator i=first;
        if (i!=last)
        {
            assign(1, *i);
            i++;
        }

        for (; i!=last; i++)
            append(1, *i);

        return *this;
    }

    /**
     *Inserts a copy of the entire string object str at character position pos1.
     **/
    SelfT& insert ( size_t pos1, const SelfT& str )
    {
        if (str.length() == 0)
            return *this;

        size_t new_len = (size_t)((length_ + str.length()) * append_rate_);

        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(new_len));
            memcpy(p + sizeof(ReferT), str_, (pos1+1)*sizeof(CharT));
            memcpy(p + sizeof(ReferT) + (pos1+1)*sizeof(CharT), str.str_, str.size());
            memcpy(p + sizeof(ReferT) + (pos1+1 + str.length())*sizeof(CharT), str_+pos1+1, (length_-pos1-1)*sizeof(CharT));
            str_ = (CharT*)(p+sizeof (ReferT));


            length_ = length_ + str.length();
            max_size_ = new_len+1;
            str_[length_] = '\0';

            derefer();
            p_ = p;
            clear_reference();

            return *this;
        }

        if (p_ == NULL)
            return assign(str);

        if (new_len+1 > max_size_)
        {
            max_size_ = new_len+1;
        }

        char* pp = (char*)HLmemory::hlmalloc(get_total_size(new_len));

        memcpy(pp + sizeof(ReferT), str_, (pos1)*sizeof(CharT));

        memcpy(pp + sizeof(ReferT) + (pos1)*sizeof(CharT), str.str_, str.size());
        memcpy(pp + sizeof(ReferT) + (pos1)*sizeof(CharT)+str.size(), str_+pos1, (length_-pos1)*sizeof(CharT));

        length_ += str.length();
        HLmemory::hlfree(p_);
        p_ = pp;
        str_ = (CharT*)(p_ + sizeof (ReferT));
        str_[length_] = '\0';
        is_attached_ = false;
        clear_reference();

        return *this;
    }

    /**
     *Inserts a copy of a substring of str at character position pos1.
     *The substring is the portion of str that begins at the character
     *position pos2 and takes up to n characters (it takes less than n
     *if the end of str is reached before).
     **/
    inline SelfT& insert ( size_t pos1, const SelfT& str, size_t pos2, size_t n )
    {
        return insert(pos1, str.substr(pos2, n));
    }

    /**
     *Inserts at the character position pos1, a copy of the string formed
     *by the first n characters in the array of characters pointed by s.
     **/
    inline SelfT& insert ( size_t pos1, const CharT* s, size_t n)
    {
        SelfT ss(s, n);
        //ss.attach(s, n);
        return insert(pos1, ss);
    }

    /**
     *Inserts at character position pos1, a copy of the string formed by
     *the null-terminated character sequence (C string) pointed by s.
     *The length of this caracter sequence is determined by the first
     *ocurrence of a null character (as determined by traits.length(s)).
     **/
    inline SelfT& insert ( size_t pos1, const CharT* s )
    {
        SelfT ss(s);
        //ss.attach(s);
        return insert(pos1, ss);
    }

    /**
     *Inserts a string formed by a repetition of character c, n times,
     *at the character position pos1.
     **/
    SelfT& insert ( size_t pos1, size_t n, CharT c )
    {
        SelfT ss(n, c);
        return insert(pos1, ss);
    }

    /**
     *Inserts a copy of character c at the position referred by iterator
     *p and returns an iterator referring to this position where it has
     *been inserted.
     **/
    inline iterator insert ( iterator p, CharT c )
    {
        uint64_t i = p - str_;
        assign_self();
        insert(i, 1, c);
        return iterator(this, str_+i+1);
    }

    /**
     *Inserts a string formed by the repetition of character c, n times,
     *at the position referred by iterator p.
     **/
    inline iterator insert ( iterator p, size_t n, CharT c )
    {
        uint64_t i = p - str_;
        assign_self();
        insert(i, n, c);
        return iterator(this, str_+i+1);
    }

    /**
     *Inserts at the internal position referred by p the content made
     *up of the characters that go from the element referred by iterator
     *first to the element right before the one referred by iterator last.
     **/
    template<class InputIterator>
    void insert( iterator p, InputIterator first, InputIterator last )
    {
        uint64_t i = p - str_;
        assign_self();
        SelfT s;
        s.assign(first, last);

        insert(i, s);
    }

    /**
     *Erases a sequence of n characters starting at position pos. Notice
     *that both parameters are optional: with only one argument, the function
     *deletes everything from position pos forwards, and with no arguments,
     *the function deletes the entire string, like member clear.
     **/
    SelfT& erase ( size_t pos = 0, size_t n = npos )
    {

        assign_self();

        if (p_==NULL)
            return *this;

        if (pos ==0 && n>= length_)
        {
            length_ = 0;
            str_[0]= '\0';

            return *this;
        }

        //assert(pos<length_);
        if (pos>=length_)
            return *this;

        if (n >= length_ - pos)
        {
            length_ = pos;
            str_[length_]= '\0';
            return *this;
        }


        for (size_t i=0; i<length_-n-pos; i++)
            str_[pos+i] = str_[pos+n+i];

        length_ -= n;
        str_[length_]= '\0';
        clear_reference();

        return *this;
    }

    /**
     *Erases the character referred by the iterator position.
     *Only one character is affected.
     **/
    inline iterator erase ( iterator position )
    {
        uint64_t i = position - str_;
        erase(i, 1);
        return iterator(this, str_+i);
    }

    /**
     *Erases all the characters between first and last.
     **/
    inline iterator erase ( iterator first, iterator last )
    {
        uint64_t i = first - str_;
        uint64_t j = last - str_;
        erase(i, j-i);
        return iterator(this, str_+i);
    }

    /**
     *Replaces a section of the current string by some other content
     *determined by the arguments passed.
     *For the versions with parameters pos1 and n1, the section replaced
     *begins at character position pos1 and spans for n1 characters
     *within the string.
     *For the versions with iterators i1 and i2, the section replaced
     *is the one formed by the elements between iterator i1 and the element
     *right before iterator i2.
     *The arguments passed to the function determine what is the replacement
     *for this section of the string:
     *The section is replaced by a copy of the entire string object str.
     **/
    SelfT& replace ( size_t pos1, size_t n1,   const SelfT& str )
    {
        if (str.length() == 0)
        {
            erase(pos1, n1);
            return *this;
        }

        size_t new_len = length_-n1+str.length();

        if (is_refered())
        {
            char* p = (char*)HLmemory::hlmalloc(get_total_size(new_len));
            memcpy(p + sizeof(ReferT), str_, pos1*sizeof(CharT));
            memcpy(p + sizeof(ReferT) + pos1*sizeof(CharT), str.str_, str.size());
            memcpy(p + sizeof(ReferT) + (pos1 + str.length())*sizeof(CharT), str_+pos1+n1, (length_-pos1-n1)*sizeof(CharT));
            str_ = (CharT*)(p+sizeof (ReferT));


            length_ = length_ -n1 + str.length();
            max_size_ = new_len+1;
            str_[length_]= '\0';

            derefer();
            p_ = p;
            clear_reference();

            return *this;
        }

        if (p_==NULL)
            return *this;

        memcpy(str_ + pos1, str.str_, str.size());
        for (size_t i=0; i<length_-n1 -pos1; i++)
            str_[pos1+str.length() + i] = str_[pos1+n1+i];

        length_ = new_len;
        str_[length_]= '\0';
        clear_reference();

        return *this;
    }

    /**
     *The section is replaced by a copy of the entire string object str.
     **/
    inline SelfT& replace ( iterator i1, iterator i2, const SelfT& str )
    {
        assert(i1<=i2);
        uint64_t i = i1 - str_;
        uint64_t j = i2 - str_;
        return replace(i, j-i+1, str);
    }

    /**
     *The section is replaced by a copy of a substring of str. The substring
     *is the portion of str that begins at the character position pos2 and
     *takes up to n2 characters (it takes less than n2 if the end of the
     *string is reached before).
     **/
    inline SelfT& replace ( size_t pos1, size_t n1, const SelfT& str, size_t pos2, size_t n2 )
    {
        return replace(pos1, n1, str.substr(pos2, n2));
    }

    /**
    * The section is replaced by a copy of the string formed by the first
    *n2 characters in the array of characters pointed by s.
     **/
    inline SelfT& replace ( size_t pos1, size_t n1,   const char* s, size_t n2 )
    {
        SelfT ss(s, n2);
        //ss.attach(s, n2);
        return replace(pos1, n1, ss);
    }

    /**
    * The section is replaced by a copy of the string formed by the first
    *n2 characters in the array of characters pointed by s.
     **/
    inline SelfT& replace ( iterator i1, iterator i2, const CharT* s, size_t n2 )
    {
        assert(i1<=i2);
        uint64_t i = i1 - str_;
        uint64_t j = i2 - str_;
        return replace(i, j-i+1, s, n2);
    }

    /**
     *The section is replaced by a copy of the string formed by the
     *null-terminated character sequence (C string) pointed by s. The
     *length of this caracter sequence is determined by the first ocurrence
     *of a null character (as determined by traits.length(s)).
     **/
    inline SelfT& replace ( size_t pos1, size_t n1,   const CharT* s )
    {
        SelfT ss(s);
        //ss.attach(s);
        return replace(pos1, n1, ss);
    }

    /**
     *The section is replaced by a copy of the string formed by the
     *null-terminated character sequence (C string) pointed by s. The
     *length of this caracter sequence is determined by the first ocurrence
     *of a null character (as determined by traits.length(s)).
     **/
    inline SelfT& replace ( iterator i1, iterator i2, const CharT* s )
    {
        assert(i1<=i2);
        uint64_t i = i1 - str_;
        uint64_t j = i2 - str_;
        return replace(i, j-i+1, s);
    }

    /**
     *The section is replaced by a repetition of character c, n2 times.
     **/
    inline SelfT& replace ( size_t pos1, size_t n1,   size_t n2, CharT c )
    {
        SelfT ss(n2, c);
        return replace(pos1, n1, ss);
    }

    /**
     *The section is replaced by a repetition of character c, n2 times.
     **/
    inline SelfT& replace ( iterator i1, iterator i2, size_t n2, CharT c )
    {
        assert(i1<=i2);
        uint64_t i = i1 - str_;
        uint64_t j = i2 - str_;
        return replace(i, j-i+1, n2, c);
    }

    /**
     *The section is replaced by the content made up of the characters
     *that go from the element referred by iterator j1 to the element
     *right before the one referred by iterator j2.
     **/
    template<class InputIterator>
    SelfT& replace ( iterator i1, iterator i2, InputIterator j1, InputIterator j2 )
    {
        assert(i1<=i2);
        uint64_t i = i1 - str_;
        uint64_t j = i2 - str_;
        SelfT s1 = substr(i, j-i+1);
        SelfT s2 = substr(j+1);
        assign(s1);
        append<InputIterator>(j1, j2);
        append(s2);

        return *this;
    }

    /**
     *Copies a sequence of characters from the string content to the
     *array pointed by s. This sequence of characters is made of the
     *characters in the string that start at character position pos and
     *span n characters from there.
     *
     *The function does not append a null character after the content copied.
     *To retrieve a temporary c-string value from a string object, a specific
     *member function exists: c_str.
     **/
    inline size_t copy ( CharT* s, size_t n, size_t pos = 0) const
    {
        size_t len = pos+n>length_? length_-pos : n;
        memcpy(s, str_+pos, len*sizeof(CharT));

        return len;
    }

    /**
     *Swaps the contents of the string with those of string object str,
     *such that after the call to this member function, the contents of
     *this string are those which were in str before the call, and the contents
     *of str are those which were in this string.
     **/
    inline void swap ( SelfT& str )
    {
        using std::swap;
        swap(length_, str.length_);
        swap(p_, str.p_);
        swap(str_, str.str_);
        swap(max_size_, str.max_size_);
        swap(is_attached_, str.is_attached_);
        swap(systemEncodingType_, str.systemEncodingType_);
    }

    /**
     *Attach some array s to string. The s is ended by null charactor.
     *That array must exist before this string being destroyed.
     *Otherwise, the buffer pointer will point to nowhere.
     *This is to avoid needless copy actions.
     **/
    SelfT& attach(CharT* s)
    {
        derefer();
        if (COPY_ON_WRITE)
        {
            is_attached_ = true;
            max_size_ = length_ = getLen(s);
            p_ = NULL;
            str_ = s;
        }
        else
        {
            max_size_ = length_ = getLen(s);
            p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
            str_ = (CharT*)(p_ + sizeof (ReferT));
            is_attached_ = false;
            clear_reference();
            memcpy(str_, s, length_*sizeof(CharT));
        }

        return *this;
    }

    /**
     *Attach part of some array s to string. The s is ended by null charactor.
     *That array must exist before this string being destroyed.
     *Otherwise, the buffer pointer will point to nowhere.
     *This is to avoid needless copy actions.
     **/
    SelfT& attach(CharT* s, size_t n)
    {
        derefer();
        if (COPY_ON_WRITE)
        {
            is_attached_ = true;
            max_size_ = length_ = n;
            p_ = NULL;
            str_ = s;
        }
        else
        {
            max_size_ = length_ = n;
            p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
            str_ = (CharT*)(p_ + sizeof (ReferT));
            is_attached_ = false;
            clear_reference();
            memcpy(str_, s, length_*sizeof(CharT));
        }

        return *this;
    }

    //******************String operations********************
public:
    /**
     *Generates a null-terminated sequence of characters (c-string)
     *with the same content as the string object and returns it
     *as a pointer to an array of characters.
     *
     *A terminating null character is automatically appended.
     *
     *The returned array points to an internal location with the
     *required storage space for this sequence of characters plus its
     *terminating null-character, but the values in this array should
     *not be modified in the program and are only granted to remain
     *unchanged until the next call to a non-constant member function
     *of the string object.
     **/
    inline const CharT* c_str ( ) const
    {
        assert(length_<=max_size_);

        if (str_==NULL || str_[length_]== '\0')
            return str_;

        assert(is_refered());
        assign_self();

        return str_;
    }

    /**
     *Returns a pointer to an array of characters with the same
     *content as the string.
     *
     *Notice that no terminating null character is appended
     *(see member c_str for such a functionality).
     *
     *The returned array points to an internal location which
     *should not be modified directly in the program. Its contents
     *are guaranteed to remain unchanged only until the next call
     *to a non-constant member function of the string object.
     **/
    inline const CharT* data() const
    {
        return str_;
    }

    /**
     *Searches the string for the content specified in either str,
     *s or c, and returns the position of the first occurrence in the string.
     *
     *When pos is specified the search only includes characters on or
     *after position pos, ignoring any possible occurrences in previous locations.
     *The search algorithm is KMP.
     *
     *@param str string to be searched for in the object. The entire content
     *of str must be matched in some part of the string to be considered a match.
     *@param s  Array with a sequence of characters.
     *
     *In the second member function version, the size of the content to be matched
     *is only determined by parameter n.
     *In the third version, a null-terminated sequence is expected, and its end is
     *determined by the first occurrence of a null character in it.
     *
     *@param n    Length of sequence of characters to search for.
     *@param c    Individual character to be searched for.
     *@param pos  Position of the first character in the string to be taken into
     *consideration for possible matches. A value of 0 means that the entire
     *string is considered.
     **/
    inline size_t find ( const SelfT& str, size_t pos = 0, StrCompMode caseChk = SM_SENSITIVE ) const
    {
        if (pos == (size_t)-1 || pos>=length_ || length_<str.length())
            return -1;
        /*
            for (size_t i=pos; i<=length_-str.length();++i)
            {
              size_t j=0;
              for (;j<str.length()&&j+i<length_&&(*this)[i+j]==str[j];++j)
                ;
              if (j==str.length())
                return i;
            }

            return -1;
        */
        SelfT tmp = this->substr(pos);

        size_t i = Algorithm<SelfT>::KMP(tmp, str, caseChk);
        if (i == (size_t)-1)
            return -1;

        return pos + i;
    }

    inline size_t find ( const CharT* s, size_t pos, size_t n ) const
    {
        SelfT ss(s, n);
        //ss.attach(s, n);

        return find(ss, pos);
    }

    inline size_t find ( const CharT* s, size_t pos = 0 ) const
    {
        SelfT ss(s);
        //ss.attach(s);

        return find(ss, pos);
    }

    size_t find ( CharT c, size_t pos = 0 ) const
    {
        for (size_t i =pos; i<length_; i++)
            if (str_[i]==c)
                return i;
        return -1;
    }

    /**
     *Searches the string for the content specified in either str, s or c,
     *and returns the position of the last occurrence in the string.
     *
     *When pos is specified, the search only includes characters between
     *the beginning of the string and position pos, ignoring any possible
     *occurrences after pos. Search algorithm is KMP.
     *
     *@param str string to be searched for in the object. The entire content
     *of str must be matched in some part of the string to be considered a match.
     *@param s    Array with a sequence of characters.
     *
     *In the second member function version, the size of the content to be
     *matched is only determined by parameter n.
     *In the third version, a null-terminated sequence is expected, and its end
     *is determined by the first occurrence of a null character in it.
     *@param n    Length of sequence of characters to search for.
     *@param c    Individual character to be searched for.
     *@param pos  Position of the last character in the string to be taken into
     *consideration for possible matches. The default value npos indicates that
     *the entire string is searched.
    **/
    inline size_t rfind ( const SelfT& str, size_t pos = npos ) const
    {
        assert(pos == npos || pos<length_);

        return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), str);
    }

    inline size_t rfind ( const char* s, size_t pos, size_t n ) const
    {
        assert(pos == npos || pos<length_);

        SelfT ss(s, n);
        //ss.attach(s, n);
        return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), ss);
    }

    inline size_t rfind ( const CharT* s, size_t pos = npos ) const
    {
        assert(pos == npos || pos<length_);

        SelfT ss(s);
        //ss.attach(s);
        return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), ss);
    }

    size_t rfind ( CharT c, size_t pos = npos ) const
    {
        assert(pos == npos || pos<length_);
        for (size_t i = (pos!=npos? pos+1: npos); i!=-1; i--)
            if (str_[i] == c)
                return i;
        return -1;
    }


//   size_t find_first_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_first_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_first_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_first_of ( char c, size_t pos = 0 ) const{}


//   size_t find_last_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_last_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_last_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_last_of ( char c, size_t pos = 0 ) const{}


//   size_t find_first_not_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_first_not_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_first_not_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_first_not_of ( char c, size_t pos = 0 ) const{}


//   size_t find_last_not_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_last_not_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_last_not_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_last_not_of ( char c, size_t pos = 0 ) const{}

    /**
     *Returns a string object with its contents initialized to a substring of
     *the current object.
     *
     *This substring is the character sequence that starts at character position
     *pos and has a length of n characters.
     *
     *@param pos    Position of a character in the current string object to be
     *used as starting character for the substring.
     *@param n    Length of the substring.
     *If this value would make the substring to span past the end of the current
     *string content, only those characters until the end of the string are used.
     *npos is a static member constant value with the greatest possible value for
     *an element of type size_t, therefore, when this value is used, all the
     *characters between pos and the end of the string are used as the initialization substring.
     **/
    inline SelfT substr ( size_t pos = 0, size_t n = npos ) const
    {
        return SelfT(*this, pos, n);
    }

    /**
     *Compares the content of this object (or a substring of it, known as
     *compared (sub)string) to the content of a comparing string, which is
     *formed according to the arguments passed.
     *
     *The member function returns 0 if all the characters in the compared
     *contents compare equal, a negative value if the first character that
     *does not match compares to less in the object than in the comparing string,
     *and a positive value in the opposite case.
     *
     *Notice that for string objects, the result of a character comparison depends
     *only on its character code (i.e., its ASCII code), so the result has some
     *limited alphabetical or numerical ordering meaning.
     *
     *For other basic_string class instantitations, the comparison depends on
     *the specific traits::compare function, where traits is one of the class
     *template parameters.
     *
     *@param str    string object with the content to be used as comparing string.
     *@param s    Array with a sequence of characters to be used as comparing string.
     *Except for the last member version, this is a null-terminated character sequence
     *whose length is determined by the first occurrence of a null-character.
     *In the last member version, the length is not determined by any occurrence
     *of null-characters but by parameter n2.
     *@param pos1   Position of the beginning of the compared substring, i.e. the first
     *character in the object (in *this) to be compared against the comparing string.
     *@param n1    Length of the compared substring.
     *@param pos2  Position of a character in object str which is the beginning
     *of the comparing string.
     *@param n2   Length in characters of the comparing string.
     **/
    int compare ( const SelfT& str ) const
    {
        size_t i=0;
        for (; i<length_ && i<str.length(); i++)
        {
            if (str_[i]>str[i])
                return 1;
            if (str_[i]<str[i])
                return -1;
        }

        if (i==length_ && i==str.length())
            return 0;
        if (i== length_)
            return -1;

        return 1;
    }

    /*!
      @param s it must be ended with '\0'
     */
    int compare ( const CharT* s ) const
    {
        size_t len = getLen(s);

        size_t i=0;
        for (; i<length_ && i<len; i++)
        {
            if (str_[i]>s[i])
                return 1;
            if (str_[i]<s[i])
                return -1;
        }

        if (i==length_ && i==len)
            return 0;
        if (i== length_)
            return -1;

        return 1;
    }

    inline int compare ( size_t pos1, size_t n1, const SelfT& str ) const
    {
        return this->substr(pos1, n1).compare(str);
    }

    inline int compare ( size_t pos1, size_t n1, const CharT* s) const
    {
        return this->substr(pos1, n1).compare(s);
    }

    inline int compare ( size_t pos1, size_t n1, const SelfT& str, size_t pos2, size_t n2 ) const
    {
        return this->substr(pos1, n1).compare(str.substr(pos2, n2));
    }

    inline int compare ( size_t pos1, size_t n1, const CharT* s, size_t n2) const
    {
        SelfT str;
        str.attach(s, n2);
        return this->substr(pos1, n1).compare(str);
    }

    /**
     *Turn this string into a char vector.
     **/
    std::vector<CharT> cast_std_vector()
    {
        std::vector<CharT> v;
        v.resize(length_);
        for (size_t i =0; i<length_; i++)
            v.push_back(str_[i]);

        return v;
    }

    /**
     *Turn this string into std::string.
     **/
    inline std::string cast_std_string()
    {
        return std::string((char*)str_, size());

    }


//******************Serialization********************
public:

    /**
     *This is the interface for boost::serialization. Make it serializable by boost.
     **/
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version)  const
    {
        ar & length_;

        ar.save_binary(str_, length_*sizeof(CharT));
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        derefer();

        ar & length_;
        if (length_ == 0)
        {
            max_size_ = length_;
            p_ = NULL;
            str_ =  NULL;
            is_attached_ = false;
            return;
        }

        max_size_ = length_ +1;

        p_ = (char*)HLmemory::hlmalloc(get_total_size(length_));
        str_ = (CharT*)(p_ + sizeof (ReferT));
        is_attached_ = false;

        ar.load_binary(str_, length_*sizeof(CharT));
        str_[length_] = '\0';
        clear_reference();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template<class DataIO> friend
    void DataIO_loadObject(DataIO& dio, vector_string<CHAR_TYPE>& x)
    {
        x.derefer();

        dio & x.length_;
        if (x.length_ == 0)
        {
            x.max_size_ = x.length_;
            x.p_ = NULL;
            x.str_ = NULL;
            x.is_attached_ = false;
            return;
        }

        x.max_size_ = x.length_ +1;

        x.p_ = (char*)HLmemory::hlmalloc(x.get_total_size(x.length_));
        x.str_ = (CharT*)(x.p_ + sizeof (ReferT));
        x.is_attached_ = false;

        dio.ensureRead(x.str_, sizeof(CharT) * x.length_);
        x.str_[x.length_] = '\0';
        x.clear_reference();
    }
    template<class DataIO> friend
    void DataIO_saveObject(DataIO& dio, const vector_string<CHAR_TYPE>& x)
    {
        dio & x.length_;
        dio.ensureWrite(x.str_, sizeof(CharT) * x.length_);
    }


    /**
     *This is for outputing into std::ostream, say, std::cout.
     **/
    friend std::ostream& operator << (std::ostream& os, const SelfT& str)
    {
        for (size_t i =0; i<str.length_; i++)
            os<<(char)str.str_[i];

        return os;
    }

#include "ustr_interface.h"

    friend std::size_t hash_value(const SelfT& vstr)
    {
        return HashFunction<SelfT>::generateHash64(reinterpret_cast<const char *>(vstr.c_str()), vstr.size());
    }

};

#include "UCS2_Table.h"

typedef izenelib::util::vector_string<uint16_t> UString;

#include "ustr_tool.h"

NS_IZENELIB_UTIL_END

template <
class CHAR_TYPE ,
      int COPY_ON_WRITE,
      uint8_t APPEND_RATE
      >
const typename izenelib::util::vector_string<CHAR_TYPE ,COPY_ON_WRITE, APPEND_RATE>::size_t izenelib::util::vector_string<CHAR_TYPE ,COPY_ON_WRITE, APPEND_RATE>::npos = -1;//!< A static member constant value with the greatest possible value for an element of type size_t.


template <
class CHAR_TYPE ,int COPY_ON_WRITE, uint8_t APPEND_RATE
>
const float izenelib::util::vector_string<CHAR_TYPE ,COPY_ON_WRITE, APPEND_RATE>::append_rate_ = (float)APPEND_RATE/100.+1.;

// template <
//   class CHAR_TYPE ,int COPY_ON_WRITE, uint8_t APPEND_RATE
//   >
// boost::mutex vector_string<CHAR_TYPE ,COPY_ON_WRITE, APPEND_RATE>::mutex_;

template <class C, int COW, uint8_t AR>
inline void swap(izenelib::util::vector_string<C, COW, AR>& a,
                 izenelib::util::vector_string<C, COW, AR>& b)
{
    a.swap(b);
}


#endif
