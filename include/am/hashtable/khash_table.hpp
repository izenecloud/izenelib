/*
 * =====================================================================================
 *
 *       Filename:  khash_table.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2013年05月21日 13时46分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */
#ifndef _IZENELIB_AM_KHASH_TABLE_HPP_
#define _IZENELIB_AM_KHASH_TABLE_HPP_
#include "types.h"
#include <cstdio>
#include <string>
#include "util/hashFunction.h"
#include "util/string/kstring.hpp"

NS_IZENELIB_AM_BEGIN

template<
class KEY_T,
      class VALUE_T
      >
class KIntegerHashTable
{
    typedef struct _NODE_
    {
        union{
            uint8_t key_[sizeof(KEY_T)];
            KEY_T key_type_value_;
        };
        union{
            uint8_t value_[sizeof(VALUE_T)];
            VALUE_T value_type_value_;
        };
        union{
            uint8_t next_[sizeof(uint32_t)];
            uint32_t next_type_value_;
        };

        _NODE_(const KEY_T& k, const VALUE_T& v, uint32_t ne = -1)
        {
            key_type_value_ = k;
            value_type_value_ = v;
            next_type_value_ = ne;
        }

        _NODE_()
        {
            next_type_value_ = -1;
        }

        KEY_T key()const
        {
            return key_type_value_;
        }

        VALUE_T value()const
        {
            return value_type_value_;
        }

        KEY_T& key()
        {
            return key_type_value_;
        }

        VALUE_T& value()
        {
            return value_type_value_;
        }

        uint32_t& next()
        {
            return next_type_value_;
        }

        uint32_t next()const
        {
            return next_type_value_;
        }

        bool operator == (const struct _NODE_& o)const
        {
            return key() == o.key();
        }

        _NODE_& operator = (const struct _NODE_& o)
        {
            key() = o.key();
            value() = o.value();
            next() = o.next();
            return *this;
        }
    } node_t;

    node_t* nodes_;
    uint32_t nodes_num_;
    uint32_t* entry_;
    uint32_t entry_size_;
    uint32_t avai_i_;
    uint32_t size_;

    uint32_t expansion_(uint32_t len)
    {
        if (len < 1000)return 2*len;
        if (len < 10000)return 1.5*len;
        return 1.1*len;
    }

    uint32_t available_()
    {
        if (size_+2 < nodes_num_)
            return avai_i_;
        uint32_t nn = expansion_(nodes_num_);
        node_t* n = new node_t[nn];
        memcpy(n, nodes_, nodes_num_*sizeof(node_t));
        for ( uint32_t i=nodes_num_-1; i<nn-1; ++i)
            n[i].next() = i+1;
        delete[] nodes_;
        nodes_ = n;
        nodes_num_ = nn;
        return avai_i_;
    }

public:
    KIntegerHashTable(uint32_t ent_size = 100000, uint32_t element_num = 50000)
    {
        nodes_ = new node_t[element_num];
        entry_ = new uint32_t[ent_size];
        for ( uint32_t i=0; i<element_num-1; ++i)
            nodes_[i].next() = i+1;
        for ( uint32_t i=0; i<ent_size; ++i)
            entry_[i] = -1;
        avai_i_ = 0;
        nodes_num_ = element_num;
        entry_size_ = ent_size;
        size_ = 0;
    }

    ~KIntegerHashTable()
    {
        delete[] nodes_;
        delete[] entry_;
    }

    void reserve(uint32_t ent_size, uint32_t element_num)
    {
        if (ent_size > entry_size_)
        {
            uint32_t* e = new uint32_t[ent_size];
            memcpy(e, entry_, entry_size_*sizeof(uint32_t));
            delete[] entry_;
            entry_ = e;
            entry_size_ = ent_size;
        }
        if (element_num > nodes_num_)
        {
            node_t* n = new node_t[element_num];
            memcpy(n, nodes_, nodes_num_*sizeof(node_t));
            delete[] nodes_;
            for ( uint32_t i=nodes_num_-1; i<element_num-1; ++i)
                n[i].next() = i+1;
            nodes_ = n;
            nodes_num_ = element_num;
        }
    }

