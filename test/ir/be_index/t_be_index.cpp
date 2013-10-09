#include <ir/be_index/DNFInvIndex.hpp>

#include <boost/test/unit_test.hpp>
#include <util/ClockTimer.h>

using namespace izenelib::ir::be;

BOOST_AUTO_TEST_SUITE(t_be_index)

void initDNFs(std::vector<DNF> & DNFs)
{
    Assignment a(1, true, list_of(1));
    Assignment b(2, true, list_of(1));
    Assignment c(3, true, list_of(1));
    Assignment d(3, true, list_of(2));
    Assignment e(2, false, list_of(2));
    Assignment f(2, true, list_of(2));
    Assignment g(1, true, list_of(1)(2));
    Assignment h(2, false, list_of(2)(1));

    Conjunction c1(1, list_of(a)(b));
    Conjunction c2(2, list_of(a)(c));
    Conjunction c3(3, list_of(a)(d)(e));
    Conjunction c4(4, list_of(f)(d));
    Conjunction c5(5, list_of(g));
    Conjunction c6(6, list_of(h));

    DNFs.push_back(DNF(1, list_of(c1)));
    DNFs.push_back(DNF(2, list_of(c2)));
    DNFs.push_back(DNF(3, list_of(c3)(c4)));
    DNFs.push_back(DNF(4, list_of(c4)));
    DNFs.push_back(DNF(5, list_of(c4)(c5)));
    DNFs.push_back(DNF(6, list_of(c6)));
}

BOOST_AUTO_TEST_CASE(do_search_reverse)
{
    std::vector<DNF> DNFs;
    initDNFs(DNFs);

    DNFInvIndex a;
    for (std::size_t i = 0; i != DNFs.size(); ++i)
    {
        a.addDNF(DNFs[i]);
    }

    //ConjunctionInvIndex a;
    //for (std::size_t i = 0; i != conjunctions.size(); ++i)
    //{
    //    a.addConjunction(conjunctions[i]);
    //}

    //std::vector<std::pair<int, int> > assignment = list_of(std::make_pair(1, 1))(std::make_pair(2, 2))(std::make_pair(3, 2));
    //std::vector<int> conjunctionIDs;
    //a.retrieve(assignment, conjunctionIDs);

    //for (std::size_t i = 0; i != conjunctionIDs.size(); ++i)
    //{
    //    std::cout << conjunctionIDs[i] << std::endl;
    //}

    std::vector<std::pair<int, int> > assignment = list_of(std::make_pair(1, 1))(std::make_pair(2, 2))(std::make_pair(3, 1));
    boost::unordered_set<int> dnfIDs;
    a.retrieve(assignment, dnfIDs);

    for (boost::unordered_set<int>::iterator i = dnfIDs.begin(); i != dnfIDs.end(); ++i)
    {
        std::cout << *i << std::endl;
    }

    a.conjIndex.show();
}

BOOST_AUTO_TEST_SUITE_END()

