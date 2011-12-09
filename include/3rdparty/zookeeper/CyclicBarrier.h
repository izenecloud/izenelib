/**
 * @file CyclicBarrier.h
 * @author Zhongxia Li
 * @date Sep 13, 2011
 * @brief 
 */
#ifndef CYCLIC_BARRIER_H_
#define CYCLIC_BARRIER_H_

#include "ZooKeeper.hpp"
#include "ZooKeeperWatcher.hpp"
#include "ZooKeeperEvent.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace izenelib {
namespace zookeeper {

class CyclicBarrier : public ZooKeeperEventHandler
{
public:
    /**
     * @param parties Required participants for the barrier.
     */
    CyclicBarrier(int parties, const std::string& host, const std::string& root, const std::string& name);

    ~CyclicBarrier();

    int await();

    int await(time_t timeout);

    int getParties();

    int getNumberWaiting();

    void reset();

    virtual void process(ZooKeeperEvent& zkEvent);

private:
    int parties_;
    std::string root_;
    std::string name_; // current process
    bool isBarrierComplete_;

    ZooKeeper* zk_;
    static boost::timed_mutex t_mutex_;

};

}}

#endif /* CYCLIC_BARRIER_H_ */
