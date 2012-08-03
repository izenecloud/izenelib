/**
   @file bbt_string.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef AVL_STRING_HPP
#define AVL_STRING_HPP

#include <hlmalloc.h>
#include <types.h>
#include <iterator>
#include "algo.hpp"
#include <vector>
#include <ostream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

//using namespace std;

NS_IZENELIB_UTIL_BEGIN

/**
 *@class bbt_string
 * bbt_string stands for Balanced Binary Tree String.
 * This string adopts bbt structure. It is designed for long string.
 * Compared to other strings, it's the fastest one in a lot of aspects,
 *says, insert, append and push_front, but less memory-saving compared
 *to deque string.
 *It has push_front which vector string and std::string don't have.
 *And it's lack of functions like c_str(), data() and attach() since
 *its data structure. If you use unicode, the template parameter
 * CHAR_TYPE should be uint16_t. If you don't want copy-on-write, just set COPY_ON_WRITE 0.
 * BUCKET_BYTES control the size of bucket in tree by bytes, which is as leaf on tree and store charactors.
  *@brief A string structed as blanced binary tree.
 **/
template <
class CHAR_TYPE = char,
      int COPY_ON_WRITE = 1,
      uint64_t BUCKET_BYTES = 1024
      >
class bbt_string
{
public:
    typedef CHAR_TYPE value_type;
    typedef CHAR_TYPE CharT;
    typedef uint32_t  ReferT;
    typedef bbt_string<CHAR_TYPE, COPY_ON_WRITE, BUCKET_BYTES> SelfT;
    typedef std::size_t size_t;

    enum
    {
        BUCKET_LENGTH = BUCKET_BYTES/sizeof(CharT) -1,
        BUCKET_SIZE = BUCKET_BYTES + sizeof(ReferT)
    };


    static const size_t npos;
protected:

    struct BUCKET_
    {
        char p_[BUCKET_SIZE];

        inline BUCKET_()
        {
            *(ReferT*)(p_ + BUCKET_BYTES) = 1;
        }

        inline CharT& operator [] (size_t i)
        {
            assert(i<=BUCKET_LENGTH);
            return *((CharT*)p_ + i);
        }

        inline size_t length()const
        {
            size_t i=0;
            for (; i<BUCKET_LENGTH; i++)
                if (*((CharT*)p_ + i)=='\0')
                    return i;

            assert(false);
        }

        inline void refer()
        {
            *(ReferT*)(p_ + BUCKET_BYTES) += 1;
        }

        inline bool is_refered()
        {
            if (*(ReferT*)(p_ + BUCKET_BYTES)>1)
                return true;
            return false;
        }

        inline void derefer()
        {
            if (*(ReferT*)(p_ + BUCKET_BYTES) != 0)
                *(ReferT*)(p_ + BUCKET_BYTES) -= 1;
        }

        void clear_reference()
        {
            *(ReferT*)(p_ + BUCKET_BYTES) = 1;
        }

        friend std::ostream& operator << (std::ostream& os, const BUCKET_& b)
        {
            os<<b.p_<<"("<<*(ReferT*)(b.p_ + BUCKET_BYTES)<<")";
            return os;
        }

    }
    ;

    struct INDEX_
    {
        size_t len_;
        BUCKET_* bptr_;

        inline INDEX_()
        {
            len_ = 0;
            bptr_ = NULL;
        }

        bool operator == (size_t i) const
        {
            return len_==i;
        }

        bool operator > (size_t i) const
        {
            return len_ > i;
        }

        bool operator < (size_t i) const
        {
            return len_ < i;
        }


        bool operator != (size_t i) const
        {
            return len_ != i;
        }

        friend std::ostream& operator << (std::ostream& os, const INDEX_& b)
        {
            os<<"["<<b.len_<<"]"<<*b.bptr_;
            return os;
        }

    };

public:
    class const_iterator;

    class iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {

        friend class const_iterator;
    public:
        iterator(INDEX_* indice=NULL, size_t ii=0 , size_t bi=0 )
        {
            indice_ = indice;
            ii_ = ii;
            bi_ = bi;
        }

        ~iterator() {}

        // The assignment and relational operators are straightforward
        iterator& operator = (const iterator& other)
        {
            ii_ = other.ii_;
            bi_ = other.bi_;
            indice_ = other.indice_;

            return(*this);
        }

        bool operator==(const iterator& other)const
        {
            return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
        }

        bool operator!=(const iterator& other)const
        {
            return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
        }

        bool operator < (const iterator& other)const
        {
            return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
        }

        bool operator > (const iterator& other)const
        {
            return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        iterator& operator++()
        {
            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(*this);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return *this;

        }

        iterator operator++(int)
        {
            iterator tmp(*this);
            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(tmp);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        iterator& operator--()
        {
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
                bi_ = indice_[ii_].len_-lastl - 1;
            }
            else
                bi_--;

            return(*this);
        }

        iterator operator--(int)
        {
            iterator tmp(*this);
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = (ii_==0? 0: indice_[ii_-1].len_);
                bi_ = indice_[ii_].len_- lastl - 1;
            }
            else
                bi_--;

            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        CharT& operator*()
        {
            assert(indice_[ii_].bptr_!=NULL);

            if (indice_[ii_].bptr_->is_refered())
            {
                BUCKET_* bu = SelfT::new_bucket();
                memcpy(bu, indice_[ii_].bptr_, BUCKET_SIZE);
                indice_[ii_].bptr_->derefer();
                indice_[ii_].bptr_ = bu;
                bu->clear_reference();
            }

            return (*indice_[ii_].bptr_)[bi_];
        }

        iterator operator + (size_t i)const
        {
            iterator tmp (*this);

            for (; i>0; i--)
                ++tmp;

            return tmp;
        }

        iterator operator - (size_t i)const
        {
            iterator tmp(*this);

            for (; i>0; i--)
                --tmp;

            return tmp;
        }

        size_t operator - (const iterator& other) const
        {
            iterator tmp(other);
            size_t i = 0;

            if (indice_ != other.indice_)
                return -1;

            if (*this>other)
                for (; tmp!=*this; i++)
                    ++tmp;

            if (*this<other)
                for (; tmp!=*this; i--)
                    --tmp;

            return i;
        }

    protected:
        size_t bi_;//!< index in bucket
        size_t ii_;//!< indice index
        INDEX_* indice_;
    };


    class const_iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {
    public:
        const_iterator(const INDEX_* indice=NULL, size_t ii=0 , size_t bi=0 )
        {
            indice_ = indice;
            ii_ = ii;
            bi_ = bi;
        }

        const_iterator(const iterator& other)
        {
            ii_ = other.ii_;
            bi_ = other.bi_;
            indice_ = other.indice_;
        }

        ~const_iterator() {}

        // The assignment and relational operators are straightforward
        const_iterator& operator=(const const_iterator& other)
        {
            ii_ = other.ii_;
            bi_ = other.bi_;
            indice_ = other.indice_;

            return(*this);
        }

        // The assignment and relational operators are straightforward
        const_iterator& operator = (const iterator& other)
        {
            ii_ = other.ii_;
            bi_ = other.bi_;
            indice_ = other.indice_;

            return(*this);
        }

