#ifndef IZENELIB_UTIL_COMPRESSED_VECTOR_ORDERED_H
#define IZENELIB_UTIL_COMPRESSED_VECTOR_ORDERED_H

#include "detail/Compressor.h"
#include <util/mem_pool.h>

#include <boost/iterator/iterator_facade.hpp>
#include <limits> 
#include <iostream>

NS_IZENELIB_UTIL_BEGIN

namespace compressed_vector{

template<bool Ordered=true>
class VIntVector
{
    izenelib::util::mem_pool* pMemPool_;
    detail::DataChunk* pHeadChunk_;
    detail::DataChunk* pTailChunk_;
    uint32_t nTotalSize_;
    uint32_t nPosInCurChunk_;
    uint32_t nTotalUsed_;
    uint32_t nLastVal_;

    uint32_t nCount_;

    void add_v_data(uint32_t val)
    {
        if (pTailChunk_ == NULL)
        {
            add_chunk();
            add_v_data(val);
            return;
        }
        int32_t left = pTailChunk_->size - nPosInCurChunk_;
	
        if (left < 7)///at least 4 free space
        {
            pTailChunk_->size = nPosInCurChunk_;///the real size
            add_chunk();
            add_v_data(val);
            return;
        }
        uint32_t ui = val;
        while ((ui & ~0x7F) != 0)
        {
            pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
            ui >>= 7;
            nTotalUsed_++;
        }
        pTailChunk_->data[nPosInCurChunk_++] = (uint8_t)ui;
        nTotalUsed_++;
        nCount_++;
    }

    void add_chunk()
    {
        size_t chunkSize = std::min(CHUNK_ALLOC_UPPER_LIMIT, 
                  std::max(CHUNK_ALLOC_LOWER_LIMIT,(int)(nTotalSize_*0.5 + 0.5)));

        uint8_t* begin = (uint8_t *)pMemPool_->allocate<uint8_t>(chunkSize).get();

        detail::DataChunk* pChunk = (detail::DataChunk*)begin;
        pChunk->size = (int32_t)(chunkSize - sizeof(detail::DataChunk*) - sizeof(int32_t));
        pChunk->next = NULL;
 
        if (pTailChunk_)
            pTailChunk_->next = pChunk;
        pTailChunk_ = pChunk;
        if (!pHeadChunk_)
            pHeadChunk_ = pTailChunk_;
        nTotalSize_ += pChunk->size;
        nPosInCurChunk_ = 0;
    }

    void read_internal(
        detail::DataChunk* &pDataChunk, 
        int32_t& currPosInChunk, 
        uint8_t* buffer, 
        size_t length
    ) const
    {
        if(!pDataChunk) return;
        size_t nLen = length;
        uint8_t* pEnd = pDataChunk->data + pDataChunk->size;
        uint8_t* pData = pDataChunk->data + currPosInChunk;
        while(nLen > 0)
        {
            if(nLen > (size_t)(pEnd - pData))
            {
                memcpy(buffer,pData,pEnd - pData);
                buffer += (pEnd - pData);
                nLen -= (pEnd - pData);
                pDataChunk = pDataChunk->next;
                if(pDataChunk)
                {
                    pData = pDataChunk->data;
                    pEnd = pDataChunk->data + pDataChunk->size;
                }
                else
                {
                    pData = NULL;
                    break;
                }
            }
            else
            {
                memcpy(buffer,pData,nLen);					
                if(nLen == (size_t)(pEnd - pData))
                {
                    pDataChunk = pDataChunk->next;
                    if(pDataChunk)
                        pData = pDataChunk->data;
                    else
                        pData = NULL;
                }						
                else
                {
                    pData += nLen;
                }
                nLen = 0;
            }
        }
        currPosInChunk = (pData == NULL)? 0 : (int32_t)(pData - pDataChunk->data);
    }


public:
    VIntVector()
        :pMemPool_(NULL)
        ,pHeadChunk_(NULL)
        ,pTailChunk_(NULL)
        ,nTotalSize_(0)
        ,nPosInCurChunk_(0)
        ,nTotalUsed_(0)
        ,nLastVal_(0)
        ,nCount_(0)
	{}

