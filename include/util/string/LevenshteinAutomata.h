#ifndef IZENELIB_UTIL_STRING_LEVENSHTEIN_AUTOMATA_H
#define IZENELIB_UTIL_STRING_LEVENSHTEIN_AUTOMATA_H

#include "DFA.h"
#include "NFA.h"
#include <util/ustring/UString.h>
#include <string>
#include <stack>

namespace izenelib
{
namespace util
{

using izenelib::util::UString;
using std::string;

template<class StringType = UString>
struct LevenshteinAutomata
{
    mutable DFA<unsigned int> DFA_;
    LevenshteinAutomata(const StringType & pattern, unsigned int distance);
    bool Match(const StringType& testString) const;
    bool NextValidString(const StringType& input, StringType& result) const;
};

template <>
LevenshteinAutomata<UString>::LevenshteinAutomata(const UString & pattern, unsigned int distance)
{
    NFA<unsigned int> nfa(NFAState<unsigned int>(0,0));
    for (unsigned int i = 0; i < pattern.length(); ++i)
    {
        UString cc(pattern, i, 1);
        std::string c;
        cc.convertString(c, UString::UTF_8);
        for (unsigned int e = 0; e < distance + 1; ++e)
        {
            //      # Correct character
            nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e), c);
            if (e < distance)
            {
                //          # Deletion
                nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i,e + 1), "ANY");
                //          # Insertion
                nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e + 1), "EPSILON");
                //          # Substitution
                nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e + 1), "ANY");
            }
        }
    }
    for (unsigned int e = 0; e < distance + 1; ++e)
    {
        if (e < distance)
            nfa.AddTransition(NFAState<unsigned int>(pattern.length(),e), NFAState<unsigned int>(pattern.length(),e + 1), "ANY");
        nfa.AddFinalState(NFAState<unsigned int>(pattern.length(),e));
    }

    DFA_ = nfa.ToDFA();
    DFA_.SetDeadState(DFAState<unsigned int>(NFAState<unsigned int>(pattern.length() + 1,pattern.length() + 1)));
}

template <>
bool LevenshteinAutomata<UString>::Match(const UString& testString) const
{
    DFAState<unsigned int> currentState = DFA_.GetStartState();
    for (unsigned int i = 0; i < testString.length(); ++i)
    {
        UString cc(testString, i, 1);
        std::string c;
        cc.convertString(c, UString::UTF_8);
        currentState = DFA_.GetNextDFAState(currentState, c);
    }
    if (DFA_.IsFinal(currentState))
        return true;
    else
        return false;
}

template <>
bool LevenshteinAutomata<UString>::NextValidString(const UString& input, UString& result) const
{
    std::stack<std::pair<UString, DFAState<unsigned int> > > state_stack;
    DFAState<unsigned int> currentState = DFA_.GetStartState();
    for (unsigned int i = 0; i < input.length(); ++i)
    {
        UString cc(input, i, 1);
        state_stack.push(std::make_pair(cc, currentState));
        std::string c;
        cc.convertString(c, UString::UTF_8);
        currentState = DFA_.GetNextDFAState(currentState, c);
        if(DFA_.IsDeadState(currentState))
        {
            UString cc(input, i+1, i);
            state_stack.push(std::make_pair(cc, currentState));
            break;
        }
    }
    if (DFA_.IsFinal(currentState))
    {
        result = input;
        return true;
    }

    while(!state_stack.empty())
    {
        std::pair<UString, DFAState<unsigned int> > item = state_stack.top();
        state_stack.pop();
        std::string c;
        item.first.convertString(c, UString::UTF_8);
        DFAState<unsigned int> state = item.second;
        if(DFA_.FindNextEdge(state,c,c))
        {
            result += item.first;
            state = DFA_.GetNextDFAState(state, c);
            if(DFA_.IsFinal(state))
            {
                return true;
            }
            state_stack.push(std::make_pair(result, state));
        }
    }
    return false;
}

template <>
LevenshteinAutomata<string>::LevenshteinAutomata(const string & pattern, unsigned int distance)
{
    NFA<unsigned int> nfa(NFAState<unsigned int>(0,0));
    for (unsigned int i = 0; i < pattern.size(); ++i)
    {
        string c(pattern, i, 1);
        for (unsigned int e = 0; e < distance + 1; ++e)
        {
            //      # Correct character
            nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e), c);
            if (e < distance)
            {
                //          # Deletion
                nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i,e + 1), "ANY");
                //          # Insertion
                nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e + 1), "EPSILON");
                //          # Substitution
                nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e + 1), "ANY");
            }
        }
    }
    for (unsigned int e = 0; e < distance + 1; ++e)
    {
        if (e < distance)
            nfa.AddTransition(NFAState<unsigned int>(pattern.size(),e), NFAState<unsigned int>(pattern.size(),e + 1), "ANY");
        nfa.AddFinalState(NFAState<unsigned int>(pattern.size(),e));
    }

    DFA_ = nfa.ToDFA();
    DFA_.SetDeadState(DFAState<unsigned int>(NFAState<unsigned int>(pattern.size() + 1,pattern.size() + 1)));
}

template <>
bool LevenshteinAutomata<string>::Match(const string& testString) const
{
    DFAState<unsigned int> currentState = DFA_.GetStartState();
    for (unsigned int i = 0; i < testString.size(); ++i)
    {
        string c(testString, i, 1);
        currentState = DFA_.GetNextDFAState(currentState, c);
    }
    if (DFA_.IsFinal(currentState))
        return true;
    else
        return false;
}

template <>
bool LevenshteinAutomata<string>::NextValidString(const string& input, string& result) const
{
    std::stack<std::pair<string, DFAState<unsigned int> > > state_stack;
    DFAState<unsigned int> currentState = DFA_.GetStartState();
    for (unsigned int i = 0; i < input.length(); ++i)
    {
        string c(input, i, 1);
        state_stack.push(std::make_pair(c, currentState));
        currentState = DFA_.GetNextDFAState(currentState, c);
        if(DFA_.IsDeadState(currentState))
        {
            string cc(input, i+1, i);
            state_stack.push(std::make_pair(cc, currentState));
            break;
        }
    }
    if (DFA_.IsFinal(currentState))
    {
        result = input;
        return true;
    }

    while(!state_stack.empty())
    {
        std::pair<string, DFAState<unsigned int> > item = state_stack.top();
        state_stack.pop();
        std::string c;
        DFAState<unsigned int> state = item.second;
        if(DFA_.FindNextEdge(state,item.first,c))
        {
            result += item.first;
            state = DFA_.GetNextDFAState(state, c);
            if(DFA_.IsFinal(state))
            {
                return true;
            }
            state_stack.push(std::make_pair(result, state));
        }
    }
    return false;
}

}
}
#endif // LEVENSHTEIN_AUTOMATA_H