        bool operator==(const const_iterator& other) const
        {
            return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
        }

        bool operator < (const const_iterator& other) const
        {
            return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
        }

        bool operator > (const const_iterator& other) const
        {
            return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
        }

        bool operator!=(const const_iterator& other) const
        {
            return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_iterator& operator++()
        {
            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(*this);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp(*this);

            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(tmp);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_iterator& operator--()
        {
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
                bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
            }
            else
                bi_--;

            return(*this);
        }

        const_iterator operator--(int)
        {
            iterator tmp(*this);
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
                bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
            }
            else
                bi_--;

            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        const CharT& operator*() const
        {
            assert(indice_[ii_].bptr_ != NULL);

            return (*indice_[ii_].bptr_)[bi_];
        }

        uint64_t operator - (const const_iterator& other) const
        {
            const_iterator tmp(other);
            size_t i = 0;

            if (indice_ != other.indice_)
                return -1;

            if (*this>other)
                for (; tmp!=*this; i++)
                    ++tmp;

            if (*this<other)
                for (; tmp!=*this; i--)
                    --tmp;

            return i;
        }

        const_iterator operator + (size_t i)const
        {
            const_iterator tmp(*this);

            for (; i>0; i--)
                ++tmp;

            return tmp;
        }

        const_iterator operator - (size_t i)const
        {
            const_iterator tmp(*this);

            for (; i>0; i--)
                --tmp;

            return tmp;
        }

    private:
        size_t bi_;//index in bucket
        size_t ii_;// indice index
        const INDEX_* indice_;
    };


    class const_reverse_iterator;

    class reverse_iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {

        friend class const_reverse_iterator;
    public:
        reverse_iterator(INDEX_* indice, size_t ii=0 , size_t bi=0 )
        {
            indice_ = indice;
            ii_ = ii;
            bi_ = bi;
        }


        ~reverse_iterator() {}

        // The assignment and relational operators are straightforward
        reverse_iterator& operator=(const reverse_iterator& other)
        {
            ii_ = other.ii_;
            bi_ = other.bi_;
            indice_ = other.indice_;

            return(*this);
        }

        bool operator==(const reverse_iterator& other)const
        {
            return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
        }

        bool operator!=(const reverse_iterator& other)const
        {
            return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
        }

        bool operator > (const reverse_iterator& other)const
        {
            return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
        }

        bool operator < (const reverse_iterator& other)const
        {
            return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        reverse_iterator& operator++()
        {
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
                bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
            }
            else
                bi_--;

            return(*this);
        }

        reverse_iterator operator++(int)
        {
            reverse_iterator tmp(*this);
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
                bi_ = indice_[ii_].len_-lastl - 1;
            }
            else
                bi_--;

            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        reverse_iterator& operator--()
        {
            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(*this);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return(*this);
        }

        reverse_iterator operator--(int)
        {
            reverse_iterator tmp(*this);

            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(tmp);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        CharT& operator*()
        {
            assert(indice_[ii_].bptr_ != NULL);

            if (indice_[ii_].bptr_->is_refered())
            {
                BUCKET_* bu = new_bucket();
                memcpy(bu, indice_[ii_].bptr_, BUCKET_SIZE);
                indice_[ii_].bptr_->derefer();
                indice_[ii_].bptr_ = bu;
                bu->clear_reference();
            }

            return (*indice_[ii_].bptr_)[bi_];
        }

        reverse_iterator operator + (size_t i)const
        {
            reverse_iterator tmp(*this);

            for (; i>0; i--)
                --tmp;

            return *this;
        }

        reverse_iterator operator - (size_t i)const
        {
            reverse_iterator tmp(*this);

            for (; i>0; i--)
                ++tmp;

            return tmp;
        }

        size_t operator - (const reverse_iterator& other) const
        {
            reverse_iterator tmp(other);
            size_t i = 0;

            if (indice_ != other.indice_)
                return -1;

            if (*this>other)
                for (; tmp!=*this; i++)
                    ++tmp;

            if (*this<other)
                for (; tmp!=*this; i--)
                    --tmp;

            return i;
        }

    private:
        size_t bi_;//index in bucket
        size_t ii_;// indice index
        INDEX_* indice_;
    };


    class const_reverse_iterator :
        public std::iterator<std::forward_iterator_tag, CharT>
    {
    public:
        const_reverse_iterator(const INDEX_* indice, size_t ii=0 , size_t bi=0 )
        {
            indice_ = indice;
            ii_ = ii;
            bi_ = bi;
        }


        const_reverse_iterator(const reverse_iterator& other)
        {
            ii_ = other.ii_;
            bi_ = other.bi_;
            indice_ = other.indice_;
        }

        ~const_reverse_iterator() {}

        // The assignment and relational operators are straightforward
        const_reverse_iterator& operator=(const const_reverse_iterator& other)
        {
            ii_ = other.ii_;
            bi_ = other.bi_;
            indice_ = other.indice_;
            return(*this);
        }

        bool operator==(const const_reverse_iterator& other) const
        {
            return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
        }

        bool operator!=(const const_reverse_iterator& other) const
        {
            return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
        }

        bool operator > (const const_reverse_iterator& other)const
        {
            return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
        }

        bool operator < (const const_reverse_iterator& other)const
        {
            return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
                   || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_reverse_iterator& operator++()
        {
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
                bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
            }
            else
                bi_--;

            return(*this);
        }

        const_reverse_iterator operator++(int)
        {
            const_reverse_iterator tmp(*this);
            if (indice_ == NULL)
                return(*this);

            if (bi_==0)
            {
                if (ii_!=(size_t)-1)
                    ii_--;

                if (ii_==(size_t)-1)
                    return(*this);

                size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
                bi_ = indice_[ii_].len_ - lastl - 1;
            }
            else
                bi_--;

            return(tmp);
        }

        // Update my state such that I refer to the next element in the
        // SQueue.
        const_reverse_iterator& operator--()
        {
            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(*this);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return(*this);
        }

        const_reverse_iterator operator--(int)
        {
            const_reverse_iterator tmp(*this);
            if (indice_ == NULL)
                return(*this);

            if (ii_==(size_t)-1)
            {
                ii_ = 0;
                bi_ = 0;
                return(tmp);
            }

            bi_++;
            if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
            {
                bi_ = 0;
                ii_++;
            }

            return(tmp);
        }

        // Return a reference to the value in the node.  I do this instead
        // of returning by value so a caller can update the value in the
        // node directly.
        const CharT& operator*() const
        {
            return (*indice_[ii_].bptr_)[bi_];
        }


        const_reverse_iterator operator + (size_t i)const
        {
            const_reverse_iterator tmp(*this);

            for (; i>0; i--)
                --tmp;

            return *this;
        }

        const_reverse_iterator
        operator - (size_t i)const
        {
            const_reverse_iterator tmp(*this);

            for (; i>0; i--)
                ++tmp;

            return tmp;
        }

        size_t operator - (const const_reverse_iterator& other) const
        {
            const_reverse_iterator tmp(other);
            size_t i = 0;

            if (indice_ != other.indice_)
                return -1;

            if (*this>other)
                for (; tmp!=*this; i++)
                    ++tmp;

            if (*this<other)
                for (; tmp!=*this; i--)
                    --tmp;

            return i;
        }

    private:
        size_t bi_;//index in bucket
        size_t ii_;// indice index
        const INDEX_* indice_;
    };


protected:
    INDEX_* indice_;//!< The indice array entry, stores string length ahead, used for find charactor at a certain position.
    size_t idx_len_;//!< Length of indice array which is being used.
    size_t idx_size_;//!< The size of entire indice array.
    size_t length_;//1< String length.

