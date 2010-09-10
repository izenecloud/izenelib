/**
* @file        ListingCache.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Posting-level cache
*/

#ifndef LISTING_CACHE_H
#define LISTING_CACHE_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/CompressParameters.h>
#include <ir/index_manager/store/IndexInput.h>

#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <boost/thread/thread.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class CacheBlockInfo
{
public:

    CacheBlockInfo(size_t cache_size)
            : info_size_((cache_size / 32) + (cache_size % 32 == 0 ? 0 : 1))
            , bit_info_(new uint32_t[info_size_])
            , block_pin_count_(new int[cache_size])
    {
        // Initially all cache blocks are ready.
        memset(bit_info_,0,info_size_*sizeof(uint32_t));
    }

    ~CacheBlockInfo()
    {
        delete[] bit_info_;
        delete[] block_pin_count_;
    }

    void pinBlock(uint64_t cache_block) 
    {
        ++block_pin_count_[cache_block];
    }
	
    void unpinBlock(uint64_t cache_block) 
    {
        if (block_pin_count_[cache_block] > 0)
            --block_pin_count_[cache_block];
    }

    void setBlock(uint64_t cache_block)
    {
        int info_idx = info_index(cache_block);
        uint32_t data = bit_info_[info_idx];

        int mask = 1 << bit_offset(cache_block);
        bit_info_[info_idx] = data | mask;
    }

    void unsetBlock(uint64_t cache_block)
    {
        int info_idx = info_index(cache_block);
        uint32_t data = bit_info_[info_idx];

        int mask = 1 << bit_offset(cache_block);
        bit_info_[info_idx] = data & ~mask;
    }

    // Returns true when the cache block is pinned.
    bool isBlockPinned(uint64_t cache_block) const
    {
        return block_pin_count_[cache_block];
    }

    // Returns true when the cache block is ready.
    bool isBlockReady(uint64_t cache_block) const
    {
        return !read_block_bit(cache_block);
    }

private:
    int info_index(uint64_t cache_block) const
    {
        return cache_block / 32;
    }

    int bit_offset(uint64_t cache_block) const
    {
        return cache_block % 32;
    }

    bool read_block_bit(uint64_t cache_block) const
    {
        int info_idx = info_index(cache_block);
        uint32_t data = bit_info_[info_idx];

        return (data >> bit_offset(cache_block) ) & 1;
    }

    int info_size_;
    uint32_t* bit_info_;
    int* block_pin_count_;
};

/****************************************************
* ListingCache
*****************************************************/
class ListingCache
{
    size_t cacheSize_;

    uint32_t* block_cache_;

    typedef std::list<std::pair<int, uint64_t> > LruList;

    typedef rde::hash_map<uint64_t, LruList::iterator> CacheMap;

    CacheBlockInfo cache_block_info_;

    boost::mutex query_mutex_;

    // Stores indexes into the block cache, in LRU order.
    LruList lru_list_;

    // Maps a block number to an array slot in the block cache.
    CacheMap cache_map_;

public:
    ListingCache(size_t cacheSize = 1024)
            :cacheSize_(cacheSize)
            ,cache_block_info_(cacheSize)
    {
        block_cache_ = new uint32_t[BLOCK_SIZE * cacheSize_ / sizeof(uint32_t)];

        for (uint64_t i = 0; i < cacheSize; ++i) 
        {
            lru_list_.push_back(make_pair(i, 0));
        }
    }

    ~ListingCache()
    {
        delete [] block_cache_;
    }

    bool applyBlocks(uint64_t starting_block_num, uint64_t ending_block_num)
    {
        boost::mutex::scoped_lock lock(query_mutex_);
        bool ret = true;
        for (uint64_t block_num = starting_block_num; block_num < ending_block_num; ++block_num)
        {
            // Our block is not in the cache, need to bring it in, and evict someone (unless we have't filled the cache yet).
            if (cache_map_.find(block_num) == cache_map_.end())
            {
                // First check whether the cache block is not used and thus can be safely invalidated.
                // Otherwise, we need to find a different block to evict, in order of least recently used.
                LruList::iterator lru_list_itr = lru_list_.begin();
                while (lru_list_itr != lru_list_.end() && cache_block_info_.isBlockPinned(lru_list_itr->first))
                {
                    ++lru_list_itr;
                }
                if(lru_list_itr == lru_list_.end())
                {
                    ///cache is not enough
                    ret = false;
                    continue;
                }

                // Evict someone from the cache.
                if ((size_t)cache_map_.size() == cacheSize_)
                {
                    uint64_t evicted_block = lru_list_itr->second;
                    int invalidated_cache_block = lru_list_itr->first;

                    cache_block_info_.unsetBlock(invalidated_cache_block);
                    cache_map_.erase(evicted_block);
                }

                int cache_block = moveToBack(lru_list_itr, block_num);
                cache_block_info_.pinBlock(cache_block);
                cache_block_info_.setBlock(cache_block);
            }
            else
            {
                // Since we're accessing a block, need to move it to the back of the LRU list.
                int cache_block = moveToBack(cache_map_[block_num], block_num);

                // Block is already in the cache, pin it so it doesn't get evicted.
                cache_block_info_.pinBlock(cache_block);
            }
        }
        return ret;
    }

    // Assumes that 'block_num' has been previously queued by a call to QueueBlocks().
    // If the 'block_num' is in the cache map and it's ready, then we can just return it.
    // If the 'block_num' is in the cache map, but is not marked as ready, then it must be transferring from disk. Wait for completion.
    uint32_t* getBlock(IndexInput* pIndexInput, uint64_t block_num)
    {
        if(cache_map_.find(block_num) == cache_map_.end())
            return NULL;
        int cache_block;
        {
            boost::mutex::scoped_lock lock(query_mutex_);

            // The block should be in the cache already.
            cache_block = moveToBack(cache_map_[block_num], block_num);
        }

        uint32_t* buffer = block_cache_ + (cache_block * BLOCK_SIZE/ sizeof(uint32_t));

        // Only do this the first time we load the block from disk.
        if (!cache_block_info_.isBlockReady(cache_block))
        {
            pIndexInput->readInternal((char *)buffer, BLOCK_SIZE);
            cache_block_info_.unsetBlock(cache_block);
        }

        return buffer;
    }

    // Unpins the block. Note that for a block to be unpinned, every list sharing this block must unpin it.
    // This handles the case when a block is shared by several lists, which occurs in adjacent lists.
    void freeBlock(uint64_t block_num)
    {
        if(cache_map_.find(block_num) != cache_map_.end())
        {
            int cache_block = cache_map_[block_num]->first;
            cache_block_info_.unpinBlock(cache_block);
        }
    }

    int moveToBack(LruList::iterator lru_list_itr, uint64_t block_num)
    {
        int cache_block = lru_list_itr->first;
        lru_list_.erase(lru_list_itr);
        lru_list_.push_back(make_pair(cache_block, block_num));

        // Cache map should update its' iterator so that it points to the last element that we just inserted.
        LruList::iterator new_lru_list_itr = --lru_list_.end();  
        cache_map_[block_num] = new_lru_list_itr;
        return new_lru_list_itr->first;
    }

};

}
NS_IZENELIB_IR_END

#endif

