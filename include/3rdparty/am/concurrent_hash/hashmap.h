#ifndef CONCURRENT_HASHMAP_H
#define CONCURRENT_HASHMAP_H

#include <cstddef>
#include "locks.h"
#include "hash.h"
#include "marked_vector.h"
#include <stdexcept>
#include <vector>
#include "atomic.h"

namespace concurrent{

class not_found : public std::logic_error
{
public:
    not_found():std::logic_error("not found") {}
};



template <typename key, typename value>
class hashmap
{
public:
    struct bucket_t
    {
        typedef std::pair<const key,value> pair_t;
        pair_t kvp_;
        bucket_t* next_;
        bucket_t(const pair_t& k):kvp_(k),next_(NULL) {}
    };
    
    struct unsafe_iterator
    {
        bucket_t* p_bucket;
        std::size_t index;
    };
    
    hashmap(const uint32_t lk = 8,const uint32_t size = 11, const uint32_t mc = 1)
            :lock_qty_(lk),
            locks_(new detail::spin_lock[lock_qty_]),
            max_chain_(mc),
            bucket_vector_(size)
    {
        for (size_t i=0;i < bucket_vector_.size();++i)
        {
            bucket_vector_[i] = NULL;
        }
    }
    bool insert(const std::pair<const key,value>& kvp)
    {
        const std::size_t hashed = hash_value(kvp.first);
retry:
        const std::size_t got_size = bucket_vector_.size();
        detail::scoped_lock<detail::spin_lock> lk(locks_[hashed % got_size % lock_qty_]);
        if (got_size != bucket_vector_.size())
        {
            lk.unlock();
            goto retry;
        }

        bucket_t **prev = const_cast<bucket_t**>(&bucket_vector_[hashed % got_size]);
        bucket_t *curr = *prev;

        uint32_t chain_counter = 0;
        while (curr != NULL)
        {
            if (curr->kvp_.first == kvp.first)
            {
                return false;
            }
            prev = &curr->next_;
            curr = curr->next_;
            ++chain_counter;
        }

        // allocate and combine
        *prev = new bucket_t(kvp);

        if (max_chain_ < chain_counter)
        {
            lk.unlock();
            buckets_extend();
        }
        return true;
    }
    bool contains(const key& k)const
    {
        const std::size_t hashed = hash_value(k);
retry:
        const std::size_t got_size = bucket_vector_.size();
        detail::scoped_lock<detail::spin_lock>
        lk(locks_[hashed % got_size % lock_qty_]);
        if (got_size != bucket_vector_.size())
        {
            lk.unlock();
            goto retry;
        }
        const bucket_t* target =
            const_cast<const bucket_t*>(bucket_vector_[hashed % bucket_vector_.size()]);
        while (target != NULL)
        {
            if (target->kvp_.first == k)
            {
                return true;
            }
            target = target->next_;
        }
        return false;
    }
    value* get_unsafe(const key& k)const
    {
        const std::size_t hashed = hash_value(k);
        bucket_t* target = &bucket_vector_[hashed % bucket_vector_.size()];
        while (target != NULL && target->kvp != NULL)
        {
            if (target->kvp->first == k)
            {
                return &target->kvp->second;
            }
            target = target->next_;
        }
        return NULL;
    }
    value get(const key& k)const throw(not_found)
    {
        const std::size_t hashed = hash_value(k);
retry:
        const size_t got_size = bucket_vector_.size();

        detail::scoped_lock<detail::spin_lock>
        lk(locks_[hashed % got_size % lock_qty_]);
        if (got_size != bucket_vector_.size())
        {
            lk.unlock();
            goto retry;
        }
        const bucket_t* target =
            const_cast<const bucket_t*>(bucket_vector_[hashed % got_size]);

        while (target != NULL)
        {
            if (target->kvp_.first == k)
            {
                return target->kvp_.second;
            }
            target = target->next_;
        };
        throw not_found();
    }
    
    bool get(const key& k, value& v)const
    {
        const std::size_t hashed = hash_value(k);
retry:
        const size_t got_size = bucket_vector_.size();

        detail::scoped_lock<detail::spin_lock>
        lk(locks_[hashed % got_size % lock_qty_]);
        if (got_size != bucket_vector_.size())
        {
            lk.unlock();
            goto retry;
        }
        const bucket_t* target =
            const_cast<const bucket_t*>(bucket_vector_[hashed % got_size]);

        while (target != NULL)
        {
            if (target->kvp_.first == k)
            {
                v = target->kvp_.second;
                return true;
            }
            target = target->next_;
        };
        return false;
    }
    