    inline bool refer_all()
    {
        if (!COPY_ON_WRITE)
            return false;

        //lock
        for (size_t i=0; i<idx_len_; i++)
        {
            indice_[i].bptr_->refer();
        }

        return true;
    }

    inline bool refer(size_t i)
    {
        if (!COPY_ON_WRITE)
            return false;

        //lock
        assert(i<idx_len_);
        indice_[i].bptr_->refer();

        return true;
    }

    void derefer_all()
    {
        for (size_t i=0; i<idx_size_; i++)
            derefer(i);

        idx_len_ = length_ = 0;
    }

    void derefer(size_t i)
    {
        //lock
        assert(i<idx_size_);

        if (!COPY_ON_WRITE)
        {
            if (indice_[i].bptr_!=NULL)
                hlfree(indice_[i].bptr_);
            indice_[i].bptr_  = NULL;
            indice_[i].len_ = 0;
            return;
        }

        if (indice_[i].bptr_ == NULL)
            return;

        if (!indice_[i].bptr_->is_refered())
            hlfree(indice_[i].bptr_);
        else
            indice_[i].bptr_->derefer();

        indice_[i].bptr_ = NULL;
    }

    void clear_all_reference()
    {
        if (!COPY_ON_WRITE)
            return ;


        //lock
        for (size_t i=0; i<idx_len_; i++)
        {
            indice_[i].bptr_->clear_reference();
        }

    }

    void clear_reference(size_t i)
    {
        if (!COPY_ON_WRITE)
            return ;

        assert(i<idx_size_);

        //lock
        indice_[i].bptr_->clear_reference();

    }

    inline void clear_indice(size_t i, size_t n)
    {
        for (size_t j=0; j<n; j++)
        {
            indice_[i+j].bptr_ = NULL;
            indice_[i+j].len_ = 0;
        }

    }


    inline size_t getLen(const CharT* s) const
    {
        CharT e = '\0';
        size_t i = 0;

        while (s[i]!=e)
        {
            i++;
        }

        return i;
    }

    static inline BUCKET_* new_bucket()
    {
        BUCKET_* bu = (BUCKET_*)hlmalloc(BUCKET_SIZE);
        bu->clear_reference();
        return bu;
    }

    inline BUCKET_* new_bucket(size_t i)const
    {
        assert(i<idx_len_ && indice_[i].bptr_!=NULL);

        BUCKET_* bu = new_bucket();
        memcpy(bu, indice_[i].bptr_, BUCKET_SIZE);
        bu->clear_reference();

        return bu;
    }

    void duplicate(size_t i)
    {
        BUCKET_* bu = new_bucket();
        memcpy(bu, indice_[i].bptr_, BUCKET_SIZE);
        bu->clear_reference();

        if (COPY_ON_WRITE)
            derefer(i);

        indice_[i].bptr_ = bu;
    }

    size_t duplicate(INDEX_** indice) const
    {
        *indice = (INDEX_*)hlmalloc(idx_len_*sizeof(INDEX_));
        memcpy (*indice, indice_, idx_len_*sizeof(INDEX_));

        for (size_t i=0; i<idx_len_; i++)
        {
            if (COPY_ON_WRITE)
            {
                (*indice)[i].bptr_->refer();
                continue;
            }

            BUCKET_* bu = new_bucket();
            memcpy(bu, (*indice)[i].bptr_, BUCKET_SIZE);
            (*indice)[i].bptr_ = bu;
        }


        return idx_len_;
    }

    size_t duplicate(size_t start, size_t n, INDEX_** indice) const
    {
        assert(start + n <= length_);

        size_t s = binary_search(start, 0, idx_len_-1);
        // std::cout<<start<<" "<<idx_len_<<" "<<s<<std::endl;
//     display();

        size_t e = binary_search(start+n-1, s, idx_len_-1);

        size_t last_s = s==0? 0: indice_[s-1].len_;
        *indice = (INDEX_*)hlmalloc((e-s+1)*sizeof(INDEX_));

        // if entire bucket
        if (start-last_s == 0 && COPY_ON_WRITE)
        {
            (*indice)[0].bptr_ = indice_[s].bptr_;
            if (n >= indice_[s].len_-start)
                (*indice)[0].bptr_->refer();
            else
                (*indice)[0].bptr_ = new_bucket(s);
        }
        else
        {
            (*indice)[0].bptr_ = new_bucket();
            memcpy((*indice)[0].bptr_->p_, indice_[s].bptr_->p_ + (start-last_s)*sizeof(CharT),
                   BUCKET_SIZE-(start-last_s)*sizeof(CharT));
            (*indice)[0].bptr_->clear_reference();
        }

        (*indice)[0].len_ = n>indice_[s].len_-start ? indice_[s].len_-start:n;
        (*(*indice)[0].bptr_)[(*indice)[0].len_] = '\0';
        last_s = indice_[s].len_;
        s++;
        size_t i=1;

        while (s<=e)
        {
            if (COPY_ON_WRITE)
            {
                (*indice)[i].bptr_ =  indice_[s].bptr_;
                if (n-(*indice)[i-1].len_ >= indice_[s].len_-last_s)
                    (*indice)[i].bptr_->refer();
                else
                    (*indice)[i].bptr_ = new_bucket(s);
            }
            else
            {
                (*indice)[i].bptr_ = new_bucket();
                memcpy((*indice)[i].bptr_->p_, indice_[s].bptr_->p_,
                       (indice_[s].len_-last_s)*sizeof(CharT));
                (*indice)[i].bptr_->clear_reference();
            }

            (*indice)[i].len_ = (*indice)[i-1].len_ +
                                (n-(*indice)[i-1].len_ > indice_[s].len_-last_s ? indice_[s].len_-last_s:n-(*indice)[i-1].len_);
            (*(*indice)[i].bptr_)[(*indice)[i].len_-(*indice)[i-1].len_] = '\0';
            last_s = indice_[s].len_;
            s++;
            i++;

        }

        return i;
    }

    size_t binary_search(size_t i, size_t start, size_t end)const
    {
        if (end == start)
            return end;

        assert(start<end);

        size_t mid = (start+end)/2;
        if (i+1 > indice_[mid].len_)
            return binary_search(i, mid+1, end);
        else if (mid==0 || i+1 <= indice_[mid].len_ && i+1 > indice_[mid-1].len_  )
            return mid;
        else if (i+1 < indice_[mid].len_)
            return binary_search(i, start, mid-1);

        return -1;
    }

