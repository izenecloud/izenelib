#ifndef IZENELIB_UTIL_STRING_NFA_H
#define IZENELIB_UTIL_STRING_NFA_H

#include <map>
#include <vector>
#include <set>
#include <string>
#include <utility>
#include "NFAState.h"
#include "DFAState.h"
#include "DFA.h"

namespace izenelib
{
namespace util
{

using std::vector;
using std::map;
using std::string;
using std::set;

//Declarations:

template <class T>
class NFA
{
    set<NFAState<T>, NFAStateComparation<T> > startStates;
    set<NFAState<T>, NFAStateComparation<T> > finalStates;
    map<NFAState<T>, map<string, set<NFAState<T>, NFAStateComparation<T> > >, NFAStateComparation<T> > transitions;

public:

    const set<NFAState<T>, NFAStateComparation<T> > Expand(set<NFAState<T>, NFAStateComparation<T> >& inpStates);
    const DFAState<T> GetNextDFAState(const DFAState<T>& inpDFAState, const string& inpInput);
    const set<string> GetInputsForDFA(const DFAState<T>& inpDFAState);
    bool IsFinal(const DFAState<T>& inpDFAState) const;

public:
    NFA(const NFAState<T>& inpStartState);
    void AddTransition(const NFAState<T>& inpSource, const NFAState<T>& inpDestination, const string& inpInput);
    void AddFinalState(const NFAState<T>& inpState);
    const set<NFAState<T>, NFAStateComparation<T> > GetStartStates();
    DFA<T> ToDFA();
};

//Definitions:

template <class T>
NFA<T>::NFA(const NFAState<T>& inpStartState)
{
    this->startStates.insert(inpStartState);
}

template <class T>
void NFA<T>::AddTransition(const NFAState<T>& inpSource, const NFAState<T>& inpDestination, const string& inpInput)
{
    typename map<NFAState<T>, map<string, set<NFAState<T>, NFAStateComparation<T> > >, NFAStateComparation<T> >::iterator transitionForSource = this->transitions.find(inpSource);
    if(transitionForSource == this->transitions.end())
    {
        map<string, set<NFAState<T>, NFAStateComparation<T> > > newTransition;
        set<NFAState<T>, NFAStateComparation<T> > dest;
        dest.insert(inpDestination);
        newTransition.insert(std::pair<string, set<NFAState<T>, NFAStateComparation<T> > >(inpInput, dest));
        this->transitions.insert(std::pair<NFAState<T>, map<string, set<NFAState<T>, NFAStateComparation<T> > > >(inpSource, newTransition));
    }
    else if (transitionForSource->second.find(inpInput) == transitionForSource->second.end())
    {
        set<NFAState<T>, NFAStateComparation<T> > dest;
        dest.insert(inpDestination);
        transitionForSource->second.insert(std::pair<string, set<NFAState<T>, NFAStateComparation<T> > >(inpInput, dest));
    }
    else
    {
        set<NFAState<T>, NFAStateComparation<T> > st = transitionForSource->second.find(inpInput)->second;
        transitionForSource->second.find(inpInput)->second.insert(inpDestination);
        st = transitionForSource->second.find(inpInput)->second;
    }
    return;
}

template <class T>
void NFA<T>::AddFinalState(const NFAState<T>& inpState)
{
    this->finalStates.insert(inpState);
}

template <class T>
bool NFA<T>::IsFinal(const DFAState<T>& inpDFAState) const
{
    typename set<NFAState<T>, NFAStateComparation<T> >::iterator iter;
    bool result = false;
    for (iter = inpDFAState.GetValue().begin(); iter != inpDFAState.GetValue().end(); ++iter)
        if (this->finalStates.find(*iter) != this->finalStates.end())
        {
            result = true;
            break;
        }
    return result;
}

template <class T>
const set<NFAState<T>, NFAStateComparation<T> > NFA<T>::Expand(set<NFAState<T>, NFAStateComparation<T> >& inpStates)
{
    set<NFAState<T>, NFAStateComparation<T> > frontier(inpStates);
    typename set<NFAState<T>, NFAStateComparation<T> >::iterator frontierIter;
    while (! frontier.empty())
    {
        frontierIter = frontier.begin();
        NFAState<T> state = *frontierIter;
        frontier.erase(frontierIter);
        set<NFAState<T>, NFAStateComparation<T> > newStates;
        typename map<NFAState<T>, map<string, set<NFAState<T>, NFAStateComparation<T> > >, NFAStateComparation<T> >::iterator transitionForState = this->transitions.find(state);
        if(transitionForState == this->transitions.end())
            continue;
        else
        {
            typename map<string, set<NFAState<T>, NFAStateComparation<T> > >::iterator epsilonTransition = transitionForState->second.find("EPSILON");
            if (epsilonTransition == transitionForState->second.end())
                continue;
            else
            {
                set<NFAState<T>, NFAStateComparation<T> > epsilonStates = epsilonTransition->second;
                typename set<NFAState<T>, NFAStateComparation<T> >::iterator inpStatesIter;
                for (inpStatesIter = inpStates.begin(); inpStatesIter != inpStates.end(); ++inpStatesIter)
                    epsilonStates.erase(*inpStatesIter);
                newStates.insert(epsilonStates.begin(), epsilonStates.end());
            }
        }
        frontier.insert(newStates.begin(), newStates.end());
        inpStates.insert(newStates.begin(), newStates.end());
    }
    return inpStates;
}

template <class T>
const set<NFAState<T>, NFAStateComparation<T> > NFA<T>::GetStartStates()
{
    return this->Expand(this->startStates);
}

template <class T>
const DFAState<T> NFA<T>::GetNextDFAState(const DFAState<T>& inpDFAState, const string& inpInput)
{
    set<NFAState<T>, NFAStateComparation<T> > destStates;
    set<NFAState<T>, NFAStateComparation<T> > states = inpDFAState.GetValue();
    typename set<NFAState<T>, NFAStateComparation<T> >::iterator statesIter;
    for (statesIter = states.begin(); statesIter != states.end(); ++statesIter)
    {
        map<string, set<NFAState<T>, NFAStateComparation<T> > > stateTransitions;
        typename map<NFAState<T>, map<string, set<NFAState<T>, NFAStateComparation<T> > >, NFAStateComparation<T> >::iterator stateTransitionsIter = this->transitions.find(*statesIter);
        if(stateTransitionsIter == this->transitions.end())
            continue;
        stateTransitions = stateTransitionsIter->second;
        set<NFAState<T>, NFAStateComparation<T> > inputStates;
        typename map<string, set<NFAState<T>, NFAStateComparation<T> > >::iterator inputStatesIter = stateTransitions.find(inpInput);
        if (inputStatesIter != stateTransitions.end())
        {
            inputStates = inputStatesIter->second;
            destStates.insert(inputStates.begin(),inputStates.end());
        }
        inputStatesIter = stateTransitions.find("ANY");
        if (inputStatesIter != stateTransitions.end())
        {
            inputStates = inputStatesIter->second;
            destStates.insert(inputStates.begin(),inputStates.end());
        }
    }
    return DFAState<T>(this->Expand(destStates));
}

template <class T>
const set<string> NFA<T>::GetInputsForDFA(const DFAState<T>& inpDFAState)
{
    set<string> inputs;
    set<NFAState<T>, NFAStateComparation<T> > states = inpDFAState.GetValue();
    typename set<NFAState<T>, NFAStateComparation<T> >::iterator statesIter;
    for (statesIter = states.begin(); statesIter != states.end(); ++statesIter)
    {
        map<string, set<NFAState<T>, NFAStateComparation<T> > > stateTransitions;
        typename map<NFAState<T>, map<string, set<NFAState<T>, NFAStateComparation<T> > >, NFAStateComparation<T> >::iterator stateTransitionsIter = this->transitions.find(*statesIter);
        if(stateTransitionsIter == this->transitions.end())
            continue;
        stateTransitions = stateTransitionsIter->second;
        typename map<string, set<NFAState<T>, NFAStateComparation<T> > >::iterator inputStatesIter;
        for (inputStatesIter = stateTransitions.begin(); inputStatesIter != stateTransitions.end(); ++inputStatesIter)
            inputs.insert(inputStatesIter->first);
    }
    return inputs;
}

template <class T>
DFA<T> NFA<T>::ToDFA()
{
    DFA<T> dfa(DFAState<T>(this->GetStartStates()));
    vector<DFAState<T> > frontier;
    frontier.push_back(DFAState<T>(this->GetStartStates()));
    set<DFAState<T>, DFAStateComparation<T> > seen;
    while (! frontier.empty())
    {
        DFAState<T> current = frontier.back();
        frontier.pop_back();
        set<string> inputs = this->GetInputsForDFA(current);
        set<string>::iterator inputsIter;
        for (inputsIter = inputs.begin(); inputsIter != inputs.end(); ++inputsIter)
        {
            if (*inputsIter == "EPSILON")
                continue;
            DFAState<T> newState = this->GetNextDFAState(current, *inputsIter);
            if (seen.find(newState) == seen.end())
            {
                frontier.push_back(newState);
                seen.insert(newState);
                if (this->IsFinal(newState))
                    dfa.AddFinalState(newState);
            }
            if (*inputsIter == "ANY")
                dfa.AddDefaultTransition(current, newState);
            else
                dfa.AddTransition(current, newState, *inputsIter);
        }
    }
    return dfa;
}

}
}
#endif

