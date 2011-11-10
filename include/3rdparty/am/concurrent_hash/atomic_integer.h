#ifndef CONCURRENT_ATOMIC_INTGER_H
#define CONCURRENT_ATOMIC_INTGER_H

namespace concurrent{

template <typename T>
class atomic_integer
{
public:
    atomic_integer(const T& v = 0):value_(v) {}
    atomic_integer(const atomic_integer& v):value_(v.get()) {}
    const T& get()const
    {
        return value_;
    }
    atomic_integer& operator=(const T& t)
    {
        value_ = t;
    }
    T faa(const T& v)
    {
        return __sync_fetch_and_add(&value_, v);
        T old;
        T new_one;
        do
        {
            old = value_;
            new_one = old + 1;
        }
        while (!__sync_bool_compare_and_swap(&value_, old, new_one));
        return old;
    }
    void cas(const T& expect, const T& new_value)
    {
        __sync_bool_compare_and_swap(&value_, expect, new_value);
    }
private:
    T value_;
};

}
#endif
