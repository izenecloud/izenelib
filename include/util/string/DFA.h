#ifndef IZENELIB_UTIL_STRING_DFA_H
#define IZENELIB_UTIL_STRING_DFA_H

#include <map>
#include <vector>
#include <set>
#include <utility>
#include <types.h>
#include "NFAState.h"
#include "DFAState.h"

namespace izenelib
{
namespace util
{

using std::vector;
using std::map;
using std::set;

//Declarations:

template <class T, class StringType>
class DFA
{
    DFAState<T> startState;
    DFAState<T> deadEnd;
    set<DFAState<T>, DFAStateComparation<T> > finalStates;
    map<DFAState<T>, map<StringType, DFAState<T> >, DFAStateComparation<T> > transitions;
    map<DFAState<T>, DFAState<T>, DFAStateComparation<T> > defaults;
public:
    static StringType DELIMITER;
    static StringType ZERO;
	
    DFA(const DFAState<T>& inpStartState);
    DFA();
    void AddTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination, const StringType& inpInput);
    void AddDefaultTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination);
    void AddFinalState(const DFAState<T>& inpState);
    bool IsFinal(const DFAState<T>& inpDFAState) const;
    bool IsDeadState(const DFAState<T>& inpDeadState) const;
    void SetDeadState(const DFAState<T>& inpDeadState);
    StringType FindNextEdge(const DFAState<unsigned int>& inpDFAState, StringType& input);
    const DFAState<T> GetNextDFAState(const DFAState<T>& inpDFAState, const StringType& inpInput);
    const DFAState<T> GetStartState();
    const DFAState<T> GetDeadState();
};

//Definitions:

template <class T, class StringType>
StringType DFA<T, StringType>::DELIMITER = StringType(1,(char)0xFF);
template <class T, class StringType>
StringType DFA<T, StringType>::ZERO = StringType(1,'\0');


template <class T, class StringType>
DFA<T, StringType>::DFA(const DFAState<T>& inpStartState)
{
    this->startState = inpStartState;
}

template <class T, class StringType>
DFA<T, StringType>::DFA() {}

template <class T, class StringType>
void DFA<T, StringType>::AddFinalState(const DFAState<T>& inpStartState)
{
    this->finalStates.insert(inpStartState);
}

template <class T, class StringType>
void DFA<T, StringType>::AddTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination, const StringType& inpInput)
{
    typename map<DFAState<T>, map<StringType, DFAState<T> >, DFAStateComparation<T> >::iterator transitionForSource = this->transitions.find(inpSource);
    if (transitionForSource == this->transitions.end())
    {
        map<StringType, DFAState<T> > newTransition;
        newTransition.insert(std::pair<StringType, DFAState<T> >(inpInput, inpDestination));
        this->transitions.insert(std::pair<DFAState<T>,  map<StringType, DFAState<T> > >(inpSource, newTransition));
    }
    else
        transitionForSource->second.insert(std::pair<StringType, DFAState<T> >(inpInput, inpDestination));
}

template <class T, class StringType>
void DFA<T, StringType>::AddDefaultTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination)
{
    this->defaults.insert(std::pair<DFAState<T>, DFAState<T> >(inpSource, inpDestination));
}

template <class T, class StringType>
bool DFA<T, StringType>::IsFinal(const DFAState<T>& inpDFAState) const
{
    if (this->finalStates.find(inpDFAState) == this->finalStates.end())
        return false;
    else
        return true;
}

template <class T, class StringType>
void DFA<T, StringType>::SetDeadState(const DFAState<T>& inpDeadState)
{
    this->deadEnd = inpDeadState;
    this->AddDefaultTransition(this->deadEnd, this->deadEnd);
}

template <class T, class StringType>
bool DFA<T, StringType>::IsDeadState(const DFAState<T>& inpDeadState) const
{
    return this->deadEnd == inpDeadState;
}

template <class T, class StringType>
StringType DFA<T, StringType>::FindNextEdge(const DFAState<unsigned int>& inpDFAState, StringType& input)
{
    if(input == DELIMITER) input = ZERO;
    if(input != ZERO) 
    {
        uint16_t code = input[input.length() - 1];
        ++code;
        input[input.length() - 1] = code;
    }

    typename map<DFAState<T>, DFAState<T> , DFAStateComparation<T> >::iterator defaultsIter = this->defaults.find(inpDFAState);
    if (defaultsIter != this->defaults.end())
    {
        return input;
    }

    typename map<DFAState<T>, map<StringType, DFAState<T> >, DFAStateComparation<T> >::iterator transitionInpState = this->transitions.find(inpDFAState);
    if (transitionInpState != this->transitions.end())
    {
        map<StringType, DFAState<T> >& transition = transitionInpState->second;
        typename map<StringType, DFAState<T> >::iterator transIter = transition.find(input);
        if(transIter == transition.end())
        {
            transIter = transition.upper_bound(input);
            if(transIter == transition.end()) 
            {
                return DELIMITER;
            }
            return (transIter->first);
        }
        else
        {
            return input;
        }
    }
    else
        return DELIMITER;
}

template <class T, class StringType>
const DFAState<T> DFA<T, StringType>::GetNextDFAState(const DFAState<T>& inpDFAState, const StringType& inpInput)
{
    typename map<DFAState<T>, map<StringType, DFAState<T> >, DFAStateComparation<T> >::iterator transitionInpState = this->transitions.find(inpDFAState);
    if (transitionInpState == this->transitions.end())
    {
        typename map<DFAState<T>, DFAState<T> , DFAStateComparation<T> >::iterator defaultsIter = this->defaults.find(inpDFAState);
        if (defaultsIter == this->defaults.end())
            return this->deadEnd;
        else
            return defaultsIter->second;
    }
    else
    {
        map<StringType, DFAState<T> >& transition = transitionInpState->second;
        typename map<StringType, DFAState<T> >::iterator transIter = transition.find(inpInput);
        if (transIter == transition.end())
        {
            typename map<DFAState<T>, DFAState<T> , DFAStateComparation<T> >::iterator defaultsIter = this->defaults.find(inpDFAState);
            if (defaultsIter == this->defaults.end())
                return this->deadEnd;
            else
                return defaultsIter->second;
        }
        else
            return transIter->second;
    }
}

template <class T, class StringType>
const DFAState<T> DFA<T, StringType>::GetStartState()
{
    return this->startState;
}

template <class T, class StringType>
const DFAState<T> DFA<T, StringType>::GetDeadState()
{
    return this->deadEnd;
}

}
}
#endif
