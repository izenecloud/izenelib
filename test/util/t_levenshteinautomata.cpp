#include <boost/test/unit_test.hpp>

#include <util/string/automata/LevenshteinAutomata.h>
#include <util/ClockTimer.h>

BOOST_AUTO_TEST_SUITE(t_levenshtein_automata)

BOOST_AUTO_TEST_CASE(automata_test)
{
    {
    using izenelib::util::UString;
    izenelib::util::LevenshteinAutomata<UString> automata(UString("aaaaa",UString::UTF_8), 1);
    BOOST_CHECK(automata.Match(UString("aaaaa",UString::UTF_8)));
    BOOST_CHECK(automata.Match(UString("aaaaab",UString::UTF_8)));
    BOOST_CHECK(automata.Match(UString("aaaa",UString::UTF_8)));	
    BOOST_CHECK(!automata.Match(UString("baaab",UString::UTF_8)));
    }
    {
    izenelib::util::LevenshteinAutomata<std::string> automata("aaaaa", 1);
    BOOST_CHECK(automata.Match("aaaaa"));
    BOOST_CHECK(automata.Match("aaaaab"));
    BOOST_CHECK(automata.Match("aaaa"));	
    BOOST_CHECK(!automata.Match("baaab"));
    }
}

BOOST_AUTO_TEST_CASE(dict_test)
{
    using izenelib::util::UString;
    {
    izenelib::util::ClockTimer timer;
    izenelib::util::LevenshteinAutomata<UString> automata (UString("asdfkllkklasdfasdfasdffood", UString::UTF_8), 2);
    std::cout<<"construction time:"<<timer.elapsed()<<std::endl;
    }

    {
    izenelib::util::LevenshteinAutomata<std::string> automata("foosddd", 5);
    std::set<std::string> dict;
    dict.insert("fod");
    dict.insert("food");
    dict.insert("bood");
    dict.insert("feod");
    dict.insert("flood");

    std::string match;
    bool ret = automata.NextValidString(string()+'\0',match);
    std::cout<<"begin match "<<match<<" ret "<<ret<<std::endl;

    while(ret)
    {
        std::set<std::string>::iterator dicIt = dict.find(match);
        if(dicIt == dict.end())
        {
            dicIt = dict.upper_bound(match);
            if(dicIt == dict.end())
                break;
        }
        std::string next = *dicIt;
        if(next == match)
        {
            std::cout<<"result: "<<next<<std::endl;
            next += '\0';
        }
        ret = automata.NextValidString(next,match);
    }
    }

    {
    izenelib::util::LevenshteinAutomata<UString> automata(UString("foosddd",UString::UTF_8), 5);
    std::set<UString> dict;
    dict.insert(UString("fod",UString::UTF_8));
    dict.insert(UString("food",UString::UTF_8));
    dict.insert(UString("bood",UString::UTF_8));
    dict.insert(UString("feod",UString::UTF_8));
    dict.insert(UString("flood",UString::UTF_8));
    dict.insert(UString("gooasddd",UString::UTF_8));


    UString match;
    UString start(1,'\0');
    bool ret = automata.NextValidString(start,match);
    std::cout<<"begin match "<<match<<" ret "<<ret<<std::endl;

    while(ret)
    {
        std::set<UString>::iterator dicIt = dict.find(match);
        if(dicIt == dict.end())
        {
            dicIt = dict.upper_bound(match);
            if(dicIt == dict.end())
                break;
        }
        UString next = *dicIt;
        if(next == match)
        {
            std::cout<<"result: "<<next<<std::endl;
            next += start;
        }
        ret = automata.NextValidString(next,match);
    }
    }

}


BOOST_AUTO_TEST_SUITE_END()

