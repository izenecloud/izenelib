#include <boost/test/unit_test.hpp>

#include <util/string/LevenshteinAutomata.h>

BOOST_AUTO_TEST_SUITE(t_levenshtein_automata)

BOOST_AUTO_TEST_CASE(automata_test)
{
    izenelib::util::LevenshteinAutomata automata("aaaaa", 1);
    BOOST_CHECK(automata.Match("aaaab"));
    BOOST_CHECK(automata.Match("aaaaab"));
    BOOST_CHECK(automata.Match("aaaa"));	
    BOOST_CHECK(!automata.Match("baaab"));
}


BOOST_AUTO_TEST_SUITE_END()

