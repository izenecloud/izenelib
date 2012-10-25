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
    mutable DFA<unsigned int, StringType> DFA_;
    LevenshteinAutomata(const StringType & pattern, unsigned int distance)
    {
        typedef NFA<unsigned int, StringType> NFAType;
        NFAType nfa(NFAState<unsigned int>(0,0));
        for (unsigned int i = 0; i < pattern.length(); ++i)
        {
            StringType c(pattern, i, 1);
            for (unsigned int e = 0; e < distance + 1; ++e)
            {
                //Correct character
                nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e), c);
                if (e < distance)
                {
                    //Deletion
                    nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i,e + 1), NFAType::ANY);
                    //Insertion
                    nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e + 1), NFAType::EPSILON);
                    //Substitution
                    nfa.AddTransition(NFAState<unsigned int>(i,e), NFAState<unsigned int>(i + 1,e + 1), NFAType::ANY);
                }
            }
        }
        for (unsigned int e = 0; e < distance + 1; ++e)
        {
            if (e < distance)
                nfa.AddTransition(NFAState<unsigned int>(pattern.length(),e), NFAState<unsigned int>(pattern.length(),e + 1), NFAType::ANY);
            nfa.AddFinalState(NFAState<unsigned int>(pattern.length(),e));
        }

        DFA_ = nfa.ToDFA();
        DFA_.SetDeadState(DFAState<unsigned int>(NFAState<unsigned int>(pattern.length() + 1,pattern.length() + 1)));
    }

    bool Match(const StringType& testString) const
    {
        DFAState<unsigned int> currentState = DFA_.GetStartState();
        for (unsigned int i = 0; i < testString.length(); ++i)
        {
            StringType c(testString, i, 1);
            currentState = DFA_.GetNextDFAState(currentState, c);
        }
        if (DFA_.IsFinal(currentState))
            return true;
        else
            return false;
    }

    bool NextValidString(const StringType& input, StringType& result) const
    {
        result.clear();
        std::stack<boost::tuple<StringType, DFAState<unsigned int>, StringType > > state_stack;
        DFAState<unsigned int> currentState = DFA_.GetStartState();
        unsigned int i = 0;
        bool deadState = false;
        for (; i < input.length(); ++i)
        {
            StringType path(input, 0, i);
            StringType x(input, i, 1);
            state_stack.push(boost::make_tuple(path, currentState, x));
            currentState = DFA_.GetNextDFAState(currentState, x);
            if(DFA_.IsDeadState(currentState))
            {
                deadState = true;
                break;
            }
        }

        if(!deadState)
        {
            StringType cc(input, 0, i);
            state_stack.push(boost::make_tuple(cc, currentState, DFA_.DELIMITER));
        }

        if (DFA_.IsFinal(currentState))
        {
            result = input;
            return true;
        }

        while(!state_stack.empty())
        {
            boost::tuple<StringType, DFAState<unsigned int>, StringType > item = state_stack.top();
            state_stack.pop();
            StringType& path = boost::get<0>(item);
            DFAState<unsigned int>& state = boost::get<1>(item);
            StringType& x = boost::get<2>(item);
            x = DFA_.FindNextEdge(state,x);
            if(x != DFA_.DELIMITER)
            {
                path += x;
                DFAState<unsigned int> currentstate = DFA_.GetNextDFAState(state, x);
                if(DFA_.IsFinal(currentstate))
                {
                    result = path;
                    return true;
                }
                state_stack.push(boost::make_tuple(path, currentstate, DFA_.DELIMITER));
            }
        }
        return false;
    }

};
}
}
#endif // LEVENSHTEIN_AUTOMATA_H