    void insert(const KEY_T& k, const VALUE_T& v)
    {
        uint32_t ei = k%entry_size_;
        uint32_t a = available_();
        if (entry_[ei]!=(uint32_t)-1)
        {
            uint32_t next = entry_[ei];
            while(1)
            {
                IASSERT(next < nodes_num_);
                if (nodes_[next].key() == k)
                {
                    nodes_[next].value() = v;
                    return;
                }
                if (nodes_[next].next() == (uint32_t)-1)
                    break;
                next = nodes_[next].next();
            }
            IASSERT(next < nodes_num_);
            nodes_[next].next() = a;
        }
        else
            entry_[ei] = a;

        IASSERT(a < nodes_num_);
        avai_i_ = nodes_[a].next();
        nodes_[a] = node_t(k, v);
        size_ ++;
    }

    VALUE_T* find(const KEY_T& k)
    {
        uint32_t ei = k%entry_size_;
        if (entry_[ei] == (uint32_t)-1)
            return NULL;

        uint32_t next = entry_[ei];
        while(1)
        {
            IASSERT(next < nodes_num_);
            if (nodes_[next].key() == k)
                return &nodes_[next].value();

            if (nodes_[next].next() == (uint32_t)-1)
                return NULL;
            next = nodes_[next].next();
        }
        IASSERT(false);
        return NULL;
    }

    bool erase(const KEY_T& k)
    {
        uint32_t ei = k%entry_size_;
        if (entry_[ei] == (uint32_t)-1)
            return false;

        uint32_t next = entry_[ei];
        uint32_t la = -1;
        while(1)
        {
            IASSERT(next < nodes_num_);
            if (nodes_[next].key() == k)
            {
                if (la == (uint32_t)-1)
                    entry_[ei] = nodes_[next].next();
                else
                    nodes_[la].next() = nodes_[next].next();

                nodes_[next].next() = avai_i_;
                avai_i_ = next;
                --size_;
                return true;
            }

            if (nodes_[next].next() == (uint32_t)-1)
                return false;
            la = next;
            next = nodes_[next].next();
        }
        IASSERT(false);
        return false;

    }

    void persistence(const std::string& nm)const
    {
        FILE* f = fopen(nm.c_str(), "w+");
        if (!f)
            throw std::runtime_error("can't open file.");

        fwrite(&nodes_num_, sizeof(nodes_num_), 1, f);
        fwrite(&entry_size_, sizeof(entry_size_), 1, f);
        fwrite(&avai_i_, sizeof(avai_i_), 1, f);
        fwrite(&size_, sizeof(size_), 1, f);
        fwrite(entry_, sizeof(uint32_t)*entry_size_, 1, f);
        fwrite(nodes_, sizeof(node_t)*nodes_num_, 1, f);

        fclose(f);
    }

    void load(const std::string& nm)
    {
        FILE* f = fopen(nm.c_str(), "r");
        if (!f)
            throw std::runtime_error(std::string("can't open file:")+nm);

        if(fread(&nodes_num_, sizeof(nodes_num_), 1, f)!=1)throw std::runtime_error("File read error.");
        if(fread(&entry_size_, sizeof(entry_size_), 1, f)!=1)throw std::runtime_error("File read error.");
        if(fread(&avai_i_, sizeof(avai_i_), 1, f)!=1)throw std::runtime_error("File read error.");
        if(fread(&size_, sizeof(size_), 1, f)!=1)throw std::runtime_error("File read error.");

        delete[] entry_;
        delete[] nodes_;
        entry_ = new uint32_t[entry_size_];
        nodes_ = new node_t[nodes_num_];

        if(fread(entry_, sizeof(uint32_t)*entry_size_, 1, f)!=1)throw std::runtime_error("File read error.");
        if(fread(nodes_, sizeof(node_t)*nodes_num_, 1, f)!=1)throw std::runtime_error("File read error.");

        fclose(f);
    }

