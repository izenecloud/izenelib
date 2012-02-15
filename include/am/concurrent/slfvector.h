/**
* @file        slfvector.h
* @version     SF1 v5.0
* @brief efficient simple vector
*/

#ifndef SLFVECTOR_H
#define SLFVECTOR_H

#include<stdexcept>
<<<<<<< HEAD
#include<stdint.h>
#include <iostream>
namespace izenelib{ namespace am { namespace concurrent {
        template<typename T>
        class slfvector{
            public:
                static uint32_t const INITIAL_CAPACITY = 32;
                static uint32_t const INITIAL_SIZE = 8;
                static uint32_t const init_bit = 3;
                slfvector();
				slfvector(uint32_t n);
                ~slfvector();
                void push_back(const T&);
                bool empty()const;
                uint32_t size()const;
                const T& at(uint32_t n)const;
                T& at(uint32_t n);
                const T& operator[](const uint32_t n)const;
                T& operator[](const uint32_t n);
                void resize(uint32_t s);
				void clear();

            private:
                inline T& internal_at(uint32_t n);
				inline const T& internal_at(uint32_t n)const;
                uint32_t highest_bit(uint32_t n)const;
            private:
                T** data_;
                uint32_t size_;
        };

        template<typename T>
        slfvector<T>::slfvector():size_(0){
            data_ = new T*[INITIAL_CAPACITY];
            data_[0] = new T[INITIAL_SIZE];
            for(uint32_t i = 1; i < INITIAL_CAPACITY; ++i)
                data_[i] = NULL;
        }

		template<typename T>
		slfvector<T>::slfvector(uint32_t n):size_(n){
			data_ = new T*[INITIAL_CAPACITY];
			uint32_t bucket = highest_bit(size_ + INITIAL_SIZE - 1) - init_bit;
			for(uint32_t i = 0; i <= bucket; ++i){
				uint32_t bucket_size = INITIAL_SIZE * (1 << bucket);
				data_[i] = new T[bucket_size];
			}
			for(uint32_t i = bucket + 1; i < INITIAL_CAPACITY; ++i){
				data_[i] = NULL;
			}
		}

        template<typename T>
        slfvector<T>::~slfvector(){
            for(uint32_t i = 0; i < INITIAL_CAPACITY; ++i)
                if(data_[i] != NULL)
                    delete[] data_[i];
            delete[] data_;
        }

