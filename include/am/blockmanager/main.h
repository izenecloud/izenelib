/***************************************************************************
 *
 *  Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2008 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/
#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <assert.h>

#include <boost/noncopyable.hpp>

#include <types.h>
#include "blockmanager_fwd.h"
#include "iobase.h"
using namespace std;

NS_IZENELIB_AM_BEGIN

//! \defgroup mnglayer Block management layer
//! Group of classes which help controlling external memory space,
//! managing disks, and allocating and deallocating blocks of external storage
//! \{

//! \brief Block identifier class

//! Stores block identity, given by file and offset within the file
template <unsigned SIZE>
struct BID
{
    enum
    {
        size = SIZE,         //!< Block size
        t_size = SIZE        //!< Blocks size, given by the parameter
    };
    file * storage;          //!< pointer to the file of the block
    int64_t offset;     //!< offset within the file of the block
    BID() : storage(NULL), offset(0) { }
    bool valid() const
    {
        return storage;
    }
    BID(file * s, int64_t o) : storage(s), offset(o) { }
    BID(const BID & obj) : storage(obj.storage), offset(obj.offset) { }
    template <unsigned BlockSize>
    explicit BID(const BID<BlockSize> & obj) : storage(obj.storage), offset(obj.offset) { }
};


//! \brief Specialization of block identifier class (BID) for variable size block size

//! Stores block identity, given by file, offset within the file, and size of the block
template <>
struct BID<0>
{
    file * storage;          //!< pointer to the file of the block
    int64_t offset;     //!< offset within the file of the block
    unsigned size;           //!< size of the block in bytes
    enum
    {
        t_size = 0           //!< Blocks size, given by the parameter
    };
    BID() : storage(NULL), offset(0), size(0) { }
    BID(file * f, int64_t o, unsigned s) : storage(f), offset(o), size(s) { }
    bool valid() const
    {
        return storage;
    }
};

template <unsigned blk_sz>
bool operator == (const BID<blk_sz> & a, const BID<blk_sz> & b)
{
    return (a.storage == b.storage) && (a.offset == b.offset) && (a.size == b.size);
}

template <unsigned blk_sz>
bool operator != (const BID<blk_sz> & a, const BID<blk_sz> & b)
{
    return (a.storage != b.storage) || (a.offset != b.offset) || (a.size != b.size);
}


template <unsigned blk_sz>
std::ostream & operator << (std::ostream & s, const BID<blk_sz> & bid)
{
    s << " storage file addr: " << bid.storage;
    s << " offset: " << bid.offset;
    s << " size: " << bid.size;
    return s;
}


template <unsigned bytes>
class filler_struct__
{
    typedef unsigned char byte_type;
    byte_type filler_array_[bytes];

public:
    filler_struct__() {}
};

template <>
class filler_struct__<0>
{
    typedef unsigned char byte_type;

public:
    filler_struct__() {}
};

//! \brief Contains data elements for \c typed_block , not intended for direct use
template <class T, unsigned Size_>
class element_block
{
public:
    typedef T type;
    typedef T value_type;
    typedef T & reference;
    typedef const T & const_reference;
    typedef type * pointer;
    typedef pointer iterator;
    typedef const type * const_iterator;

    enum
    {
        size = Size_ //!< number of elements in the block
    };

    //! Array of elements of type T
    T elem[size];

    element_block() {}

    //! An operator to access elements in the block
    reference operator [] (int i)
    {
        return elem[i];
    }

    //! \brief Returns \c iterator pointing to the first element
    iterator begin()
    {
        return elem;
    }
    //! \brief Returns \c const_iterator pointing to the first element
    const_iterator begin() const
    {
        return elem;
    }
    //! \brief Returns \c const_iterator pointing to the first element
    const_iterator cbegin() const
    {
        return begin();
    }
    //! \brief Returns \c iterator pointing to the end element
    iterator end()
    {
        return elem + size;
    }
    //! \brief Returns \c const_iterator pointing to the end element
    const_iterator end() const
    {
        return elem + size;
    }
    //! \brief Returns \c const_iterator pointing to the end element
    const_iterator cend() const
    {
        return end();
    }
};

//! \brief Contains BID references for \c typed_block , not intended for direct use
template <class T, unsigned Size_, unsigned RawSize_, unsigned NBids_ = 0>
class block_w_bids : public element_block<T, Size_>
{
public:
    enum
    {
        raw_size = RawSize_,
        nbids = NBids_
    };
    typedef BID<raw_size> bid_type;

    //! Array of BID references
    bid_type ref[nbids];

    //! An operator to access bid references
    bid_type & operator () (int i)
    {
        return ref[i];
    }

    block_w_bids() {}
};

template <class T, unsigned Size_, unsigned RawSize_>
class block_w_bids<T, Size_, RawSize_, 0>: public element_block<T, Size_>
{
public:
    enum
    {
        raw_size = RawSize_,
        nbids = 0
    };
    typedef BID<raw_size> bid_type;

    block_w_bids() {}
};

//! \brief Contains per block information for \c typed_block , not intended for direct use
template <class T_, unsigned RawSize_, unsigned NBids_, class InfoType_ = void>
class block_w_info :
            public block_w_bids<T_, ((RawSize_ - sizeof(BID<RawSize_>) * NBids_ - sizeof(InfoType_)) / sizeof(T_)), RawSize_, NBids_>
{
public:
    //! \brief Type of per block information element
    typedef InfoType_ info_type;

    //! \brief Per block information element
    info_type info;

    enum { size = ((RawSize_ - sizeof(BID<RawSize_>) * NBids_ - sizeof(InfoType_)) / sizeof(T_)) };

public:
    block_w_info() {}
};

template <class T_, unsigned RawSize_, unsigned NBids_>
class block_w_info<T_, RawSize_, NBids_, void>:
            public block_w_bids<T_, ((RawSize_ - sizeof(BID<RawSize_>) * NBids_) / sizeof(T_)), RawSize_, NBids_>
{
public:
    typedef void info_type;
    enum { size = ((RawSize_ - sizeof(BID<RawSize_>) * NBids_) / sizeof(T_)) };

public:
    block_w_info() {}
};

template <size_t ALIGNMENT>
inline void * aligned_alloc(size_t size, size_t meta_info_size = 0)
{
    char * buffer = new char[size + ALIGNMENT + sizeof(char *) + meta_info_size];
    memset(buffer, 0, size + ALIGNMENT + sizeof(char *) + meta_info_size);
    char * reserve_buffer = buffer + sizeof(char *) + meta_info_size;
    char * result = reserve_buffer + ALIGNMENT -
                    (((unsigned long)reserve_buffer) % (ALIGNMENT)) - meta_info_size;
    assert(int(result - buffer) >= int(sizeof(char *)));
    *(((char **)result) - 1) = buffer;

    return result;
}

template <size_t ALIGNMENT>
inline void aligned_dealloc(void * ptr)
{
    delete[] * (((char **)ptr) - 1);
}


//! \brief Block containing elements of fixed length

//! Template parameters:
//! - \c RawSize_ size of block in bytes
//! - \c T_ type of block's records
//! - \c NRef_ number of block references (BIDs) that can be stored in the block (default is 0)
//! - \c InfoType_ type of per block information (default is no information - void)
//!
//! The data array of type T_ is contained in the parent class \c element_block, see related information there.
//! The BID array of references is contained in the parent class \c block_w_bids, see related information there.
//! The "per block information" is contained in the parent class \c block_w_info, see related information there.
//!  \warning If \c RawSize_ > 2MB object(s) of this type can not be allocated on the stack (as a
//! function variable for example), because Linux POSIX library limits the stack size for the
//! main thread to (2MB - system page size)
template <unsigned RawSize_, class T_, unsigned NRef_ = 0, class InfoType_ = void>
class typed_block :
            public block_w_info<T_, RawSize_, NRef_, InfoType_>,
            public filler_struct__<(RawSize_ - sizeof(block_w_info<T_, RawSize_, NRef_, InfoType_>))>
{
public:
    typedef T_ type;
    typedef T_ value_type;
    typedef T_ & reference;
    typedef const T_ & const_reference;
    typedef type * pointer;
    typedef pointer iterator;
    typedef type const * const_iterator;

    enum { has_filler = (RawSize_ != sizeof(block_w_info<T_, RawSize_, NRef_, InfoType_>)) };

    typedef BID<RawSize_> bid_type;

    typed_block() {}

    enum
    {
        raw_size = RawSize_,                                      //!< size of block in bytes
        size = block_w_info<T_, RawSize_, NRef_, InfoType_>::size //!< number of elements in block
    };

    /*! \brief Writes block to the disk(s)
     *! \param bid block identifier, points the file(disk) and position
     *! \param on_cmpl completion handler
     *! \return \c pointer_ptr object to track status I/O operation after the call
     */
    request_ptr write(const BID<raw_size> & bid,
                      completion_handler on_cmpl = default_completion_handler())
    {
        return bid.storage->awrite(this,bid.offset,raw_size,on_cmpl);
    }

    /*! \brief Reads block from the disk(s)
     *! \param bid block identifier, points the file(disk) and position
     *! \param on_cmpl completion handler
     *! \return \c pointer_ptr object to track status I/O operation after the call
     */
    request_ptr read(const BID<raw_size> & bid,
                     completion_handler on_cmpl = default_completion_handler())
    {
        return bid.storage->aread(this, bid.offset, raw_size, on_cmpl);
    }

    static void * operator new[] (size_t bytes)
    {
        unsigned_type meta_info_size = bytes % raw_size;

        void * result = aligned_alloc<BLOCK_ALIGN>(bytes, meta_info_size);
        memset(result, 0, bytes);
        char * tmp = (char *)result;
        tmp += RawSize_;
        while (tmp < ((char *)result) + bytes)
        {
            tmp += RawSize_;
        }
        return result;
    }

    static void * operator new (size_t bytes)
    {
        unsigned_type meta_info_size = bytes % raw_size;

        void * result = aligned_alloc<BLOCK_ALIGN>(bytes, meta_info_size);
        memset(result, 0, bytes);
        char * tmp = (char *)result;
        tmp += RawSize_;
        while (tmp < ((char *)result) + bytes)
        {
            tmp += RawSize_;
        }
        return result;
    }

    static void * operator new (size_t /*bytes*/, void * ptr)     // construct object in existing memory
    {
        return ptr;
    }

    static void operator delete[] (void * ptr)
    {
        aligned_dealloc<BLOCK_ALIGN>(ptr);
    }

    static void operator delete (void * ptr)
    {
        aligned_dealloc<BLOCK_ALIGN>(ptr);
    }

    static void operator delete (void *, void *)
    { }

    // STRANGE: implementing destructor makes g++ allocate
    // additional 4 bytes in the beginning of every array
    // of this type !? makes aligning to 4K boundaries difficult
    //
    // http://www.cc.gatech.edu/grads/j/Seung.Won.Jun/tips/pl/node4.html :
    // "One interesting thing is the array allocator requires more memory
    //  than the array size multiplied by the size of an element, by a
    //  difference of delta for metadata a compiler needs. It happens to
    //  be 8 bytes long in g++."
    // ~typed_block() { }
};



