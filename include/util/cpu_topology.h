#ifndef IZENELIB_UTIL_CPUTOPOLOGY_H_
#define IZENELIB_UTIL_CPUTOPOLOGY_H_

#include <types.h>
#include <vector>
#include <iostream>
#include <libNUMA.h>

namespace izenelib{
namespace util{


/* |======= socket0 =======|======= socket1 =======|
 * |== core0 ==|== core1 ==|== core2 ==|== core3 ==|
 * | cpu0,cpu1 | cpu2,cpu3 | cpu4,cpu5 | cpu6,cpu7 |
 *  */
struct CpuTopologyT
{
    bool cpu_topology_supported;
    bool is_hyper_thread;
    std::vector< std::vector< int > > cpu_topology_array;
};

class CpuInfo
{
public:
    static void InitCpuTopologyInfo(CpuTopologyT& topo)
    {
        if(topo.cpu_topology_array.size() > 0)
            return;
        int memnode_n = NUMA_memnode_system_count();
        int cpu_count = NUMA_cpu_system_count();
        topo.cpu_topology_array.resize(memnode_n);
        topo.is_hyper_thread = false;
        topo.cpu_topology_supported = true;

        for(int i = 0; i < memnode_n; ++i)
        {
            std::vector< int >& cpu_array = topo.cpu_topology_array[i];
            memnode_set_t* memsetp;
            size_t memnode_size;
            memsetp = MEMNODE_ALLOC(1);
            memnode_size = MEMNODE_ALLOC_SIZE(1);
            MEMNODE_ZERO_S(memnode_size, memsetp);
            MEMNODE_SET_S(i, memnode_size, memsetp);
            cpu_set_t* cpu_out;
            size_t cpu_size;
            cpu_out = CPU_ALLOC(cpu_count);
            cpu_size = CPU_ALLOC_SIZE(cpu_count);
            CPU_ZERO_S(cpu_size, cpu_out);
            int ret = NUMA_memnode_to_cpu(memnode_size, memsetp, cpu_size, cpu_out);
            if(ret == -1)
            {
                std::cout << "== Warning: init cpu topology failed, will use openmp for multi-thread searching!" << std::endl;
                CPU_FREE(cpu_out);
                MEMNODE_FREE(memsetp);
                topo.cpu_topology_supported = false;
                return;
            }

            cpu_set_t* single_cpu;
            single_cpu = CPU_ALLOC(1);
            size_t single_cpu_size = CPU_ALLOC_SIZE(1);
            cpu_set_t* level_cpus;
            size_t level_cpu_size;
            level_cpus = CPU_ALLOC(cpu_count);
            level_cpu_size = CPU_ALLOC_SIZE(cpu_count);
            for(int j = 0; j < cpu_count; j++)
            {
                if(CPU_ISSET_S(j, cpu_size, cpu_out))
                {
                    bool exist = false;
                    for(int m = 0; m < (int)cpu_array.size(); ++m)
                    {
                        if(cpu_array[m] == j)
                        {
                            exist = true;
                            break;
                        }
                    }
                    if(!exist)
                    {
                        cpu_array.push_back(j);
                    }
                    else
                        continue;

                    // find same hyper-thread cpu
                    // searching level cpus
                    // level 1 : in same core (hyper-thread)
                    // level 2 : in same socket
                    // level 3 : in same machine
                    CPU_ZERO_S(single_cpu_size, single_cpu);
                    CPU_SET_S(j, single_cpu_size, single_cpu);
                    CPU_ZERO_S(level_cpu_size, level_cpus);
                    int n = NUMA_cpu_level_mask(level_cpu_size, level_cpus,
                        single_cpu_size, single_cpu, 1);
                    if(n < 1)
                        continue;
                    for(int k = 0; k < cpu_count; ++k)
                    {
                        if(CPU_ISSET_S(k, level_cpu_size, level_cpus))
                        {
                            bool exist = false;
                            for(int m = 0; m < (int)cpu_array.size(); ++m)
                            {
                                if(cpu_array[m] == k)
                                {
                                    exist = true;
                                    break;
                                }
                            }
                            if(!exist)
                            {
                                cpu_array.push_back(k);
                                topo.is_hyper_thread = true;
                            }
                        }
                    }
                }
            }
            CPU_FREE(single_cpu);
            CPU_FREE(level_cpus);
            CPU_FREE(cpu_out);
            MEMNODE_FREE(memsetp);
            if(cpu_array.size() == 0)
            {
                topo.cpu_topology_supported = false;
                topo.cpu_topology_array.clear();
                return;
            }
        }
        std::cout << "cpu info : ";
        std::cout << "hyper thread support: " << topo.is_hyper_thread << std::endl;
        std::cout << "memnode number: " << topo.cpu_topology_array.size() << std::endl;
        for(size_t i = 0; i < topo.cpu_topology_array.size(); ++i)
        {
            std::cout << "in memnode: " << i << ", cpu number :" << topo.cpu_topology_array[i].size() << std::endl;
            std::cout << "cpu topology is : ";
            for(size_t j = 0; j < topo.cpu_topology_array[i].size(); ++j)
            {
                std::cout << topo.cpu_topology_array[i][j] << ",";
            }
            std::cout << std::endl;
        }
    }
};

}
}

#endif