    bool remove(const key& k)
    {
        const std::size_t hashed = hash_value(k);
retry:
        const size_t got_size = bucket_vector_.size();

        detail::scoped_lock<detail::spin_lock>
        lk(locks_[hashed % got_size % lock_qty_]);
        if (got_size != bucket_vector_.size())
        {
            lk.unlock();
            goto retry;
        }

        bucket_t **pred =
            const_cast<bucket_t**>(&bucket_vector_[hashed % got_size]);
        bucket_t *curr = *pred;
        while (curr != NULL)
        {
            if (curr->kvp_.first == k)
            {
                *pred = curr->next_;
                delete curr;
                return true;
            }
            pred = &curr->next_;
            curr = curr->next_;
        };
        return false;
    }
    void clear()
    {
        for (size_t i=0; i < lock_qty_; ++i)
        {
            locks_[i].lock();
        }
        for (size_t i=0; i < bucket_vector_.size(); i++)
        {
            bucket_t* ptr = const_cast<bucket_t*>(bucket_vector_[i]), *old_next;
            while (ptr != NULL)
            {
                old_next = ptr->next_;
                delete ptr;
                ptr = old_next;
            }
            bucket_vector_[i] = NULL;
        }
        for (size_t i=0; i < lock_qty_; ++i)
        {
            locks_[i].unlock();
        }
    }
    
    ~hashmap()
    {
        clear();
        delete[] locks_;
    }
    
    unsafe_iterator unsafe_begin()
    {
        unsafe_iterator it;
        it.index = 0;
        if(bucket_vector_.size()==0)
        {
            it.p_bucket = NULL;
        }
        else
        {
            it.p_bucket = const_cast<bucket_t*>(bucket_vector_[0]);
        }
        return it;
    }
    
    bool get_and_next(unsafe_iterator& it, std::pair<key, value>& kvp)
    {
        if(it.p_bucket!=NULL)
        {
            kvp = it.p_bucket->kvp_;
            it.p_bucket = it.p_bucket->next_;
            return true;
        }
        else
        {
            std::size_t next_index = it.index+1;
            if(next_index<bucket_vector_.size())
            {
                unsafe_iterator new_it;
                new_it.index = next_index;
                new_it.p_bucket = const_cast<bucket_t*>(bucket_vector_[next_index]);
                return get_and_next(new_it, kvp);
            }
            else
            {
                return false;
            }
        }
    }
    
    void dump()const
    {
        for (int i=0; i < lock_qty_; ++i)
        {
            locks_[i].lock();
        }
        bucket_t* const * const array = const_cast<bucket_t *const * const>
                                        (bucket_vector_.get());
        for (int i=0; i < bucket_vector_.size(); ++i)
        {
            const bucket_t* ptr = array[i];
            //std::cout << "[" << i << "]->";
            while (ptr != NULL)
            {
                ptr = ptr->next;
            }
        }
        for (int i=0; i < lock_qty_; ++i)
        {
            locks_[i].unlock();
        }
    }
private:
    bool buckets_extend()
    {
        if (!bucket_vector_.try_mark())
        {
            return false; // other one already extending bucket
        }
        const std::size_t oldsize = bucket_vector_.size();
        const std::size_t newsize = oldsize * 2 + 1;
        marked_vector<volatile bucket_t*> new_buckets(newsize);
        bool locked[lock_qty_];

        for (size_t i=0; i < lock_qty_; ++i)
        {
            locked[i] = false;
        }
        for (size_t i=0; i < newsize; ++i)
        {
            new_buckets[i] = NULL;
        }
        for (size_t i=0; i < oldsize; ++i)
        {
            const int target_lock = i % lock_qty_;
            if (!locked[target_lock]) // get lock
            {
                locks_[target_lock].lock();
                locked[target_lock] = true;
            }

            bucket_t *curr = const_cast<bucket_t*>(bucket_vector_[i]);

            while (curr != NULL)
            {
                const std::size_t new_place = hash_value(curr->kvp_.first) % newsize;
                bucket_t* const old_next = const_cast<bucket_t* const>(curr->next_);
                curr->next_ = const_cast<bucket_t*>(new_buckets[new_place]);
                new_buckets[new_place] = curr;
                curr = old_next;
            }
        }
        // assert all lock gained
        bucket_vector_.swap(new_buckets);
        for (size_t i=0; i < lock_qty_; ++i)
        {
            locks_[i].unlock();
        }
        return true;
    }
    
    
    const std::size_t lock_qty_;
    mutable detail::spin_lock* locks_;
    const uint32_t max_chain_;
    marked_vector<volatile bucket_t*> bucket_vector_;
};

}
#endif