template <unsigned BLK_SIZE>
class BIDArray : private boost::noncopyable
{
protected:
    unsigned_type _size;
    BID<BLK_SIZE> * array;

public:
    typedef BID<BLK_SIZE> & reference;
    typedef BID<BLK_SIZE> * iterator;
    typedef const BID<BLK_SIZE> * const_iterator;
    BIDArray() : _size(0), array(NULL)
    { }
    iterator begin()
    {
        return array;
    }
    iterator end()
    {
        return array + _size;
    }

    BIDArray(unsigned_type size) : _size(size)
    {
        array = new BID<BLK_SIZE>[size];
    }
    unsigned_type size() const
    {
        return _size;
    }
    reference operator [] (int_type i)
    {
        return array[i];
    }
    void resize(unsigned_type newsize)
    {
        if (array)
        {
            BID<BLK_SIZE> * tmp = array;
            array = new BID<BLK_SIZE>[newsize];
            memcpy((void *)array, (void *)tmp,
                   sizeof(BID<BLK_SIZE>) * (std::min(_size, newsize)));
            delete[] tmp;
            _size = newsize;
        }
        else
        {
            array = new BID<BLK_SIZE>[newsize];
            _size = newsize;
        }
    }
    ~BIDArray()
    {
        if (array)
            delete[] array;
    }
};