    bool is_refered(size_t i)
    {
        //lock
        assert(i<idx_size_);

        if (!COPY_ON_WRITE)
            return false;

        if (indice_[i].bptr_==NULL)
            return false;

        return indice_[i].bptr_->is_refered();

    }

public:
    /**
     * Content is initialized to an empty string.
     **/
    explicit bbt_string()
    {
        idx_len_ = length_ = 0;
        indice_ = (INDEX_*)hlmalloc(BUCKET_LENGTH*sizeof(INDEX_));
        clear_indice(0, BUCKET_LENGTH);

        idx_size_ = BUCKET_LENGTH;
    }

    /**
     *Content is initialized to a copy of the string object str.
     **/
    bbt_string ( const SelfT& str )
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;

        assign(str);
    }

    /**
     * Content is initialized to a copy of a substring of str. The substring is the portion
     *f str that begins at the character position pos and takes up to n characters (it takes
     *less than n if the end of str is reached before).
     **/
    bbt_string ( const SelfT& str, size_t pos, size_t n = npos )
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;
        assign(str, pos, n);
    }

    /**
     *Content is initialized to a copy of the string formed by the first n characters in
     *the array of characters pointed by s.
     **/
    bbt_string ( const CharT * s, size_t n )
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;

        assign(s, n);
    }

    /**
     *Content is initialized to a copy of the string formed by the null-terminated character
     *sequence (C string) pointed by s. The length of the caracter sequence is determined by
     *the first occurrence of a null character (as determined by traits.length(s)). This
     *version can be used to initialize a string object using a string literal constant.
     **/
    bbt_string ( const CharT * s )
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;

        assign(s);
    }

    /**
     *Content is initialized as a string formed by a repetition of character c, n times.
     **/
    bbt_string ( size_t n, CharT c )
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;

        assign(n, c);
    }

    /**
     *Content is initialized as a string represented by vector 'v'.
     **/
    bbt_string (const std::vector<CharT>& v)
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;

        assign(v);
    }

    /**
     *Content is initialized as a string represented std::string.
     **/
    bbt_string (const std::string& str)
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;

        assign(str);
    }

    /**
     *If InputIterator is an integral type, behaves as the sixth constructor version
     *(the one right above this) by typecasting begin and end to call it:
     *
     *string(static_cast<size_t>(begin),static_cast<char>(end));
     *In any other case, the parameters are taken as iterators, and the content is initialized
     *with the values of the elements that go from the element referred by iterator begin to
     *the element right before the one referred by iterator end.
     **/
    template<class InputIterator> bbt_string (InputIterator begin, InputIterator end)
    {
        length_ = idx_len_ = idx_size_ = 0;
        indice_ =NULL;

        assign(begin(), end);
    }

    virtual ~bbt_string()
    {
        clear();
    }

    /**
     *Sets a copy of the argument as the new content for the string object.
     *The previous content is dropped.
     *The assign member function provides a similar functionality with additional options.
     **/
    SelfT& operator= ( const SelfT& str )
    {
        return this->assign(str);
    }

    SelfT& operator= ( const CharT* s )
    {
        return this->assign(s);
    }

    SelfT& operator= ( CharT c )
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
    const_iterator begin() const
    {
        return const_iterator(indice_, 0, 0);
    }

    iterator begin()
    {
        return iterator(indice_, 0, 0);
    }

    /**
     *An iterator past the end of the string.
     *The type of this iterator is either string::iterator member type
     *or string::const_iterator member type, which are compiler specific iterator
     *types suitable to iterate through the elements of a string object.
     **/
    const_iterator end() const
    {
        if (length_==0)
            return begin();

        //return const_iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_));
        return const_iterator(indice_, idx_len_, 0);
    }

    iterator end()
    {
        if (length_==0)
            return begin();

        //return iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_));
        return iterator(indice_, idx_len_, 0);
    }

    /**
     *A reverse iterator to the reverse beginning of the string (i.e., the last character).
     *The type of this iterator is either string::reverse_iterator member type or
     *string::const_reverse_iterator member type, which are compiler specific iterator types
     *suitable to perform a reverse iteration through the elements of a string object.
     **/
    reverse_iterator rbegin()
    {
        return reverse_iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_)-1);
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_)-1);
    }

    /**
     *A reverse iterator to the reverse end of the string (i.e., the element right
     *before its first character).
     *The type of this iterator is either string::reverse_iterator member type or
     *string::const_reverse_iterator member type, which are compiler specific iterator
     *types suitable to perform a reverse iteration through the elements of a string object.
     **/
    reverse_iterator rend()
    {
        if (length_==0)
            return rbegin();

        return reverse_iterator(indice_, -1, 0);
    }

    const_reverse_iterator rend() const
    {
        if (length_==0)
            return rbegin();

        return const_reverse_iterator(indice_, -1, 0);
    }


    //******************Capacity********************
