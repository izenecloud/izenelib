/**
 * @file JobAggregator.h
 * @author Zhongxia Li
 * @date Jun 23, 2011
 * @brief JobAggregator distribute Job to multiple Workers and gather results.
 */
#ifndef JOB_AGGREGATOR_H_
#define JOB_AGGREGATOR_H_

#include <boost/shared_ptr.hpp>
#include <3rdparty/msgpack/rpc/client.h>

#include "AggregatorConfig.h"


namespace net{
namespace aggregator{

class JobAggregator
{
    typedef msgpack::rpc::client RPCClient;
    typedef msgpack::rpc::future RPCFutureReply;

public:
    JobAggregator()
    {
    }

    JobAggregator(const std::string& host, uint16_t port)
    : srvInfo_(host, port)
    {
    }

public:
    void setWorkerListConfig(AggregatorConfig& aggregatorConfig)
    {
        const std::vector<ServerInfo>& workerInfoList = aggregatorConfig.getWorkerList();

        for (size_t i = 0; i < workerInfoList.size(); i ++)
        {
            registerWorker(workerInfoList[i]);
        }
    }

    /**
     *
     */
//    template<typename DataT, typename MethodT = std::string>
//    void call(const MethodT& method, const DataT DataObject)
//    {
//
//    }

    void request(const std::string& func)
    {
        async_call();

        join();
    }

private:
    void init()
    {

    }

    void registerWorker(const ServerInfo& srvInfo)
    {
        if (isLocalWorker(srvInfo)) {
            //
        }
        else {
            createWorkerClient(srvInfo);
        }
    }

    bool isLocalWorker(const ServerInfo& srvInfo)
    {
        return false; // XXX
    }

    void createWorkerClient(const ServerInfo& srvInfo)
    {
        try
        {
            boost::shared_ptr<RPCClient> workerClt(
                    new RPCClient(srvInfo.host_, srvInfo.port_)
            );
            workerClientList_.push_back(workerClt);
        }
        catch(std::exception& e)
        {
            std::cerr<<"Failed connecting to: "<<srvInfo.host_<<":"<<srvInfo.port_<<std::endl
                     <<e.what()<<std::endl;
        }
    }

private:
    void async_call()
    {
        workerReplyList_.clear();

        worker_iterator witer = workerClientList_.begin();
        for (; witer != workerClientList_.end(); witer++)
        {
            RPCFutureReply frep = (*witer)->call("add", 1, 1);
            workerReplyList_.push_back( frep );
        }
    }

    void join()
    {
        int ret = 0;

        worker_ret_iterator writer = workerReplyList_.begin();
        for (; writer != workerReplyList_.end(); writer++)
        {
            try
            {
                ret += writer->get<int>(); // inner join
            }
            catch (msgpack::rpc::connect_error& e)
            {
                // "connect failed", XXX

            }
            catch (msgpack::rpc::timeout_error& e)
            {
                // "request timed out"
            }
            catch (msgpack::rpc::no_method_error& e)
            {
                // "method not found"
            }
            catch (msgpack::rpc::argument_error& e)
            {
                // "argument mismatch"
            }
            catch (msgpack::rpc::remote_error& e)
            {
                // "remote_error"
            }
            catch (std::exception& e)
            {
                std::cerr<<e.what()<<std::endl;
            }
        }

        std::cout << "join: "<<ret << std::endl;
    }

private:
    ServerInfo srvInfo_;

    typedef std::vector<boost::shared_ptr<RPCClient> >::iterator worker_iterator;
    typedef std::vector<RPCFutureReply>::iterator worker_ret_iterator;
    std::vector<boost::shared_ptr<RPCClient> > workerClientList_;
    std::vector<RPCFutureReply> workerReplyList_;

};

}} // end - namespace

#endif /* JOB_AGGREGATOR_H_ */