    uint32_t size()const
    {
        return size_;
    }

    class iterator
    {
        KIntegerHashTable<KEY_T, VALUE_T>* ptr_;
        uint32_t idx_;
        uint32_t ei_;

    public:
        iterator(KIntegerHashTable<KEY_T, VALUE_T>* ptr = NULL, uint32_t idx = -1, uint32_t ei = 0)
            :ptr_(ptr),idx_(idx),ei_(ei)
        {}

        iterator& operator ++(int)
        {
            if (!ptr_ || idx_ == (uint32_t)-1)
                return *this;

            idx_ = ptr_->nodes_[idx_].next();
            if(idx_ != (uint32_t)-1)
                return *this;

            ei_++;
            while(ei_ < ptr_->entry_size_ &&  ptr_->entry_[ei_] == (uint32_t)-1)
                ei_++;

            if (ei_ >= ptr_->entry_size_)
                return *this;

            idx_ = ptr_->entry_[ei_];
            return *this;
        }

        iterator& operator ++()
        {
            return (*this)++;
        }

        KEY_T* key()
        {
            if (idx_ == (uint32_t)-1)
                return NULL;
            return &(ptr_->nodes_[idx_].key());
        }

        VALUE_T* value()
        {
            if (idx_ == (uint32_t)-1)
                return NULL;
            return &(ptr_->nodes_[idx_].value());
        }

        bool operator == (const iterator& o)const
        {
            return ptr_ == o.ptr_ && idx_ == o.idx_;
        }

        bool operator != (const iterator& o)const
        {
            return ptr_ != o.ptr_ || idx_ != o.idx_;
        }
    };

    iterator begin()
    {
        uint32_t ei = 0;
        while(ei < entry_size_ &&  entry_[ei] == (uint32_t)-1)
            ei++;
        if (ei >= entry_size_)
            return end();
        return iterator(this, entry_[ei], ei);
    }

    iterator end()
    {
        return iterator(this, -1);
    }
};

template<
class KEY_T,
      class VALUE_T
      >
class KStringHashTable
{
    KIntegerHashTable<uint64_t, VALUE_T> table_;

    uint64_t hash_(const std::string& str)const
    {
        return izenelib::util::HashFunction<std::string>::generateHash64(str);
    }

    uint64_t hash_(const izenelib::util::KString& kstr)const
    {
        std::string str = kstr.get_bytes("utf-8");
        return hash_(str);
        //return izenelib::util::HashFunction<std::string>::generateHash64((char*)kstr.get_bytes(), kstr.length()*sizeof(uint16_t));
    }

public:
    KStringHashTable(uint32_t ent_size = 100000, uint32_t element_num = 50000)
        :table_(ent_size, element_num)
    {}

    void reserve(uint32_t ent_size, uint32_t element_num)
    {
        table_.reserve(ent_size, element_num);
    }

    void insert(const KEY_T& k, const VALUE_T& v)
    {
        uint64_t h = hash_(k);
        table_.insert(h, v);
    }

    void insert(const uint64_t& k, const VALUE_T& v)
    {
        table_.insert(k, v);
    }

    VALUE_T* find(const KEY_T& k)
    {
        uint64_t h = hash_(k);
        return table_.find(h);
    }
    
    VALUE_T* find(const uint64_t& k)
    {
        return table_.find(k);
    }

    bool erase(const KEY_T& k)
    {
        uint64_t h = hash_(k);
        return table_.erase(h);
    }

    void persistence(const std::string& nm)const
    {
        table_.persistence(nm);
    }

    void load(const std::string& nm)
    {
        table_.load(nm);
    }

    uint32_t size()const
    {
        return table_.size();
    }

    typedef typename KIntegerHashTable<uint64_t, VALUE_T>::iterator iterator;

    iterator begin()
    {
        return table_.begin();
    }

    iterator end()
    {
        return table_.end();
    }

};
NS_IZENELIB_AM_END
#endif