public:

    /**
     *Returns a count of the number of bytes in the string.
     *string::length is an alias of string::size, sometimes, returning different value.
     **/
    size_t size() const
    {
        return length_ * sizeof(CharT);
    }

    /**
     *Returns a count of the number of characters in the string.
     **/
    size_t length() const
    {
        return length_;
    }

    /**
     * Return the length of indice
     **/
    size_t indice_length() const
    {
        return idx_len_;
    }

    /**
     *The maximum number of characters a string object can have as its content
     **/
    size_t max_size ( ) const
    {
        return idx_len_ * BUCKET_LENGTH;
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
        if (n == length_ )
            return;

        if (n < length_)
        {
            size_t i = binary_search(n-1, 0, idx_len_-1);
            size_t il = i==0? 0: indice_[i-1].len_;
            indice_[i].len_ = n;

            duplicate(i);

            (*indice_[i].bptr_)[n-il]= '\0';

            for (size_t j=i+1; j<idx_size_; j++)
                derefer(j);

            idx_len_ = i+1;
            length_ = n;

            return;
        }

        append(n-length_, c);
    }

    void resize ( size_t n )
    {
        if (n == max_size())
            return;

        resize(n, 0);
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
    size_t capacity ( ) const
    {
        size_t i=0;
        //std::cout<<idx_len_<<"-"<<idx_size_<<std::endl;

        for (; i<idx_size_; i++)
            if (indice_[i].bptr_ == NULL)
                break;

        return i * sizeof(CharT) * BUCKET_LENGTH;
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

        size_t n = res_arg/sizeof(CharT);
        n = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0?0:1)-idx_len_;

        assert(n>0);

        if (idx_len_+n > idx_size_)
        {
            indice_ = (INDEX_*)hlrealloc(indice_, (idx_len_+n)*sizeof(INDEX_));
            clear_indice(idx_size_, idx_len_+n-idx_size_);
            idx_size_ = idx_len_+n;
        }

        for (size_t i=0; i<n; i++)
        {
            if (indice_[idx_len_+i].bptr_!=NULL)
            {
                if (is_refered(idx_len_+i))
                    derefer(idx_len_+i);
            }


            if (indice_[idx_len_+i].bptr_ == NULL)
                indice_[idx_len_+i].bptr_ = new_bucket();

            indice_[idx_len_+i].len_ = indice_[idx_len_+i-1].len_;
        }

        //idx_len_ += n;
    }

    /**
     *The string content is set to an empty string, erasing any previous content
     *and thus leaving its size at 0 characters.
     **/
    void clear()
    {
        derefer_all();
        length_ = idx_len_ = idx_size_ = 0;

        if (indice_!=NULL)
            hlfree(indice_);
        indice_ = NULL;
    }

    /**
     *Returns whether the string is empty, i.e. whether its size is 0.
     *This function does not modify the content of the string in any way.
     *To clear the content of the string, member clear can be used.
     **/
    bool empty ( ) const
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
    const CharT& operator[] ( size_t pos ) const
    {
        assert(pos<length_);

        size_t t = binary_search(pos, 0, idx_len_-1);
        size_t l = t==0? 0: indice_[t-1].len_;

        return (*indice_[t].bptr_)[pos-l];
    }

    CharT& operator[] ( size_t pos )
    {
        assert(pos<length_);

        size_t t = binary_search(pos, 0, idx_len_-1);
        size_t l = t==0? 0: indice_[t-1].len_;
        assert(pos>=l);

        if (!COPY_ON_WRITE)
            return (*indice_[t].bptr_)[pos-l];

        bool f = (indice_[t].bptr_!=NULL) && (*(ReferT*)(indice_[t].bptr_->p_ + BUCKET_BYTES)>1);
        if (f)
            duplicate(t);
        return (*indice_[t].bptr_)[pos-l];
        // size_t t = binary_search(pos, 0, idx_len_-1);
//     if (is_refered(t))
//       duplicate(t);

//     size_t l = t==0? 0: indice_[t-1].len_;

//     assert(pos>=l);

//     return (*indice_[t].bptr_)[pos-l];

    }

    /**
     *Returns the character at position pos in the string.
     *This member function behaves as operator[] , except that at also performs
     *a range check, throwing an assertion.
     **/
    const CharT& at ( size_t pos ) const
    {
        assert(pos<length_);

        size_t t = binary_search(pos, 0, idx_len_-1);
        size_t l = t==0? 0: indice_[t-1].len_;

        return (*indice_[t].bptr_)[pos-l];
    }

    CharT& at ( size_t pos )
    {
        assert(pos<length_);

        size_t t = binary_search(pos, 0, idx_len_-1);
        size_t l = t==0? 0: indice_[t-1].len_;
        assert(pos>=l);

        if (!COPY_ON_WRITE)
            return (*indice_[t].bptr_)[pos-l];

        bool f = (indice_[t].bptr_!=NULL) && (*(ReferT*)(indice_[t].bptr_->p_ + BUCKET_BYTES)>1);
        if (f)
            duplicate(t);
        return (*indice_[t].bptr_)[pos-l];
//     assert(pos<length_);
//     size_t t = binary_search(pos, 0, idx_len_-1);
//     if (is_refered(t))
//       duplicate(t);

//     size_t l = t==0? 0: indice_[t-1].len_;

//     return (*indice_[t].bptr_)[pos-l];
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
    SelfT& operator+= ( const SelfT& str )
    {
        return append(str);
    }

    SelfT& operator+= ( const CharT* s )
    {
        size_t len = getLen(s);
        return append(s, len);
    }

    SelfT& operator+= ( CharT c )
    {
        return append(1, c);
    }

    /**
     *Appends a copy of str.
     **/
    SelfT& append ( const SelfT& str )
    {
        if (str.length()==0)
            return *this;

        if (idx_len_>0)
        {
            size_t ei = indice_[idx_len_-1].len_- (idx_len_>1? indice_[idx_len_-2].len_: 0);
            if (ei+str.length() <= BUCKET_LENGTH)
            {
                if (is_refered(idx_len_-1))
                    duplicate(idx_len_-1);
                for (const_iterator i=str.begin(); i!= str.end(); i++, ei++, length_++)
                    (*indice_[idx_len_-1].bptr_)[ei] = *i;
                (*indice_[idx_len_-1].bptr_)[ei] = '\0';
                indice_[idx_len_-1].len_ += str.length();
                return *this;
            }
        }

        //extend indice
        if (str.indice_length()+idx_len_>idx_size_)
        {
            indice_ = (INDEX_*)hlrealloc(indice_, (str.indice_length()+idx_len_)*sizeof(INDEX_));
            clear_indice(idx_size_, idx_len_+str.indice_length()-idx_size_);
            idx_size_ = str.indice_length()+idx_len_;
        }

        for (size_t j=idx_len_; j<idx_size_; j++)
            derefer(j);

        memcpy(indice_+idx_len_, str.indice_, str.indice_length()*sizeof(INDEX_));

        //copy str into
        for (size_t i=idx_len_; i<idx_len_+str.indice_length(); i++)
        {
            if (COPY_ON_WRITE)
            {
                indice_[i].bptr_->refer();
            }
            else
            {
                duplicate(i);
            }

            indice_[i].len_ += length_;
        }

        idx_len_ += str.indice_length();
        length_ += str.length();

        return *this;
    }

    /**
     *Appends a copy of a substring of str. The substring is the portion of str
     *that begins at the character position pos and takes up to n characters
     *(it takes less than n if the end of string is reached before).
     **/
    SelfT& append ( const SelfT& str, size_t pos, size_t n=npos )
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

        size_t k=0;
        if (idx_len_>0)
        {
            size_t ei = indice_[idx_len_-1].len_- (idx_len_>1? indice_[idx_len_-2].len_: 0)-1;
            if (ei+1<BUCKET_LENGTH)
            {
                if (is_refered(idx_len_-1))
                    duplicate(idx_len_-1);

                size_t j=ei+1;
                for (; j<BUCKET_LENGTH && k<n; j++, k++)
                    (*indice_[idx_len_-1].bptr_)[j] = s[k];
                indice_[idx_len_-1].len_ += k;
                (*indice_[idx_len_-1].bptr_)[j] = '\0';
                length_ += k;
            }

            if (k == n)
                return *this;
        }


        size_t ex = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
        //extend indice
        if (ex+idx_len_>idx_size_)
        {
            indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
            clear_indice(idx_size_, ex+idx_len_-idx_size_);
            idx_size_ = ex+idx_len_;
        }

        for (size_t j= idx_len_; k<n; j++, idx_len_++)
        {
            if (is_refered(j))
                derefer(j);
            if (indice_[j].bptr_==NULL)
                indice_[j].bptr_ = new_bucket();


            size_t t=0;
            for (; t<BUCKET_LENGTH && k<n; k++,t++, length_++)
                (*indice_[j].bptr_)[t] = s[k];

            (*indice_[j].bptr_)[t] = '\0';
            indice_[j].len_ = indice_[j-1].len_ + t;
        }

        return *this;

    }

    /**
     *Appends a copy of the string formed by the null-terminated character
     *sequence (C string) pointed by s. The length of this character sequence
     *is determined by the first ocurrence of a null character (as determined
     *by traits.length(s)).
     **/
    SelfT& append ( const CharT* s )
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

        size_t k=0;
        if (idx_len_>0)
        {
            size_t ei = indice_[idx_len_-1].len_- (idx_len_>1? indice_[idx_len_-2].len_: 0)-1;
            if (ei+1<BUCKET_LENGTH)
            {
                if (is_refered(idx_len_-1))
                    duplicate(idx_len_-1);

                size_t j=ei+1;
                for (; j<BUCKET_LENGTH && k<n; j++, k++)
                    (*indice_[idx_len_-1].bptr_)[j] = c;

                indice_[idx_len_-1].len_ += k;
                (*indice_[idx_len_-1].bptr_)[j] = '\0';
                length_ += k;
            }

            if (k == n)
                return *this;
        }

        size_t ex = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
        //extend indice
        if (ex+idx_len_>idx_size_)
        {
            indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
            clear_indice(idx_size_, ex+idx_len_-idx_size_);
            idx_size_ = ex+idx_len_;
        }

        for (size_t j= idx_len_; k<n; j++, idx_len_++)
        {
            if (is_refered(j))
                derefer(j);

            if (indice_[j].bptr_ == NULL)
                indice_[j].bptr_ = new_bucket();

            size_t t=0;
            for (; t<BUCKET_LENGTH && k<n; k++,t++,length_++)
                (*indice_[j].bptr_)[t] = c;

            (*indice_[j].bptr_)[t] = '\0';
            indice_[j].len_ = (j>0? indice_[j-1].len_:0) + t;
        }

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
    void push_back ( CharT c )
    {
        append(1, c);
    }

    /**
     *Inserts a copy of str at the front, increasing its size by str's length.
     **/
    void push_front(const SelfT& str)
    {
        size_t ex = str.indice_length();

        //extend indice
        if (ex+idx_len_>idx_size_)
        {
            indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
            idx_size_ = ex + idx_len_;
        }

        //shift indice
        for (size_t i=idx_len_-1; i!=(size_t)-1; i--)
        {
            indice_[i].len_ += str.length();
            indice_[i+ex] = indice_[i];
        }
        idx_len_ += ex;

        memcpy(indice_, str.indice_, str.indice_length()*sizeof(INDEX_));

        for (size_t i=0; i<str.indice_length(); i++)
        {
            if (COPY_ON_WRITE)
            {
                refer(i);
                continue;
            }

            duplicate(i);
        }

        length_ += str.length();
    }

    /**
     *Sets a copy of str as the new content.
     **/
    SelfT& assign ( const SelfT& str )
    {
        clear();

        idx_size_ = idx_len_ = str.duplicate(&indice_);

        length_ = str.length();

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
        if (n != npos)
            assert(str.length_ >= pos+n);

        clear();

        if (n == npos)
            length_ = str.length_ - pos;
        else
            length_ = n;

        idx_len_ = idx_size_ = str.duplicate(pos, length_, &indice_);

        return *this;
    }

    /**
     *Sets as the new content a copy of the string formed by the first
     *n characters of the array pointed by s.
     **/
    SelfT& assign ( const CharT* s, size_t n )
    {
        clear();

        idx_len_ = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
        idx_size_ = idx_len_ > BUCKET_LENGTH? idx_len_: BUCKET_LENGTH;
        length_ = n;

        indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
        clear_indice(0, idx_size_);

        for (size_t i=0; i<idx_len_; i++)
        {
            indice_[i].bptr_ = new_bucket();
            size_t lastl = i==0?0: indice_[i-1].len_;
            indice_[i].len_ = (n-lastl > BUCKET_LENGTH? BUCKET_LENGTH: n-lastl) + lastl;

            memcpy(indice_[i].bptr_->p_, s+lastl, (indice_[i].len_-lastl)*sizeof(CharT));
            (*indice_[i].bptr_)[indice_[i].len_ - lastl] = '\0';
        }

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
        derefer_all();

        length_ = getLen(s);

        return assign(s, length_);
    }

    /**
     *Sets a string formed by a repetition of character c, n times, as the new content.
     **/
    SelfT& assign ( size_t n, CharT c )
    {
        derefer_all();

        idx_len_ = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
        idx_size_ = idx_len_ > BUCKET_LENGTH? idx_len_: BUCKET_LENGTH;
        length_ = n;

        indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
        memset (indice_, 0, idx_size_*sizeof(INDEX_));

        for (size_t i=0; i<idx_len_; i++)
        {
            indice_[i].bptr_ = new_bucket();
            indice_[i].len_ = n>BUCKET_LENGTH? BUCKET_LENGTH: n;
            n -= indice_[i].len_;

            for (size_t j=0; j<indice_[i].len_; j++)
                (*indice_[i].bptr_)[j] = c;

            (*indice_[i].bptr_)[indice_[i].len_] = '\0';
            indice_[i].bptr_->clear_reference();
            if (i>0)
                indice_[i].len_ += indice_[i-1].len_;
        }

        return *this;
    }

    /**
     *Set a string formated by a char vector.
     **/
    SelfT& assign (const std::vector<CharT>& v)
    {
        return assign(v.data(), v.size());
    }

    /**
     *Set a string formated by std string.
     **/
    SelfT& assign (const std::string& str)
    {
        return assign((CharT*)str.c_str(), str.length()/sizeof(CharT));
    }

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
        derefer_all();
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

        if (this == &str)
        {
            SelfT s = str;
            return insert(pos1, s);
        }

        length_ += str.length();
        size_t s = binary_search(pos1, 0, idx_len_-1);
        size_t last_s = (s==0? 0: indice_[s-1].len_);
        size_t ii = pos1-last_s;

        //within one bucket
        if (str.length() + indice_[s].len_-last_s<=BUCKET_LENGTH)
        {
            //std::cout<<str.length()<<" "<<s<<std::endl;
            //str.display();

            if (is_refered(s))
                duplicate(s);
            BUCKET_* bu = indice_[s].bptr_;

            for (size_t i = indice_[s].len_-last_s-1; i>ii; i--)
                (*bu)[i+str.length()] = (*bu)[i];
            (*bu)[indice_[s].len_-last_s+str.length()]= '\0';

            for (size_t i = 0; i<str.length(); i++)
                (*bu)[ii+1+i] = str[i];

            for (size_t i=s; i<idx_len_; i++)
                indice_[i].len_ += str.length();

            return *this;
        }

        size_t ex = str.indice_length();
        BUCKET_* tail = NULL;
        size_t tt = 0;

        //split bucket
        if (ii+1 < indice_[s].len_-last_s)
        {
            ex++;
            //copy head
            if (is_refered(s))
                duplicate(s);

            //copy tail
            tail = new_bucket();
            memcpy(tail, indice_[s].bptr_->p_+(ii+1)*sizeof(CharT), (BUCKET_LENGTH - (ii+1))*sizeof(CharT));
            tail->clear_reference();
            tt  = indice_[s].len_ - last_s - ii-1;
            (*tail)[tt] = '\0';

            (*indice_[s].bptr_)[ii+1] = '\0';
            indice_[s].len_ = last_s + ii + 1;
        }

        last_s = indice_[s].len_;

        //extend indice
        if (ex + idx_len_>idx_size_)
        {
            indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
            idx_size_ = ex + idx_len_;
        }

        //shift indices
        for (size_t i = idx_len_-1; i>s; i--)
            indice_[i+ex] = indice_[i];
        memcpy(indice_+s+1, str.indice_, str.indice_length()*sizeof(INDEX_));

        //copy str into
        for (size_t i=s+1; i<s+1+str.indice_length(); i++)
        {
            if (COPY_ON_WRITE)
                indice_[i].bptr_->refer();
            else
                duplicate(i);

            indice_[i].len_ += last_s;
        }

        idx_len_ += ex;

        if (tail!=NULL)
        {
            indice_[s+1+str.indice_length()].bptr_ = tail;
            indice_[s+1+str.indice_length()].len_ = indice_[s+str.indice_length()].len_ + tt;
        }

        for (size_t i=s+ex+1; i<idx_len_; i++)
            indice_[i].len_ += str.length();

        return *this;
    }

    /**
     *Inserts a copy of a substring of str at character position pos1.
     *The substring is the portion of str that begins at the character
     *position pos2 and takes up to n characters (it takes less than n
     *if the end of str is reached before).
     **/
    SelfT& insert ( size_t pos1, const SelfT& str, size_t pos2, size_t n )
    {
        return insert(pos1, str.substr(pos2, n));
    }

    /**
     *Inserts at the character position pos1, a copy of the string formed
     *by the first n characters in the array of characters pointed by s.
     **/
    SelfT& insert ( size_t pos1, const CharT* s, size_t n)
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
    SelfT& insert ( size_t pos1, const CharT* s )
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
    iterator insert ( iterator p, CharT c )
    {
        uint64_t i = p - begin();
        insert(i, 1, c);
        return iterator(begin()+i+1);
    }

    /**
     *Inserts a string formed by the repetition of character c, n times,
     *at the position referred by iterator p.
     **/
    void insert ( iterator p, size_t n, CharT c )
    {
        uint64_t i = p - begin();

        insert(i, n, c);
        return iterator(begin()+i+1);
    }

    /**
     *Inserts at the internal position referred by p the content made
     *up of the characters that go from the element referred by iterator
     *first to the element right before the one referred by iterator last.
     **/
    template<class InputIterator>
    void insert( iterator p, InputIterator first, InputIterator last )
    {
        uint64_t i = p - begin();
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
        if (pos>=length_ || n==0)
            return *this;

        if (n == npos)
            n = length_ -pos;

        length_ -= n;
        size_t i = binary_search(pos, 0, idx_len_-1);
        size_t e = binary_search(pos+n-1, i, idx_len_-1);

        size_t il = i==0?0 : indice_[i-1].len_;
        size_t el = e==0?0 : indice_[e-1].len_;

        if (i == e)
        {
            if (is_refered(i))
                duplicate(i);

            size_t j=pos+n-il;
            for (; j<indice_[i].len_-il; j++ )
                (*indice_[i].bptr_)[j-n] = (*indice_[i].bptr_)[j];
            (*indice_[i].bptr_)[j-n] = '\0';

            for (size_t j=i; j<idx_len_; j++)
                indice_[j].len_ -= n;

            return *this;
        }

        assert(pos>=il);
        assert(pos+n-1>=el);

        size_t si = pos - il;
        size_t ei = pos+n-1 - el;

        if (si>0)
        {
            if (is_refered(i))
            {
                duplicate(i);
            }

            (*indice_[i].bptr_)[si] = '\0';
            indice_[i].len_ = il + si;
            i++;
        }

        if (ei+1 < indice_[e].len_-el)
        {
            if (is_refered(e))
            {
                duplicate(e);
            }

            for (size_t j=ei+1; j<indice_[e].len_-el; j++)
                (*indice_[e].bptr_)[j-ei-1] = (*indice_[e].bptr_)[j];

            (*indice_[e].bptr_)[indice_[e].len_-el-ei-1] = '\0';

            e--;
        }

        //std::cout<<i<<" "<<e<<std::endl;

        //display();
        for (size_t j=i; j<=e; j++)
            derefer(j);

        for (size_t j=e+1; j<idx_len_; j++)
            indice_[j].len_ -= n;

        for (size_t j=e+1; e+1>i && j<idx_len_; j++)
        {
            indice_[j - (e-i+1)] = indice_[j];
            indice_[j].bptr_ = NULL;
        }

        if (e+1>i)
            idx_len_ -= (e-i+1);

        return *this;
    }

    /**
     *Erases the character referred by the iterator position.
     *Only one character is affected.
     **/
    iterator erase ( iterator position )
    {
        size_t i = position - begin();
        erase(i, 1);
        return iterator(begin()+i);
    }

    /**
     *Erases all the characters between first and last.
     **/
    iterator erase ( iterator first, iterator last )
    {
        uint64_t i = first - begin();
        uint64_t j = last - begin();
        erase(i, j-i);
        return iterator(begin()+i);
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
        erase(pos1, n1);

        if (str.length()==0)
            return *this;

        if (pos1>1)
            insert(pos1-1, str);
        else
            push_front(str);

        return *this;
    }

    /**
     *The section is replaced by a copy of the entire string object str.
     **/
    SelfT& replace ( iterator i1, iterator i2, const SelfT& str )
    {
        assert(i1<=i2);
        uint64_t i = i1 - begin();
        uint64_t j = i2 - begin();
        return replace(i, j-i+1, str);
    }

    /**
     *The section is replaced by a copy of a substring of str. The substring
     *is the portion of str that begins at the character position pos2 and
     *takes up to n2 characters (it takes less than n2 if the end of the
     *string is reached before).
     **/
    SelfT& replace ( size_t pos1, size_t n1, const SelfT& str, size_t pos2, size_t n2 )
    {
        return replace(pos1, n1, str.substr(pos2, n2));
    }

    /**
    * The section is replaced by a copy of the string formed by the first
    *n2 characters in the array of characters pointed by s.
     **/
    SelfT& replace ( size_t pos1, size_t n1,   const CharT* s, size_t n2 )
    {
        SelfT ss(s, n2);
        //ss.attach(s, n2);
        return replace(pos1, n1, ss);
    }

    /**
    * The section is replaced by a copy of the string formed by the first
    *n2 characters in the array of characters pointed by s.
     **/
    SelfT& replace ( iterator i1, iterator i2, const CharT* s, size_t n2 )
    {
        assert(i1<=i2);
        uint64_t i = i1 - begin();
        uint64_t j = i2 - begin();
        return replace(i, j-i+1, s, n2);
    }

    /**
     *The section is replaced by a copy of the string formed by the
     *null-terminated character sequence (C string) pointed by s. The
     *length of this caracter sequence is determined by the first ocurrence
     *of a null character (as determined by traits.length(s)).
     **/
    SelfT& replace ( size_t pos1, size_t n1,   const CharT* s )
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
    SelfT& replace ( iterator i1, iterator i2, const CharT* s )
    {
        assert(i1<=i2);
        uint64_t i = i1 - begin();
        uint64_t j = i2 - begin();
        return replace(i, j-i+1, s);
    }

    /**
     *The section is replaced by a repetition of character c, n2 times.
     **/
    SelfT& replace ( size_t pos1, size_t n1,   size_t n2, CharT c )
    {
        SelfT ss(n2, c);
        return replace(pos1, n1, ss);
    }

    /**
     *The section is replaced by a repetition of character c, n2 times.
     **/
    SelfT& replace ( iterator i1, iterator i2, size_t n2, CharT c )
    {
        assert(i1<=i2);
        uint64_t i = i1 - begin();
        uint64_t j = i2 - begin();

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
        uint64_t i = i1 - begin();
        uint64_t j = i2 - begin();
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
    size_t copy ( CharT* s, size_t n, size_t pos = 0) const
    {
        if (pos + n >= length_)
            n = length_-pos;

        size_t i = binary_search(pos, 0, idx_len_-1);
        size_t e = binary_search(pos+n-1, i, idx_len_-1);
        size_t il = i==0?0 : indice_[i-1].len_;
        size_t si = pos - il;

        if (e == i)
        {
            for (size_t j=0; j<n; j++)
            {
                s[j] = (*indice_[i].bptr_)[j+si];
            }

            return n;

        }

        for (size_t j=i,k=0; j<=e; j++)
        {
            size_t t = 0;
            if (j==i)
                t = si;

            for (; (*indice_[i].bptr_)[t]!='\0' && k<n; t++,k++)
                s[k] = (*indice_[j].bptr_)[t];

        }

        return n;
    }

    /**
     *Swaps the contents of the string with those of string object str,
     *such that after the call to this member function, the contents of
     *this string are those which were in str before the call, and the contents
     *of str are those which were in this string.
     **/
    void swap ( SelfT& str )
    {
        SelfT s = str;
        str = *this;
        *this = s;
    }

    //******************String operations********************
public:
    /**
     *Unavailable
     **/
    const CharT* c_str ( ) const
    {
        return NULL;
    }

    /**
     *Unavailable
     **/
    const CharT* data() const
    {
        return NULL;
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
    size_t find ( const SelfT& str, size_t pos = 0 ) const
    {
        size_t i = Algorithm<SelfT>::KMP(substr(pos), str);
        if (i == (size_t)-1)
            return -1;

        return pos + i;
    }

    size_t find ( const CharT* s, size_t pos, size_t n ) const
    {
        SelfT ss(s, n);
        //ss.attach(s, n);

        return find(ss, pos);
    }

    size_t find ( const CharT* s, size_t pos = 0 ) const
    {
        SelfT ss(s);
        //ss.attach(s);

        return find(ss, pos);
    }

    size_t find ( CharT c, size_t pos = 0 ) const
    {
        size_t r = 0;

        for (const_iterator i =begin()+pos; i<end(); i++, r++)
            if (*i==c)
                return r+pos;

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
    size_t rfind ( const SelfT& str, size_t pos = npos ) const
    {
        assert(pos == npos || pos<length_);

        return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), str);
    }

    size_t rfind ( const char* s, size_t pos, size_t n ) const
    {
        assert(pos == npos || pos<length_);

        SelfT ss(s, n);
        //ss.attach(s, n);
        return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), ss);
    }

    size_t rfind ( const CharT* s, size_t pos = npos ) const
    {
        assert(pos == npos || pos<length_);

        SelfT ss(s);
        //ss.attach(s);
        return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), ss);
    }

    size_t rfind ( CharT c, size_t pos = npos ) const
    {
        assert(pos == npos || pos<length_);

        if (pos == npos)
            pos = length_ -1;

        size_t r = pos;
        for (const_reverse_iterator i =rbegin()+(length_-1-pos); i<rend(); i++, r--)
            if (*i==c)
                return r;

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
    SelfT substr ( size_t pos = 0, size_t n = npos ) const
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
        const_iterator i=str.begin();
        const_iterator j=begin();
        for (; i!=str.end() && j!=end(); i++,j++)
        {
            if (*j>*i)
                return 1;
            if (*j<*i)
                return -1;
        }

        if (i==str.end() && j==end())
            return 0;

        if (j== end())
            return -1;

        return 1;
    }

    int compare ( const CharT* s ) const
    {
        size_t len = getLen(s);

        size_t i=0;
        const_iterator it = begin();

        for (; it!=end() && i<len; i++, it++)
        {
            //std::cout<<*it<<"  "<<s[i]<<std::endl;
            if (*it>s[i])
                return 1;
            if (*it<s[i])
                return -1;
        }

        //std::cout<<i<<" "<<length_<<" "<<len<<std::endl;

        if (i==length_ && i==len)
            return 0;
        if (i== length_)
            return -1;

        return 1;
    }

    int compare ( size_t pos1, size_t n1, const SelfT& str ) const
    {
        return substr(*this, pos1, n1).compare(str);
    }

    int compare ( size_t pos1, size_t n1, const CharT* s) const
    {
        return substr(*this, pos1, n1).compare(s);
    }

    int compare ( size_t pos1, size_t n1, const SelfT& str, size_t pos2, size_t n2 ) const
    {
        return substr(*this, pos1, n1).compare(str.substr(pos2, n2));
    }

    int compare ( size_t pos1, size_t n1, const CharT* s, size_t n2) const
    {
        SelfT str(s, n2);
        return substr(*this, pos1, n1).compare(str);
    }

    std::vector<CharT> cast_std_vector()
    {
        std::vector<CharT> v;
        v.resize(length_);
        for (const_iterator i =begin(); i!=end(); i++)
            v.push_back(*i);

        return v;
    }

    std::string cast_std_string()
    {
        std::string v;
        v.reserve(length_);
        for (const_iterator i =begin(); i!=end(); i++)
            v += (*i);

        return v;

    }


