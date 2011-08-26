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

#include "JobInfo.h"

namespace net{
namespace aggregator{

class AggregatorConfig
{
public:
    AggregatorConfig()
    :enableLocalWorker_(true)
    {}

    void addWorker(const std::string& host, uint16_t port)
    {
        ServerInfo workerInfo(host, port);
        workerInfoList_.push_back(workerInfo);
    }

    const std::vector<ServerInfo>& getWorkerList() const
    {
        return workerInfoList_;
    }

public:
    bool enableLocalWorker_;

    std::vector<ServerInfo> workerInfoList_;
};


}}

#endif /* AGGREGATOR_CONFIG_H_ */
