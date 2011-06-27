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

namespace net{
namespace aggregator{

struct ServerInfo
{
    std::string host_;
    uint16_t port_;

    ServerInfo(const std::string& host, uint16_t port)
    :host_(host), port_(port)
    {
    }

    ServerInfo(){}
};

class AggregatorConfig
{
public:
    void addWorker(const std::string& host, uint16_t port)
    {
        ServerInfo workerInfo(host, port);
        workerInfoList_.push_back(workerInfo);
    }

    const std::vector<ServerInfo>& getWorkerList() const
    {
        return workerInfoList_;
    }

private:
    std::vector<ServerInfo> workerInfoList_;
};


}}

#endif /* AGGREGATOR_CONFIG_H_ */
