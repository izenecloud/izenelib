/**
* @file        LFVector.h
* @version     SF1 v5.0
* @brief Lock free concurrent vector
*/

#ifndef LFVECTOR_H
#define LFVECTOR_H

#include<stddef.h>
namespace izenelib{ namespace am { namespace concurrent {

    template<class T>
    class LFVector{
        public:
            static size_t const INITIAL_CAPACITY = 32;
            static size_t const INITIAL_SIZE = 8;
            LFVector();
            LFVector(size_t);
            LFVector(const LFVector&);
            ~LFVector();
            void push_back(const T&);
            void pop_back();
            bool empty()const;
            size_t size()const;
            const T& at(size_t n)const;
            T& at(size_t n);
            LFVector<T>& operator=(const LFVector<T>& x);

        private:
            class WriteDescriptor{
                public:
                    WriteDescriptor(){
                        completed = true;
                    }
                    WriteDescriptor(const T& oldv, const T& newv, size_t loc)
                        :old_value(oldv), new_value(newv), location(loc), completed(false){
                    }
                    ~WriteDescriptor(){
                    }

                    friend bool operator==(const WriteDescriptor& lhs, const WriteDescriptor& rhs){
                        return (lhs.old_value == rhs.old_value && lhs.new_value == rhs.new_value
                            && lhs.location == rhs.location && lhs.completed == rhs.completed);
                    }

                    friend bool operator!=(const WriteDescriptor& lhs, const WriteDescriptor& rhs){
                        return !(lhs == rhs);
                    }

                    T old_value;
                    T new_value;
                    size_t location;
                    bool completed;
            };
            class Descriptor{
                public:
                    Descriptor(size_t s, WriteDescriptor& wd):size(s), wdesc(wd){
                    }

                    ~Descriptor(){
                    }

                    Descriptor& operator=(const Descriptor& desc){
                        size = desc.size;
                        wdesc = desc.wdesc;
                        return *this;
                    }

                    friend bool operator==(const Descriptor& lhs, const Descriptor& rhs){
                        return (lhs.size == rhs.size && lhs.wdesc == rhs.wdesc);
                    }

                    friend bool operator!=(const Descriptor& lhs, const Descriptor& rhs){
                        return !(lhs == rhs);
                    }

                    size_t size;
                    WriteDescriptor wdesc;
            };

        private:
            void complete_write(WriteDescriptor& wd);
            void alloc_bucket(size_t bucket);
            size_t highest_bit(size_t n);
            T at_nocheck(size_t n);

        private:
            Descriptor* desc;
            T** data;
    };
}}}
#endif
