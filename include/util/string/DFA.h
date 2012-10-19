#ifndef IZENELIB_UTIL_STRING_DFA_H
#define IZENELIB_UTIL_STRING_DFA_H

#include <map>
#include <vector>
#include <set>
#include <string>
#include <utility>
#include "NFAState.h"
#include "DFAState.h"

namespace izenelib{ namespace util{

using std::vector;
using std::map;
using std::string;
using std::set;

//Declarations:

template <class T>
class DFA {
    DFAState<T> startState;
    DFAState<T> deadEnd;
    set<DFAState<T>, DFAStateComparation<T> > finalStates;
    map<DFAState<T>, map<string, DFAState<T> >, DFAStateComparation<T> > transitions;
    map<DFAState<T>, DFAState<T>, DFAStateComparation<T> > defaults;
public:
    DFA(const DFAState<T>& inpStartState);
    DFA();
    void AddTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination, const string& inpInput);
    void AddDefaultTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination);
    void AddFinalState(const DFAState<T>& inpState);
    bool IsFinal(const DFAState<T>& inpDFAState) const;
    void setDeadState(const DFAState<T>& inpDeadState);
    const DFAState<T> GetNextDFAState(const DFAState<T>& inpDFAState, const string& inpInput);
    const DFAState<T> GetStartState();
};

//Definitions:

template <class T>
DFA<T>::DFA(const DFAState<T>& inpStartState){
    this->startState = inpStartState;
}

template <class T>
DFA<T>::DFA(){}

template <class T>
void DFA<T>::AddFinalState(const DFAState<T>& inpStartState){
    this->finalStates.insert(inpStartState);
}

template <class T>
void DFA<T>::AddTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination, const string& inpInput){
    typename map<DFAState<T>, map<string, DFAState<T> >, DFAStateComparation<T> >::iterator transitionForSource = this->transitions.find(inpSource);
    if (transitionForSource == this->transitions.end()){
        map<string, DFAState<T> > newTransition;
        newTransition.insert(std::pair<string, DFAState<T> >(inpInput, inpDestination));
        this->transitions.insert(std::pair<DFAState<T>,  map<string, DFAState<T> > >(inpSource, newTransition));
    } else
        transitionForSource->second.insert(std::pair<string, DFAState<T> >(inpInput, inpDestination));
}

template <class T>
void DFA<T>::AddDefaultTransition(const DFAState<T>& inpSource, const DFAState<T>& inpDestination){
    this->defaults.insert(std::pair<DFAState<T>, DFAState<T> >(inpSource, inpDestination));
}

template <class T>
bool DFA<T>::IsFinal(const DFAState<T>& inpDFAState) const{
    if (this->finalStates.find(inpDFAState) == this->finalStates.end())
        return false;
    else
        return true;
}

template <class T>
void DFA<T>::setDeadState(const DFAState<T>& inpDeadState){
    this->deadEnd = inpDeadState;
    this->AddDefaultTransition(this->deadEnd, this->deadEnd);
}

template <class T>
const DFAState<T>  DFA<T>::GetNextDFAState(const DFAState<T>& inpDFAState, const string& inpInput){
    //state_transitions = self.transitions.get(src, {})
    typename map<DFAState<T>, map<string, DFAState<T> >, DFAStateComparation<T> >::iterator transitionInpState = this->transitions.find(inpDFAState);
    if (transitionInpState == this->transitions.end()){
        typename map<DFAState<T>, DFAState<T> , DFAStateComparation<T> >::iterator defaultsIter = this->defaults.find(inpDFAState);
        if (defaultsIter == this->defaults.end())
            return this->deadEnd;
        else
            return defaultsIter->second;
    } else{
        map<string, DFAState<T> > transition = transitionInpState->second;
        typename map<string, DFAState<T> >::iterator transIter = transition.find(inpInput);
        if (transIter == transition.end()){
            typename map<DFAState<T>, DFAState<T> , DFAStateComparation<T> >::iterator defaultsIter = this->defaults.find(inpDFAState);
            if (defaultsIter == this->defaults.end())
                return this->deadEnd;
            else
                return defaultsIter->second;
        } else
            return transIter->second;
    }
    //return state_transitions.get(input, self.defaults.get(src, None))
}

template <class T>
const DFAState<T> DFA<T>::GetStartState(){
    return this->startState;
}

}}
#endif
