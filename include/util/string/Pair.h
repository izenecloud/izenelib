#ifndef IZENELIB_UTIL_STRING_PAIR_H
#define IZENELIB_UTIL_STRING_PAIR_H

namespace izenelib{ namespace util{

template <class T>
class Pair {
    T firstValue;
    T secondValue;

public:
    Pair(const T& inpFirstValue, const T& inpSecondValue);
    const T& GetFirstValue() const;
    const T& GetSecondValue() const;
};

template <class T>
Pair<T>::Pair(const T& inpFirstValue, const T& inpSecondValue): firstValue(inpFirstValue),secondValue(inpSecondValue){}

template <class T>
const T& Pair<T>::GetFirstValue() const{
    return this->firstValue;
}

template <class T>
const T& Pair<T>::GetSecondValue() const{
    return this->secondValue;
}

}}
#endif