    VIntVector(izenelib::util::mem_pool* pMemPool)
        :pMemPool_(pMemPool)
        ,pHeadChunk_(NULL)
        ,pTailChunk_(NULL)
        ,nTotalSize_(0)
        ,nPosInCurChunk_(0)
        ,nTotalUsed_(0)
        ,nLastVal_(0)
        ,nCount_(0)
	{}


    VIntVector(const VIntVector& other)
        :pMemPool_(other.pMemPool_)
        ,pHeadChunk_(other.pHeadChunk_)
        ,pTailChunk_(other.pTailChunk_)
        ,nTotalSize_(other.nTotalSize_)
        ,nPosInCurChunk_(other.nPosInCurChunk_)
        ,nTotalUsed_(other.nTotalUsed_)
        ,nLastVal_(other.nLastVal_)
        ,nCount_(other.nCount_)
	{}

    ~VIntVector(){}

    void push_back(uint32_t val)
    {
        add_v_data(val - nLastVal_);
        nLastVal_ = val;
    }

    size_t size() const { return nCount_;}

    template<typename Element>
    class vector_iterator
       : public boost::iterator_facade< 
                       vector_iterator<Element>, 
                       Element, 
                       boost::forward_traversal_tag >
    {
        const VIntVector * vector_;
        uint32_t data_len_;
        detail::DataChunk* p_data_chunk_;
        int32_t curr_pos_in_chunk_;
        mutable uint32_t curr_val_;
        uint8_t buffer_[CHUNK_ALLOC_LOWER_LIMIT];
        uint32_t buffer_len_;
        uint32_t buffer_start_;
        uint32_t buffer_pos_;

    public:
        vector_iterator()
        : vector_(0)
        , data_len_(0)
        , p_data_chunk_(0)
        , curr_pos_in_chunk_(0)
        , curr_val_(0)
        , buffer_len_(0)
        , buffer_start_(0)
        , buffer_pos_(0) {}

        explicit vector_iterator(const VIntVector<Ordered>& p)
        : vector_(&p)
        , data_len_(p.nTotalUsed_)
        , p_data_chunk_(p.pHeadChunk_)
        , curr_pos_in_chunk_(0)
        , curr_val_(0)
        , buffer_len_(0)
        , buffer_start_(0)
        , buffer_pos_(0) 
        {
            increment();
        }

    private:
        friend class boost::iterator_core_access;

        void increment() 
        { 
            if(vector_)
            {
                uint32_t v = read_vint32();
                curr_val_ += v;
                if(buffer_start_ + buffer_pos_ >= data_len_) vector_ = NULL;
            }
        }

        bool equal(vector_iterator<Element> const& other) const
        {
            return this->vector_ == other.vector_;
        }

        Element& dereference() const {return curr_val_; }

        uint32_t read_vint32()
        {
            uint8_t b = read_byte();
            uint32_t i = b & 0x7F;
            for (uint32_t shift = 7; (b & 0x80) != 0; shift += 7)
            {
                b = read_byte();
                i |= (b & 0x7FL) << shift;
            }
            return i;
        }

        uint8_t read_byte()
        {
            if (buffer_pos_ >= buffer_len_)
                refill();
            return buffer_[buffer_pos_++];
        }

        void refill()
        {
            buffer_start_ += buffer_pos_;
            buffer_pos_ = buffer_len_ = 0;
	
            assert(buffer_start_ < data_len_);
	
            if(buffer_start_ + CHUNK_ALLOC_LOWER_LIMIT > data_len_)
                buffer_len_ = data_len_ - buffer_start_;
            else
                buffer_len_ =  CHUNK_ALLOC_LOWER_LIMIT;

            vector_->read_internal(p_data_chunk_,curr_pos_in_chunk_,buffer_,buffer_len_);
        }
    };
    typedef vector_iterator<uint32_t> iterator;
    typedef vector_iterator<uint32_t const> const_iterator;
    iterator begin()
    {
        return iterator(*this);
    }
    iterator end()
    {
        return iterator();
    }
    const_iterator begin() const
    {
        return const_iterator(*this);
    }
    const_iterator end() const
    {
        return const_iterator();
    }
};

typedef VIntVector<true> OrderedVIntVector;
typedef VIntVector<false> UnOrderedVIntVector;
}

NS_IZENELIB_UTIL_END


#endif

