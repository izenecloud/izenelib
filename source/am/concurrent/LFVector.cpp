#include"LFVector.h"
#include<cmath>
#include<stdexcept>
#include<boost/atomic.hpp>
#include<boost/lockfree/detail/cas.hpp>
#include<boost/thread/thread.hpp>
#include<iostream>
using namespace std;
using namespace boost;
using namespace boost::lockfree;
namespace izenelib{ namespace am { namespace concurrent {

    template<class T>
    LFVector<T>::LFVector() {
        WriteDescriptor wd;
        desc = new Descriptor(0, wd);
        data = new T*[INITIAL_CAPACITY];
        for(size_t i = 0; i < INITIAL_CAPACITY; ++i)
            data[i] = NULL;
        data[0] = new T[INITIAL_SIZE];
    }

    template<class T>
    LFVector<T>::LFVector(size_t s){

    }

    template<class T>
    LFVector<T>::LFVector(const LFVector& l){

    }

    template<class T>
    LFVector<T>::~LFVector(){
        delete desc;
        for(size_t i = 0; i < INITIAL_CAPACITY; ++i)
            if(data[i] != NULL)
                delete[] data[i];
        delete[] data;
    }

    template<class T>
    void LFVector<T>::push_back(const T& elem){
        size_t bucket = 0;
        size_t ini_bit = highest_bit(INITIAL_SIZE);
        WriteDescriptor writedesc;
        Descriptor* current;
        bool flag;
        do{
            current = desc;
            complete_write(current -> wdesc);
            bucket = highest_bit(current -> size + INITIAL_SIZE) - ini_bit;
            if(data[bucket] == NULL)
                alloc_bucket(bucket);

            WriteDescriptor writeop(at_nocheck(current -> size), elem, current -> size);
            writedesc = writeop;
            Descriptor next(current -> size + 1, writedesc);
            flag = atomic_cas_emulation(desc, *current, next);
        }while(!flag);
        complete_write(desc -> wdesc);
    }

    template<class T>
    void LFVector<T>::pop_back(){
        int flag = 0;
        do{
            Descriptor* current = desc;
            complete_write(current -> wdesc);
            if(desc -> size >0){
                WriteDescriptor wd;
                Descriptor next(current -> size - 1, wd);
                flag = atomic_cas_emulation(desc, *current, next);
            }else{
                flag = 1;
            }
        }while(!flag);
    }

    template<class T>
    bool LFVector<T>::empty() const{
        return (desc -> size == 0);
    }

    template<class T>
    size_t LFVector<T>::size() const{
        return desc -> size;
    }

    template<class T>
    T& LFVector<T>::at(size_t n){
        if(n < 0 || n >= desc->size)
            throw std::out_of_range("out of vector range.");
        size_t init_bit = highest_bit(INITIAL_SIZE);
        size_t pos = n + INITIAL_SIZE;
        size_t hi_bit = highest_bit(pos);
        size_t idx = pos ^ (size_t)pow(2, hi_bit);
        return data[hi_bit - init_bit][idx];
    }

    template<class T>
    const T& LFVector<T>::at(size_t n)const{
        return at(n);
    }

    template<class T>
    LFVector<T>& LFVector<T>::operator=(const LFVector<T>& x){

    }


    //Follows are the private methods
    template<class T>
    void LFVector<T>::complete_write(WriteDescriptor& wd){
        if(!(wd.completed)){
            size_t init_bit = highest_bit(INITIAL_SIZE);
            size_t pos = wd.location + INITIAL_SIZE;
            size_t hi_bit = highest_bit(pos);
            size_t idx = pos ^ (size_t)pow(2, hi_bit);
            T old_v = wd.old_value;
            T new_v = wd.new_value;
            atomic_cas_emulation(&data[hi_bit - init_bit][idx], old_v, new_v);
            wd.completed = 1;
        }
    }

    template<class T>
    void LFVector<T>::alloc_bucket(size_t bucket){
        size_t bucket_size = INITIAL_SIZE * pow(2, bucket + 1);
        T* mem= new T[bucket_size];
        int flag = atomic_cas_emulation<T*>(&data[bucket], NULL, mem);

        if(!flag)
            delete[] mem;
    }

    template<class T>
    size_t LFVector<T>::highest_bit(size_t n){
        int b = 0;
         __asm__ __volatile__(
            "movl %0, %%eax\n\
            movl %1, %%ebx\n\
            bsr %%eax, %%ebx\n\
            movl %%eax, %0\n\
            movl %%ebx, %1"
            :
            :"g"(n),"g"(b)
        );
        return b;
    }

    template<class T>
    T LFVector<T>::at_nocheck(size_t n){
        size_t init_bit = highest_bit(INITIAL_SIZE);
        size_t pos = n + INITIAL_SIZE;
        size_t hi_bit = highest_bit(pos);
        size_t idx = pos ^ (size_t)pow(2, hi_bit);

        return data[hi_bit - init_bit][idx];
    }
}}}
