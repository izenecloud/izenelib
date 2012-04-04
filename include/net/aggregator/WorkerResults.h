/**
 * @file WorkerResults.h
 * @brief a result list gathered from each worker
 * @author Jun Jiang
 * @date 2012-04-03
 */

#ifndef IZENE_NET_AGGREGATOR_WORKER_RESULTS_H_
#define IZENE_NET_AGGREGATOR_WORKER_RESULTS_H_

#include "Typedef.h"
#include <utility>
#include <vector>

namespace net{
namespace aggregator{

template <typename ResultT>
struct WorkerResults
{
    typedef std::pair<workerid_t, ResultT> pair_t;
    typedef std::vector<pair_t> list_t;

    list_t results_;

    void add(workerid_t id, const ResultT& result)
    {
        results_.push_back(pair_t(id, result));
    }

    bool empty() const { return results_.empty(); }

    std::size_t size() const { return results_.size(); }

    workerid_t workerId(std::size_t i) const
    {
        return results_[i].first;
    }

    const ResultT& result(std::size_t i) const
    {
        return results_[i].second;
    }
};

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_WORKER_RESULTS_H_
