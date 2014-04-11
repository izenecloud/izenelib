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
        unsigned int timeout = 15,
        unsigned int sessionPoolThreadNum = 2)
    : is_ro_(false), timeout_(timeout)
    , sessionPoolThreadNum_(sessionPoolThreadNum)
    {
    }

    void reset()
    {
        workerInfoList_.clear();
        is_ro_ = false;
    }

    void setReadyOnly()
    {
        is_ro_ = true;
        if (sessionPoolThreadNum_ < 10)
            sessionPoolThreadNum_ = 10;
    }

    bool isReadOnly() const
    {
        return is_ro_;
    }
    void addReadOnlyWorker(const std::string& host, uint16_t port, uint32_t workerid)
    {
        WorkerServerInfo workerInfo(host, port, workerid, false);
        ro_workerInfoList_.push_back(workerInfo);
    }
    const std::vector<WorkerServerInfo>& getReadOnlyWorkerList() const
    {
        return ro_workerInfoList_;
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

    std::string toString() const
    {
        std::stringstream ss;
        for (size_t i = 0; i < workerInfoList_.size(); i++)
        {
            ss <<"- worker "<<workerInfoList_[i].workerid_<<", "
               <<workerInfoList_[i].host_<<":"<<workerInfoList_[i].port_
               <<(workerInfoList_[i].isLocal_ ? " (local worker)" : "")
               <<endl;
        }
        for (size_t i = 0; i < ro_workerInfoList_.size(); i++)
        {
            ss <<"- read only worker "<< ro_workerInfoList_[i].workerid_<<", "
               << ro_workerInfoList_[i].host_<<":"<< ro_workerInfoList_[i].port_
               <<(ro_workerInfoList_[i].isLocal_ ? " (local worker)" : "")
               <<endl;
        }

        return ss.str();
    }

private:
    bool is_ro_;
    std::vector<WorkerServerInfo> workerInfoList_;
    std::vector<WorkerServerInfo> ro_workerInfoList_;
    unsigned int timeout_; // second
    unsigned int sessionPoolThreadNum_;
};


}}

#endif /* AGGREGATOR_CONFIG_H_ */
