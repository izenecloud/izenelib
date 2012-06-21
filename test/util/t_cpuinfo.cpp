#include <boost/test/unit_test.hpp>

#include <util/cpu_topology.h>


using namespace izenelib::util;


BOOST_AUTO_TEST_SUITE(t_cpuinfo_util)

BOOST_AUTO_TEST_CASE(normal_test)
{
    CpuTopologyT cpu_topo;
    CpuInfo::InitCpuTopologyInfo(cpu_topo);
    BOOST_CHECK(cpu_topo.cpu_topology_supported == true);
    BOOST_CHECK(cpu_topo.cpu_topology_array.size() > 0);
    for(size_t i = 0; i < cpu_topo.cpu_topology_array.size(); ++i)
    {
        BOOST_CHECK(cpu_topo.cpu_topology_array[i].size() > 0);
    }
}


BOOST_AUTO_TEST_SUITE_END()