class DiskAllocator : private boost::noncopyable
{
    boost::mutex mutex;

    typedef std::pair<int64_t, int64_t> place;
    struct FirstFit : public std::binary_function<place, int64_t, bool>
    {
        bool operator () (const place & entry,const int64_t size) const
        {
            return (entry.second >= size);
        }
    };
    struct OffCmp
    {
        bool operator () (const int64_t & off1, const int64_t & off2)
        {
            return off1 < off2;
        }
    };

    DiskAllocator() { }

protected:
    typedef std::map<int64_t, int64_t> sortseq;
    sortseq free_space;
    //  sortseq used_space;
    int64_t free_bytes;
    int64_t disk_bytes;

    void dump()
    {
        int64_t total = 0;
        sortseq::const_iterator cur = free_space.begin();
        for ( ; cur != free_space.end(); ++cur)
        {
            total += cur->second;
        }
    }

    void check_corruption(int64_t region_pos, int64_t region_size,
                          sortseq::iterator pred, sortseq::iterator succ)
    {
        if (pred != free_space.end())
        {
            if (pred->first <= region_pos && pred->first + pred->second > region_pos)
            {
                std::ostringstream msg;
                msg << "Error in DiskAllocator::check_corruption " << ": " << "Error: double deallocation of external memory " <<
                "System info: P " << pred->first << " " << pred->second << " " << region_pos;
                throw(std::runtime_error(msg.str()));
            }
        }
        if (succ != free_space.end())
        {
            if (region_pos <= succ->first && region_pos + region_size > succ->first)
            {
                std::ostringstream msg;
                msg << "Error in DiskAllocator::check_corruption " << ": " << "Error: double deallocation of external memory " <<
                "System info: P " << region_pos << " " << region_size << " " << succ->first;
                throw(std::runtime_error(msg.str()));
            }
        }
    }

public:
    inline DiskAllocator(int64_t disk_size);

