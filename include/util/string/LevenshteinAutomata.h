#ifndef IZENELIB_UTIL_STRING_LEVENSHTEIN_AUTOMATA_H
#define IZENELIB_UTIL_STRING_LEVENSHTEIN_AUTOMATA_H

#include "DFA.h"
#include "NFA.h"
#include <util/ustring/UString.h>

#include <string>
#include <stack>
#include <boost/tuple/tuple.hpp>

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
    result.clear();
    std::stack<boost::tuple<string, DFAState<unsigned int>, char > > state_stack;
    DFAState<unsigned int> currentState = DFA_.GetStartState();
    unsigned int i = 0;
    bool deadState = false;
    for (; i < input.length(); ++i)
    {
        string path(input, 0, i);
        char x = input[i];
        state_stack.push(boost::make_tuple(path, currentState, x));
        currentState = DFA_.GetNextDFAState(currentState, string()+x);
        if(DFA_.IsDeadState(currentState))
        {
            deadState = true;
            break;
        }
    }

    if(!deadState)
    {
        string cc(input, 0, i);
        state_stack.push(boost::make_tuple(cc, currentState, -1));
    }

    if (DFA_.IsFinal(currentState))
    {
        result = input;
        return true;
    }

    while(!state_stack.empty())
    {
        boost::tuple<string, DFAState<unsigned int>, char > item = state_stack.top();
        state_stack.pop();
        std::string path = item.get<0>();
        DFAState<unsigned int> state = item.get<1>();
        char x = item.get<2>();
        x = DFA_.FindNextEdge(state,x);
        if(x != -1)
        {
            path += x;
            state = DFA_.GetNextDFAState(state, string()+x);
            if(DFA_.IsFinal(state))
            {
                result = path;
                return true;
            }
            state_stack.push(boost::make_tuple(path, state, -1));
        }
    }
    return false;
}

}
}
#endif // LEVENSHTEIN_AUTOMATA_H