//******************Serialization********************
public:

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version)  const
    {
        ar & length_;
        ar & idx_len_;
        ar & idx_size_;

        for (size_t i=0; i<idx_len_; i++)
        {
            ar & indice_[i].len_;
            ar.save_binary(indice_[i].bptr_, BUCKET_SIZE);
        }
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        clear();

        ar & length_;
        ar & idx_len_;
        ar & idx_size_;

        indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
        clear_indice(0, idx_size_);

        for (size_t i=0; i<idx_len_; i++)
        {
            ar & indice_[i].len_;
            indice_[i].bptr_ = new_bucket();
            ar.load_binary(indice_[i].bptr_, BUCKET_SIZE);
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    friend std::ostream& operator << (std::ostream& os, const SelfT& str)
    {
        for (const_iterator i =str.begin(); i!=str.end(); i++)
            os<<(char)*i;

        return os;
    }

    void display() const
    {
        for (size_t i=0; i<idx_len_; i++)
        {
            std::cout<<indice_[i]<<std::endl;
        }
    }

}
;

template <
class CHAR_TYPE ,
      int COPY_ON_WRITE,
      uint64_t BUCKET_BYTES
      >
const size_t bbt_string<CHAR_TYPE ,COPY_ON_WRITE, BUCKET_BYTES>::npos = -1;

NS_IZENELIB_UTIL_END
#endif
