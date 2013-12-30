#include <ir/be_index/InvIndex.hpp>
#include <ir/be_index/DNF.hpp>
// #include <ir/be_index/IDMapper.hpp>
// #include <ir/be_index/AVMapper.hpp>

#include <boost/test/unit_test.hpp>
#include <util/ClockTimer.h>

#include <boost/assign/list_of.hpp>

// #include <fstream>

using namespace izenelib::ir::be_index;
using namespace boost::assign;

BOOST_AUTO_TEST_SUITE(t_be_index)

BOOST_AUTO_TEST_CASE(do_search_reverse)
{
    // Conjunction c1{{Assignment{"age", true, {"3"}}, Assignment{"state", true, {"NY"}}}};
    // Conjunction c2{{Assignment{"age", true, {"3"}}, Assignment{"gender", true, {"F"}}}};
    // Conjunction c3{{Assignment{"age", true, {"3"}}, Assignment{"gender", true, {"M"}}, Assignment{"state", false, {"CA"}}}};
    // Conjunction c4{{Assignment{"state", true, {"CA"}}, Assignment{"gender", true, {"M"}}}};
    // Conjunction c5{{Assignment{"age", true, {"3", "4"}}}};
    // Conjunction c6{{Assignment{"state", false, {"CA", "NY"}}}};

    // DNF d1{{c1}};
    // DNF d2{{c2}};
    // DNF d3{{c3, c4}};
    // DNF d4{{c4}};
    // DNF d5{{c4, c5}};
    // DNF d6{{c6}};

    Assignment a1("age", true, list_of("3"));
    Assignment a2("state", true, list_of("NY"));
    Assignment a3("gender", true, list_of("F"));
    Assignment a4("gender", true, list_of("M"));
    Assignment a5("state", false, list_of("CA"));
    Assignment a6("state", true, list_of("CA"));
    Assignment a7("age", true, list_of("3")("4"));
    Assignment a8("state", false, list_of("CA")("NY"));

    Conjunction c1(list_of(a1)(a2).convert_to_container<std::vector<Assignment> >());
    Conjunction c2(list_of(a1)(a3).convert_to_container<std::vector<Assignment> >());
    Conjunction c3(list_of(a1)(a4)(a5).convert_to_container<std::vector<Assignment> >());
    Conjunction c4(list_of(a6)(a4).convert_to_container<std::vector<Assignment> >());
    Conjunction c5(list_of(a7).convert_to_container<std::vector<Assignment> >());
    Conjunction c6(list_of(a8).convert_to_container<std::vector<Assignment> >());

    DNF d1(list_of(c1).convert_to_container<std::vector<Conjunction> >());
    DNF d2(list_of(c2).convert_to_container<std::vector<Conjunction> >());
    DNF d3(list_of(c3)(c4).convert_to_container<std::vector<Conjunction> >());
    DNF d4(list_of(c4).convert_to_container<std::vector<Conjunction> >());
    DNF d5(list_of(c4)(c5).convert_to_container<std::vector<Conjunction> >());
    DNF d6(list_of(c6).convert_to_container<std::vector<Conjunction> >());

    DNFInvIndex a;

    a.addDNF(1, d1);
    a.addDNF(2, d2);
    a.addDNF(3, d3);
    a.addDNF(4, d4);
    a.addDNF(10, d5);
    a.addDNF(6, d6);

    a.show();

    std::vector<std::pair<std::string, std::string> > assignment = list_of(std::make_pair("age", "3"))(std::make_pair("state", "CA"))(std::make_pair("gender", "M"));
    boost::unordered_set<uint32_t> dnfIDs;

    a.retrieve(assignment, dnfIDs);

    std::cout << std::endl;

    for (boost::unordered_set<uint32_t>::iterator i = dnfIDs.begin(); i != dnfIDs.end(); ++i) {
        std::cout << *i << std::endl;
    }

    std::cout << std::endl;

    std::cout << a.totalNumDNF() << std::endl;

    DNFInvIndex b(a);

    std::cout << b.totalNumDNF() << std::endl;

    // std::ofstream ofs("out");
    // a.save_binary(ofs);

    // DNFInvIndex c;
    // std::ifstream ifs("out");
    // c.load_binary(ifs);
    // std::ofstream ofs("out2");
    // c.save_binary(ofs);
}

BOOST_AUTO_TEST_SUITE_END()
