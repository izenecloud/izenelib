#ifndef _IZENELIB_AM_SUCCINCT_HP_ARRAY_HPP
#define _IZENELIB_AM_SUCCINCT_HP_ARRAY_HPP

#include <am/succinct/constants.hpp>

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>

NS_IZENELIB_AM_BEGIN

namespace succinct
{

#define HUGE_PROTECTION (PROT_READ | PROT_WRITE)
#define HUGE_FLAGS (MAP_HUGETLB | MAP_ANONYMOUS | MAP_PRIVATE)

template <class T>
class HPArray
{
public:
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

    HPArray() : use_hp_(true), size_(), capacity_(), data_() {}

    HPArray(size_type size)
    {
        resize(size);
    }

    HPArray(const std::vector<value_type>& vec)
    {
        assign(vec);
    }

    ~HPArray()
    {
        clear();
    }

    size_type capacity()
    {
        return capacity_;
    }

    size_type size()
    {
        return size_;
    }

    value_type* data()
    {
        return data_;
    }
    const value_type* data() const
    {
        return data_;
    }

    reference operator[](size_type pos)
    {
        return data_[pos];
    }
    const_reference operator[](size_type pos) const
    {
        return data_[pos];
    }

    void clear()
    {
        if (data_)
        {
            if (use_hp_)
            {
                munmap(data_, sizeof(value_type) * capacity_);
            }
            else
            {
                free(data_);
            }
            size_ = 0;
            capacity_ = 0;
            data_ = 0;
        }
    }

    void resize(size_type size)
    {
        if (size > capacity_)
        {
            bool new_use_hp = true;
            size_type new_capacity = std::max(size, capacity_ * 2);
            value_type* new_data = (value_type*)mmap(NULL, sizeof(value_type) * new_capacity, HUGE_PROTECTION, HUGE_FLAGS, 0, 0);
            if (new_data == MAP_FAILED)
            {
                std::cout << "mmap was not successful" << std::endl;
                new_use_hp = false;
                new_data = (value_type*)malloc(sizeof(value_type) * new_capacity);
            }
            if (data_)
            {
                memcpy(new_data, data_, sizeof(value_type) * size_);
                if (use_hp_)
                {
                    munmap(data_, sizeof(value_type) * capacity_);
                }
                else
                {
                    free(data_);
                }
            }
            use_hp_ = new_use_hp;
            capacity_ = new_capacity;
            data_ = new_data;
        }
        size_ = size;
    }

    void assign(const std::vector<value_type>& vec)
    {
        if (vec.size() > capacity_)
        {
            bool new_use_hp = true;
            if (data_)
            {
                if (use_hp_)
                {
                    munmap(data_, sizeof(value_type) * capacity_);
                }
                else
                {
                    free(data_);
                }
            }
            capacity_ = std::max(vec.size(), capacity_ * 2);
            data_ = (value_type*)mmap(NULL, sizeof(value_type) * capacity_, HUGE_PROTECTION, HUGE_FLAGS, 0, 0);
            if (data_ == MAP_FAILED)
            {
                std::cout << "mmap was not successful" << std::endl;
                new_use_hp = false;
                data_ = (value_type*)malloc(sizeof(value_type) * capacity_);
            }
            use_hp_ = new_use_hp;
        }
        size_ = vec.size();
        memcpy(data_, &vec[0], sizeof(value_type) * size_);
    }

    void swap(HPArray& other)
    {
        std::swap(use_hp_, other.use_hp_);
        std::swap(size_, other. size_);
        std::swap(capacity_, other.capacity_);
        std::swap(data_, other.data_);
    }

    void save(std::ostream& os) const
    {
        os.write((const char*)&size_, sizeof(size_));
        os.write((const char*)data_, sizeof(value_type) * size_);
    }
    void load(std::istream& is)
    {
        is.read((char*)&size_, sizeof(size_));
        resize(size_);
        is.read((char*)data_, sizeof(value_type) * size_);
    }

private:
    bool use_hp_;
    size_type size_;
    size_type capacity_;
    value_type* data_;
};

}

NS_IZENELIB_AM_END

#endif
