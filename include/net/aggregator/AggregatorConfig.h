/**
 * @file AggregatorConfig.h
 * @author Zhongxia Li
 * @date Jun 23, 2011
 * @brief 
 */
#ifndef AGGREGATOR_CONFIG_H_
#define AGGREGATOR_CONFIG_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>

#include "JobInfo.h"

namespace net{
namespace aggregator{

class AggregatorConfig
{
public:
    AggregatorConfig() {}

    void reset()
    {
        workerInfoList_.clear();
    }

    void addWorker(const std::string& host, uint16_t port, uint32_t workerid, bool isLocal=false)
    {
        WorkerServerInfo workerInfo(host, port, workerid, isLocal);
        workerInfoList_.push_back(workerInfo);
    }

    const std::vector<WorkerServerInfo>& getWorkerList() const
    {
        return workerInfoList_;
    }

    const WorkerServerInfo* getWorkerSrvInfoByWorkerId(uint32_t workerid) const
    {
        for (size_t i = 0; i < workerInfoList_.size(); i++)
        {
            if (workerInfoList_[i].workerid_ == workerid)
                return &(workerInfoList_[i]);
        }

        return NULL;
    }

    std::string toString()
    {
        std::stringstream ss;
        for (size_t i = 0; i < workerInfoList_.size(); i++)
        {
            ss <<"- worker "<<workerInfoList_[i].workerid_<<", "
               <<workerInfoList_[i].host_<<":"<<workerInfoList_[i].port_
               <<(workerInfoList_[i].isLocal_ ? " (local worker)" : "")
               <<endl;
        }
        return ss.str();
    }

public:
    std::vector<WorkerServerInfo> workerInfoList_;
};


}}

#endif /* AGGREGATOR_CONFIG_H_ */
