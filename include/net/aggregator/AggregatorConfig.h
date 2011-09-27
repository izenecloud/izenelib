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
    AggregatorConfig()
    :enableLocalWorker_(false)
    {}

    void reset()
    {
        workerInfoList_.clear();
    }

    void addWorker(const std::string& host, uint16_t port)
    {
        ServerInfo workerInfo(host, port);
        workerInfoList_.push_back(workerInfo);
    }

    const std::vector<ServerInfo>& getWorkerList() const
    {
        return workerInfoList_;
    }

    std::string toString()
    {
        std::stringstream ss;
        ss <<"[AggregatorConfig] localWorker enabled ? "<<enableLocalWorker_<<endl;

        for (size_t i = 0; i < workerInfoList_.size(); i++)
        {
            ss << "worker="<<workerInfoList_[i].host_<<":"<<workerInfoList_[i].port_<<endl;
        }

        return ss.str();
    }

public:
    bool enableLocalWorker_;

    std::vector<ServerInfo> workerInfoList_;
};


}}

#endif /* AGGREGATOR_CONFIG_H_ */
