/**
* @file        slfvector.h
* @version     SF1 v5.0
* @brief efficient simple vector
*/

#ifndef SLFVECTOR_H
#define SLFVECTOR_H

#include<stdexcept>
namespace izenelib{ namespace am { namespace concurrent {
        template<typename T>
        class slfvector{
            public:
                static size_t const INITIAL_CAPACITY = 32;
                static size_t const INITIAL_SIZE = 8;
                static size_t const init_bit = 3;
                slfvector();
                ~slfvector();
                void push_back(const T&);
                bool empty()const;
                size_t size()const;
                const T& at(size_t n)const;
                T& at(size_t n);
                void reserve(size_t s);

            private:
                inline T& internal_at(size_t n);
                inline size_t highest_bit(size_t n);
            private:
                T** data_;
                size_t size_;
        };

        template<typename T>
        slfvector<T>::slfvector():size_(0){
            data_ = new T*[INITIAL_CAPACITY];
            data_[0] = new T[INITIAL_SIZE];
            for(size_t i = 1; i < INITIAL_CAPACITY; ++i)
                data_[i] = NULL;
        }

        template<typename T>
        slfvector<T>::~slfvector(){
            for(size_t i = 0; i < INITIAL_CAPACITY; ++i)
                if(data_[i] != NULL)
                    delete[] data_[i];
            delete[] data_;
        }

        template<typename T>
        void slfvector<T>::push_back(const T& elem){
            size_t bucket = highest_bit(size_ + INITIAL_SIZE) - init_bit;
            if(data_[bucket] == NULL){
                size_t bucket_size = INITIAL_SIZE * (1 << bucket + 1);
                data_[bucket] = new T[bucket_size];
            }
            internal_at(size_) = elem;
            ++size_;
        }

        template<typename T>
        bool slfvector<T>::empty()const{
            return size_ == 0;
        }

        template<typename T>
        size_t slfvector<T>::size()const{
            return size_;
        }

        template<typename T>
        inline const T& slfvector<T>::at(size_t n)const{
            if(n < 0 || n >= size_)
                throw std::out_of_range("out of vector range.");
            return internal_at(n);
        }

        template<typename T>
        inline T& slfvector<T>::at(size_t n){
            if(n < 0 || n >= size_)
                throw std::out_of_range("out of vector range.");
            return internal_at(n);
        }

        //resize the vector, delete the unnecessory memory
        template<typename T>
        void slfvector<T>::reserve(size_t s){
            size_t expect_bucket = highest_bit(s + INITIAL_SIZE - 1) - init_bit;
            if(expect_bucket < 0)
                expect_bucket = 0;
            size_ = s;
            for( ; ++expect_bucket < INITIAL_CAPACITY; )
                if(data_[expect_bucket] != NULL){
                    delete[] data_[expect_bucket];
                    data_[expect_bucket] = NULL;
                }
        }

        template<typename T>
        inline T& slfvector<T>::internal_at(size_t n){
            size_t pos = n + INITIAL_SIZE;
            size_t hi_bit = highest_bit(pos);
            size_t idx = pos ^ (1 << hi_bit);
            return data_[hi_bit - init_bit][idx];
        }

        template<typename T>
        inline size_t slfvector<T>::highest_bit(size_t n){
            int b = 0;
             __asm__ __volatile__(
                "movl %0, %%eax\n\
                movl %1, %%ebx\n\
                bsr %%eax, %%ebx\n\
                movl %%ebx, %1"
                :
                :"g"(n),"g"(b)
            );
            return b;
        }
}}}

#endif