    inline int64_t get_free_bytes() const
    {
        return free_bytes;
    }
    inline int64_t get_used_bytes() const
    {
        return disk_bytes - free_bytes;
    }
    inline int64_t get_total_bytes() const
    {
        return disk_bytes;
    }

    template <unsigned BLK_SIZE>
    int64_t new_blocks(BIDArray<BLK_SIZE> & bids);

    template <unsigned BLK_SIZE>
    int64_t new_blocks(BID<BLK_SIZE> * begin,
                       BID<BLK_SIZE> * end);

    template <unsigned BLK_SIZE>
    void delete_block(const BID<BLK_SIZE> & bid);
};

DiskAllocator::DiskAllocator(int64_t disk_size) :
        free_bytes(disk_size),
        disk_bytes(disk_size)
{
    free_space[0] = disk_size;
}


template <unsigned BLK_SIZE>
int64_t DiskAllocator::new_blocks(BIDArray<BLK_SIZE> & bids)
{
    return new_blocks(bids.begin(), bids.end());
}

template <unsigned BLK_SIZE>
int64_t DiskAllocator::new_blocks(BID<BLK_SIZE> * begin,
                                  BID<BLK_SIZE> * end)
{
    boost::mutex::scoped_lock lock(mutex);


    int64_t requested_size = 0;

    typename BIDArray<BLK_SIZE>::iterator cur = begin;
    for ( ; cur != end; ++cur)
    {
        requested_size += cur->size;
    }

    if (free_bytes < requested_size)
    {
        begin->offset = disk_bytes; // allocate at the end
        for (++begin; begin != end; ++begin)
        {
            begin->offset = (begin - 1)->offset + (begin - 1)->size;
        }
        disk_bytes += requested_size;

        return disk_bytes;
    }

    // dump();

    sortseq::iterator space =
        std::find_if(free_space.begin(), free_space.end(),
                     bind2nd(FirstFit(), requested_size));

    if (space != free_space.end())
    {
        int64_t region_pos = (*space).first;
        int64_t region_size = (*space).second;
        free_space.erase(space);
        if (region_size > requested_size)
            free_space[region_pos + requested_size] = region_size - requested_size;

        begin->offset = region_pos;
        for (++begin; begin != end; ++begin)
        {
            begin->offset = (begin - 1)->offset + (begin - 1)->size;
        }
        free_bytes -= requested_size;
        //dump();

        return disk_bytes;
    }

    // no contiguous region found
    if (requested_size == BLK_SIZE)
    {
        assert(end - begin == 1);

        dump();

        begin->offset = disk_bytes; // allocate at the end
        disk_bytes += BLK_SIZE;

        return disk_bytes;
    }

    assert(requested_size > BLK_SIZE);
    assert(end - begin > 1);

    lock.unlock();

    typename  BIDArray<BLK_SIZE>::iterator middle = begin + ((end - begin) / 2);
    new_blocks(begin, middle);
    new_blocks(middle, end);

    return disk_bytes;
}