        template<typename T>
        void slfvector<T>::push_back(const T& elem){
            uint32_t bucket = highest_bit(size_ + INITIAL_SIZE) - init_bit;
            if(data_[bucket] == NULL){
                uint32_t bucket_size = INITIAL_SIZE * (1 << bucket);
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
        uint32_t slfvector<T>::size()const{
            return size_;
        }

        template<typename T>
        inline const T& slfvector<T>::at(uint32_t n)const{
            if(n < 0 || n >= size_)
                throw std::out_of_range("out of vector range.");
            return internal_at(n);
        }

        template<typename T>
        inline T& slfvector<T>::at(uint32_t n){
            if(n < 0 || n >= size_)
                throw std::out_of_range("out of vector range.");
            return internal_at(n);
        }

        template<typename T>
        inline const T& slfvector<T>::operator[](const uint32_t n)const{
            if(n < 0 || n >= size_)
                throw std::out_of_range("out of vector range.");
            std::cout<<"abc"<<std::endl;
            return internal_at(n);
        }

        template<typename T>
        inline T& slfvector<T>::operator[](const uint32_t n){
            if(n < 0 || n >= size_)
                throw std::out_of_range("out of vector range.");
            return internal_at(n);
        }

        //resize the vector
        template<typename T>
        void slfvector<T>::resize(uint32_t s){
			if(s < 0)
					return;
            const uint32_t expect_bucket = highest_bit(s + INITIAL_SIZE - 1) - init_bit;
			const uint32_t current_bucket = highest_bit(size_ + INITIAL_SIZE - 1) - init_bit;
            if(current_bucket > expect_bucket){
				size_ = s;
            	for(uint32_t bucket = expect_bucket + 1 ; ++bucket <= current_bucket; )
                	if(data_[bucket] != NULL){
                    	delete[] data_[bucket];
                    	data_[bucket] = NULL;
                	}
			}else{
				for(uint32_t bucket = current_bucket + 1; ++bucket <= expect_bucket; ){
                	uint32_t bucket_size = INITIAL_SIZE * (1 << bucket);
					data_[bucket] = new T[bucket_size];
				}	
				size_ = s;
			}
        }

		template<typename T>
		void slfvector<T>::clear(){
			size_ = 0;
			for(uint32_t i = 0; i < INITIAL_CAPACITY; ++i)
			    if(data_[i] != NULL)
						delete[] data_[i];
			delete[] data_;
		}

		template<typename T>
		inline const T& slfvector<T>::internal_at(uint32_t n)const{
			uint32_t pos = n + INITIAL_SIZE;
			uint32_t hi_bit = highest_bit(pos);
			uint32_t idx = pos ^ (1 << hi_bit);
			return data_[hi_bit - init_bit][idx];
		}

        template<typename T>
        inline T& slfvector<T>::internal_at(uint32_t n){
            uint32_t pos = n + INITIAL_SIZE;
            uint32_t hi_bit = highest_bit(pos);
            uint32_t idx = pos ^ (1 << hi_bit);
            return data_[hi_bit - init_bit][idx];
        }

        template<typename T>
        uint32_t slfvector<T>::highest_bit(uint32_t n)const{
            volatile uint32_t b = 0;
			 #ifdef __x86_64__
			__asm__ __volatile__(
                "bsrl %1, %0"
               	:"=r"(b)
			   	:"r"(n)
			);
			#endif

			#ifndef __x86_64__
            int b_start = 0;
            int b_end = sizeof(uint32_t) * 8;
            while(b_start != b_end){
                int mid = (b_start + b_end) >> 1;
                if((n>>mid) != 0){
                    if(b_start == b_end - 1 )
                        ++b_start;
                    else
                        b_start = mid;
                }
                else
                    b_end = mid;

           }
		   b = b_start;
		   #endif
   		   return b;
	}
=======
namespace izenelib{namespace am{namespace concurrent{

template<typename T>
class slfvector
{
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
    const T& operator[](const size_t n)const;
    T& operator[](const size_t n);
    void reserve(size_t s);

private:
    inline T& internal_at(size_t n);
    inline int highest_bit(size_t n);
private:
    T** data_;
    size_t size_;
};

template<typename T>
slfvector<T>::slfvector():size_(0)
{
    data_ = new T*[INITIAL_CAPACITY];
    data_[0] = new T[INITIAL_SIZE];
    for(size_t i = 1; i < INITIAL_CAPACITY; ++i)
        data_[i] = NULL;
}

template<typename T>
slfvector<T>::~slfvector()
{
    for(size_t i = 0; i < INITIAL_CAPACITY; ++i)
        if(data_[i] != NULL)
            delete[] data_[i];
    delete[] data_;
}

template<typename T>
void slfvector<T>::push_back(const T& elem)
{
    size_t bucket = highest_bit(size_ + INITIAL_SIZE) - init_bit;
    if(data_[bucket] == NULL)
    {
        size_t bucket_size = INITIAL_SIZE * (1 << (bucket + 1));
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
size_t slfvector<T>::size()const
{
    return size_;
}

template<typename T>
inline const T& slfvector<T>::at(size_t n)const
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

template<typename T>
inline T& slfvector<T>::at(size_t n)
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

template<typename T>
inline const T& slfvector<T>::operator[](const size_t n)const
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

template<typename T>
inline T& slfvector<T>::operator[](const size_t n)
{
    if(n < 0 || n >= size_)
        throw std::out_of_range("out of vector range.");
    return internal_at(n);
}

//resize the vector, delete the unnecessory memory
template<typename T>
void slfvector<T>::reserve(size_t s)
{
    size_t expect_bucket = highest_bit(s + INITIAL_SIZE - 1) - init_bit;
    if(expect_bucket < 0)
        expect_bucket = 0;
    size_ = s;
    for( ; ++expect_bucket < INITIAL_CAPACITY; )
        if(data_[expect_bucket] != NULL)
        {
            delete[] data_[expect_bucket];
            data_[expect_bucket] = NULL;
        }
}

template<typename T>
inline T& slfvector<T>::internal_at(size_t n)
{
    size_t pos = n + INITIAL_SIZE;
    size_t hi_bit = highest_bit(pos);
    size_t idx = pos ^ (1 << hi_bit);
    return data_[hi_bit - init_bit][idx];
}

template<typename T>
inline int slfvector<T>::highest_bit(size_t n)
{
//            int b = 0;
//             __asm__ __volatile__(
//                "movl %0, %%eax\n\
//                movl %1, %%ebx\n\
//                bsr %%eax, %%ebx\n\
//                movl %%ebx, %1"
//                :"=g"(n),"=g"(b)
//                :"1"(b)
//            );
//            return b;
    int b_start = 0;
    int b_end = sizeof(size_t) * 8;
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
    return b_start;
}

>>>>>>> c4bbb5f580dda709ef957d64355a4a00a579cc91
}}}

#endif
