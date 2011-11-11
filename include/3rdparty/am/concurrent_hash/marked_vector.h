#ifndef CONCURRENT_MARKED_PTR_H
#define CONCURRENT_MARKED_PTR_H
#include <stdint.h>
#include <utility>
#include <assert.h>

namespace concurrent{

template <typename T>
class marked_vector
{
    // warning: this is a smart pointer
    typedef marked_vector<T> marked_vector_t;
public:
    explicit marked_vector(const int& num):size_(num),ptr_(new T[num]) {}
    bool try_mark()
    {
        T* const old = ptr_;
        if (reinterpret_cast<T*>(
                    (reinterpret_cast<uintptr_t>(ptr_) & 1)
                ))
        {
            return false;
        }
        T* const new_ptr = reinterpret_cast<T*>(
                               reinterpret_cast<uintptr_t>(ptr_) | 1
                           );
        return cas(old, new_ptr);
    }
    bool is_marked()const
    {
        return reinterpret_cast<T*>(
                   (reinterpret_cast<uintptr_t>(ptr_) & 1)
               );
    }
    T* get()const
    {
        return reinterpret_cast<T*>(
                   reinterpret_cast<uintptr_t>(ptr_)	& (static_cast<uintptr_t>(-1) - 1)
               );
    }
    bool cas(T* const expected, T* const new_ptr)
    {
        return __sync_bool_compare_and_swap(&ptr_, expected, new_ptr);
    }
    void rebind(T* const new_ptr)
    {
        ptr_ = new_ptr;
    }
    void swap(marked_vector_t& other)
    {
        std::swap(size_, other.size_);
        //std::swap(ptr_, other.ptr_);
        T* old;
        do
        {
            old = ptr_;
        }
        while (!__sync_bool_compare_and_swap(&ptr_,old,other.ptr_));
        other.ptr_ = old;
    }
    T& operator[](const int index)
    {
        assert(0 <= index);
        assert(static_cast<size_t>(index) < size_);
        return get()[index];
    }
    size_t size()const
    {
        return  size_;
    }
    const T& operator[](const int index)const
    {
        return get()[index];
    }
    ~marked_vector()
    {
        delete [] get();
    }
private:
    marked_vector();
    marked_vector(const marked_vector<T>&);
    marked_vector& operator=(const marked_vector<T>&);
    size_t size_;
    T* ptr_;
};

}
#endif