template <unsigned BLK_SIZE>
void DiskAllocator::delete_block(const BID<BLK_SIZE> & bid)
{
    boost::mutex::scoped_lock lock(mutex);

    int64_t region_pos = bid.offset;
    int64_t region_size = bid.size;
    if (!free_space.empty())
    {
        sortseq::iterator succ = free_space.upper_bound(region_pos);
        sortseq::iterator pred = succ;
        pred--;
        check_corruption(region_pos, region_size, pred, succ);
        if (succ == free_space.end())
        {
            if (pred == free_space.end())
            {
                dump();
                assert(pred != free_space.end());
            }
            if ((*pred).first + (*pred).second == region_pos)
            {
                // coalesce with predecessor
                region_size += (*pred).second;
                region_pos = (*pred).first;
                free_space.erase(pred);
            }
        }
        else
        {
            if (free_space.size() > 1)
            {
                bool succ_is_not_the_first = (succ != free_space.begin());
                if ((*succ).first == region_pos + region_size)
                {
                    // coalesce with successor
                    region_size += (*succ).second;
                    free_space.erase(succ);
                }
                if (succ_is_not_the_first)
                {
                    if (pred == free_space.end())
                    {
                        dump();
                        assert(pred != free_space.end());
                    }
                    if ((*pred).first + (*pred).second == region_pos)
                    {
                        // coalesce with predecessor
                        region_size += (*pred).second;
                        region_pos = (*pred).first;
                        free_space.erase(pred);
                    }
                }
            }
            else
            {
                if ((*succ).first == region_pos + region_size)
                {
                    // coalesce with successor
                    region_size += (*succ).second;
                    free_space.erase(succ);
                }
            }
        }
    }

    free_space[region_pos] = region_size;
    free_bytes += int64_t(bid.size);

    //dump();
}

//! \brief Block manager class

//! Manages allocation and deallocation of blocks in multiple/single disk setting
//! \remarks is a singleton
class block_manager : public singleton<block_manager>
{
    friend class singleton<block_manager>;

    DiskAllocator * disk_allocator;
    file * disk_file;

    unsigned ndisks;

    block_manager() {}


protected:
    template <class BIDType, class BIDIteratorClass>
    void new_blocks_int(const unsigned_type nblocks,BIDIteratorClass out);

public:
    void init(const char* path, int64_t disk_size = 1000 * 1024 * 1024)
    {
        disk_file = new boostfd_file(path,file::CREAT | file::RDWR | file::DIRECT);
        disk_file->set_size(disk_size);
        disk_allocator = new DiskAllocator(disk_size);
    }

    //! \brief Allocates new blocks

