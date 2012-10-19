#ifndef IZENELIB_UTIL_STRING_LEVENSHTEIN_AUTOMATA_H
#define IZENELIB_UTIL_STRING_LEVENSHTEIN_AUTOMATA_H

#include "DFA.h"
#include "NFA.h"
#include <string>

namespace izenelib{ namespace util{

using std::string;

struct LevenshteinAutomata
{
    mutable DFA<unsigned int> DFA_;
    LevenshteinAutomata(const string & pattern, unsigned int distance);
    bool Match(const string& testString) const;
};

}}
#endif // LEVENSHTEIN_AUTOMATA_H
