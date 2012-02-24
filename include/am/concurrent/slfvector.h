/**
* @file        slfvector.h
* @version     SF1 v5.0
* @brief efficient simple vector
*/

#ifndef SLFVECTOR_H
#define SLFVECTOR_H

#include<stdexcept>
#include<boost/serialization/access.hpp>
#include<boost/serialization/array.hpp>
#include<stdint.h>
#include <iostream>
namespace izenelib {
namespace am {
namespace concurrent {

template<typename T>
class slfvector
{
public:
    static uint32_t const INITIAL_CAPACITY = 32;
    static uint32_t const INITIAL_SIZE = 8;
    static uint32_t const init_bit = 3;
    slfvector();
    slfvector(uint32_t n);
    slfvector(const slfvector<T>& orig);
    ~slfvector();
    slfvector<T>& operator=(const slfvector<T>&);
    friend inline bool operator==(const slfvector<T>& lhs, const slfvector<T>& rhs) {
        return (lhs.size_ == rhs.size_) && (lhs.data_ == rhs.data_);
    }
    friend inline bool operator!=(const slfvector<T>& lhs, const slfvector<T>& rhs) {
        return !(lhs == rhs);
    }
    void push_back(const T&);
    bool empty()const;
    uint32_t size()const;
    void swap(slfvector<T>& vec);
    const T& at(uint32_t n)const;
    T& at(uint32_t n);
    const T& operator[](const uint32_t n)const;
    T& operator[](const uint32_t n);
    void resize(uint32_t s);
    void reserve(uint32_t s) {}
    inline T& back();
    inline const T& back()const;
    void clear();

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);
private:
    inline T& internal_at(uint32_t n);
    inline const T& internal_at(uint32_t n)const;
    uint32_t highest_bit(uint32_t n)const;
    void init_memory(uint32_t n);
private:
    T** data_;
    uint32_t size_;
};

template<typename T>
slfvector<T>::slfvector():size_(0)
{
    data_ = new T*[INITIAL_CAPACITY];
    for(uint32_t i = 0; i < INITIAL_CAPACITY; ++i)
        data_[i] = NULL;
}

template<typename T>
slfvector<T>::slfvector(const slfvector<T>& orig):size_(orig.size_) {
    init_memory(size_);
    for(uint32_t idx = 0; idx < size_; ++idx) {
        internal_at(idx) = orig.internal_at(idx);
    }
}

template<typename T>
slfvector<T>::slfvector(uint32_t n):size_(n) {
    init_memory(n);
}

template<typename T>
slfvector<T>::~slfvector()
{
    for(uint32_t i = 0; i < INITIAL_CAPACITY; ++i)
        if(data_[i] != NULL)
            delete[] data_[i];
    delete[] data_;
}

template<typename T>
slfvector<T>& slfvector<T>::operator=(const slfvector<T>& rhs) {
    if(*this == rhs)
        return *this;
    clear();
    init_memory(rhs.size_);
    for(uint32_t idx = 0; idx < rhs.size_; ++idx) {
        internal_at(idx) = rhs.internal_at(idx);
    }
    size_ = rhs.size_;
    return *this;
}


template<typename T>
void slfvector<T>::push_back(const T& elem)
{
    uint32_t bucket = highest_bit(size_ + INITIAL_SIZE) - init_bit;
    if(data_[bucket] == NULL)
    {
        uint32_t bucket_size = INITIAL_SIZE * (1 << bucket);
        data_[bucket] = new T[bucket_size];
    }
    internal_at(size_) = elem;
    ++size_;
}

template<typename T>
bool slfvector<T>::empty()const
{
    return size_ == 0;
}

template<typename T>
uint32_t slfvector<T>::size()const
{
    return size_;
}

template<typename T>
void slfvector<T>::swap(slfvector<T>& vec) {
    T** temp_ptr = vec.data_;
    vec.data_ = data_;
    data_ = temp_ptr;
    uint32_t temp_size = vec.size_;
    vec.size_ = size_;
    size_ = temp_size;
}

template<typename T>
inline const T& slfvector<T>::at(uint32_t n)const
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

template<typename T>
inline T& slfvector<T>::at(uint32_t n)
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

template<typename T>
inline const T& slfvector<T>::operator[](const uint32_t n)const
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

template<typename T>
inline T& slfvector<T>::operator[](const uint32_t n)
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

template<typename T>
inline T& slfvector<T>::back() {
    if(size_ == 0)
        throw std::out_of_range("back() out of vector range:0.");
    return internal_at(size_ - 1);
}

template<typename T>
inline const T& slfvector<T>::back()const {
    if(size_ == 0)
        throw std::out_of_range("back() out of vector range:0.");
    return internal_at(size_ - 1);
}

//resize the vector
template<typename T>
void slfvector<T>::resize(uint32_t s)
{
    if(s < 0 || s == size_)
        return;
    const int32_t expect_bucket = highest_bit(s + INITIAL_SIZE - 1) - init_bit;
    const int32_t current_bucket = highest_bit(size_ + INITIAL_SIZE - 1) - init_bit;
    if(current_bucket > expect_bucket)
    {
        size_ = s;
        for(int32_t bucket = expect_bucket + 1 ; bucket <= current_bucket; ++bucket ) {
            delete[] data_[bucket];
            data_[bucket] = NULL;
        }
    }
    else
    {
        for(int32_t bucket = current_bucket + 1; bucket <= expect_bucket; ++bucket )
        {
            uint32_t bucket_size = INITIAL_SIZE * (1 << bucket);
            data_[bucket] = new T[bucket_size];
        }
        size_ = s;
    }
}

template<typename T>
void slfvector<T>::clear()
{
    size_ = 0;
    for(uint32_t i = 0; i < INITIAL_CAPACITY; ++i)
        if(data_[i] != NULL) {
            delete[] data_[i];
            data_[i] = NULL;
        }
}

template<typename T> template<typename Archive>
void slfvector<T>::serialize(Archive& ar, const unsigned int version) {
    ar & size_;
    int32_t bucket = highest_bit(size_ + INITIAL_SIZE - 1) - init_bit;
    for(int32_t i = 0; i <= bucket; ++i) {
        uint32_t bucket_size = INITIAL_SIZE * (1 << i);
        if(data_[i] == NULL)
            data_[i] = new T[bucket_size];
        ar & boost::serialization::make_array(data_[i], bucket_size);
    }
}


template<typename T>
inline const T& slfvector<T>::internal_at(uint32_t n)const
{
    uint32_t pos = n + INITIAL_SIZE;
    uint32_t hi_bit = highest_bit(pos);
    uint32_t idx = pos ^ (1 << hi_bit);
    return data_[hi_bit - init_bit][idx];
}

template<typename T>
inline T& slfvector<T>::internal_at(uint32_t n)
{
    uint32_t pos = n + INITIAL_SIZE;
    uint32_t hi_bit = highest_bit(pos);
    uint32_t idx = pos ^ (1 << hi_bit);
    return data_[hi_bit - init_bit][idx];
}

template<typename T>
void slfvector<T>::init_memory(uint32_t n) {
    data_ = new T*[INITIAL_CAPACITY];
    int32_t bucket = highest_bit(n + INITIAL_SIZE - 1) - init_bit;
    for(int32_t i = 0; i <= bucket; ++i)
    {
        uint32_t bucket_size = INITIAL_SIZE * (1 << bucket);
        data_[i] = new T[bucket_size];
    }
    for(uint32_t i = bucket + 1; i < INITIAL_CAPACITY; ++i)
    {
        data_[i] = NULL;
    }
}

template<typename T>
uint32_t slfvector<T>::highest_bit(uint32_t n)const
{
    uint32_t b = 0;
#ifdef __x86_64__
    __asm__ __volatile__(
        "bsrl %1, %0"
        :"=r"(b)
        :"r"(n)
    );
#else
    int b_start = 0;
    int b_end = sizeof(uint32_t) * 8;
    while(b_start != b_end)
    {
        int mid = (b_start + b_end) >> 1;
        if((n>>mid) != 0)
        {
            if(b_start == b_end - 1 )
                ++b_start;
            else
                b_start = mid;
        }
        else
            b_end = mid;

    }
    b = b_start - 1;
#endif
    return b;
}

}
}
}
#endif