    //! Allocates new blocks according to the strategy
    //! given by \b functor and stores block identifiers
    //! to the range [ \b bidbegin, \b bidend)
    //! \param functor object of model of \b allocation_strategy concept
    //! \param bidbegin bidirectional BID iterator object
    //! \param bidend bidirectional BID iterator object
    template <class BIDIteratorClass>
    void new_blocks(BIDIteratorClass bidbegin,BIDIteratorClass bidend);

    //! Allocates new blocks according to the strategy
    //! given by \b functor and stores block identifiers
    //! to the output iterator \b out
    //! \param nblocks the number of blocks to allocate
    //! \param functor object of model of \b allocation_strategy concept
    //! \param out iterator object of OutputIterator concept
    //!
    //! The \c BlockType template parameter defines the type of block to allocate
    template <class BlockType, class BIDIteratorClass>
    void new_blocks(const unsigned_type nblocks,BIDIteratorClass out);


    //! \brief Deallocates blocks

    //! Deallocates blocks in the range [ \b bidbegin, \b bidend)
    //! \param bidbegin iterator object of \b bid_iterator concept
    //! \param bidend iterator object of \b bid_iterator concept
    template <class BIDIteratorClass>
    void delete_blocks(const BIDIteratorClass & bidbegin, const BIDIteratorClass & bidend);

    //! \brief Deallocates a block
    //! \param bid block identifier
    template <unsigned BLK_SIZE>
    void delete_block(const BID<BLK_SIZE> & bid);

    ~block_manager()
    {
        delete disk_allocator;
        delete disk_file;
    }
};


template <class BIDType, class OutputIterator>
void block_manager::new_blocks_int(const unsigned_type nblocks,OutputIterator out)
{
    typedef BIDType bid_type;
    typedef BIDArray<bid_type::t_size> bid_array_type;

    bid_array_type disk_bid;

    unsigned_type i = 0;

    disk_bid.resize(nblocks);
    const int64_t old_capacity = disk_allocator->get_total_bytes();
    const int64_t new_capacity = disk_allocator->new_blocks(disk_bid);
    if (old_capacity != new_capacity)
    {
        // resize the file
        disk_file->set_size(new_capacity);
        if (new_capacity != disk_allocator->get_total_bytes())
            cout<<"File resizing failed: actual size " << disk_allocator->get_total_bytes() << " != requested size " << new_capacity<<endl;
    }

    OutputIterator it = out;
    for (i = 0; i != nblocks; ++it, ++i)
    {
        bid_type bid(disk_file, disk_bid[i].offset);
        *it = bid;
    }

}

template <class BlockType, class OutputIterator>
void block_manager::new_blocks(const unsigned_type nblocks,OutputIterator out)
{
    typedef typename BlockType::bid_type bid_type;
    new_blocks_int<bid_type>(nblocks, out);
}

template <class BIDIteratorClass>
void block_manager::new_blocks(BIDIteratorClass bidbegin,BIDIteratorClass bidend)
{
    typedef typename std::iterator_traits<BIDIteratorClass>::value_type bid_type;

    unsigned_type nblocks = 0;

    BIDIteratorClass bidbegin_copy(bidbegin);
    while (bidbegin_copy != bidend)
    {
        ++bidbegin_copy;
        ++nblocks;
    }

    new_blocks_int<bid_type>(nblocks, bidbegin);
}


template <unsigned BLK_SIZE>
void block_manager::delete_block(const BID<BLK_SIZE> & bid)
{
    disk_allocator->delete_block(bid);
    disk_file->delete_region(bid.offset, bid.size);
}


template <class BIDIteratorClass>
void block_manager::delete_blocks(
    const BIDIteratorClass & bidbegin,
    const BIDIteratorClass & bidend)
{
    for (BIDIteratorClass it = bidbegin; it != bidend; it++)
    {
        delete_block(*it);
    }
}


// in bytes
#ifndef DEFAULT_BLOCK_SIZE
#define DEFAULT_BLOCK_SIZE(type) (2 * 1024 * 1024) // use traits
#endif

NS_IZENELIB_AM_END


#endif

