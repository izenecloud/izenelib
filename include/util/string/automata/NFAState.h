#ifndef IZENELIB_UTIL_STRING_NFASTATE_H
#define IZENELIB_UTIL_STRING_NFASTATE_H

#include "Pair.h"

namespace izenelib
{
namespace util
{

//Declarations:

template <class T>
class NFAState
{
    Pair<T> value;

public:
    NFAState(const T& inpFirstValue, const T& inpSecondValue);
    const Pair<T>& GetValue() const;
};

template <class T>
class NFAStateComparation
{
public:
    bool operator() (const NFAState<T>& lhs, const NFAState<T>& rhs) const;
};

template <class T>
bool operator<(const NFAState<T>& inpLeftNFAState, const NFAState<T>& inpRightNFAState);

template <class T>
bool operator<(const Pair<T>& inpLeftPair, const Pair<T>& inpRightPair);


//Definitions:

template <class T>
NFAState<T>::NFAState(const T& inpFirstValue, const T& inpSecondValue): value(Pair<T>(inpFirstValue, inpSecondValue)) {}

template <class T>
const Pair<T>& NFAState<T>::GetValue() const
{
    return this->value;
}


template <class T>
bool NFAStateComparation<T>::operator()(const NFAState<T>& lhs, const NFAState<T>& rhs) const
{
    return lhs<rhs;
}

template <class T>
bool operator<(const NFAState<T>& inpLeftNFAState, const NFAState<T>& inpRightNFAState)
{
    return inpLeftNFAState.GetValue() < inpRightNFAState.GetValue();
}

template <class T>
bool operator==(const NFAState<T>& inpLeftNFAState, const NFAState<T>& inpRightNFAState)
{
    return inpLeftNFAState.GetValue() == inpRightNFAState.GetValue();
}

template <class T>
bool operator<(const Pair<T>& inpLeftPair, const Pair<T>& inpRightPair)
{
    if (inpLeftPair.GetFirstValue() < inpRightPair.GetFirstValue())
        return true;
    else if (inpLeftPair.GetFirstValue() == inpRightPair.GetFirstValue())
        return inpLeftPair.GetSecondValue() < inpRightPair.GetSecondValue();
    else
        return false;
}

template <class T>
bool operator==(const Pair<T>& inpLeftPair, const Pair<T>& inpRightPair)
{
    return (inpLeftPair.GetFirstValue() == inpRightPair.GetFirstValue()) &&
                (inpLeftPair.GetSecondValue() == inpRightPair.GetSecondValue());
}

}
}

#endif
