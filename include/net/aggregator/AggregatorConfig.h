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

#include "Typedef.h"

namespace net{
namespace aggregator{

class AggregatorConfig
{
public:
    AggregatorConfig(
        unsigned int timeout = 30,
        unsigned int sessionPoolThreadNum = 30)
    : timeout_(timeout)
    , sessionPoolThreadNum_(sessionPoolThreadNum)
    {
    }

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

    unsigned int getTimeout() const
    {
        return timeout_;
    }

    unsigned int getSessionPoolThreadNum() const
    {
        return sessionPoolThreadNum_;
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

private:
    std::vector<WorkerServerInfo> workerInfoList_;
    unsigned int timeout_; // second
    unsigned int sessionPoolThreadNum_;
};


}}

#endif /* AGGREGATOR_CONFIG_H_ */
