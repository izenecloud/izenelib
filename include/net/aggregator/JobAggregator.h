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
#include "JobInfo.h"


namespace net{
namespace aggregator{

/**
 * Job Aggregator base class
 * @brief Implement a concrete aggregator using inheritance, the following function must be implemented:
 * join_impl(DataType& result, const DataType& singleWorkerResult);
 * join_impl() used to merge result incrementally, it's not virtual, because base class won't known the DataType
 * of a concrete aggregator.
 */
template <typename ConcreteAggregator>
class JobAggregator
{
    typedef msgpack::rpc::client RPCClient;
    typedef msgpack::rpc::future RPCFutureReply;

public:
    /// todo, remove
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

    template <typename RequestParamType, typename ResultParamType>
    void processRequest(
            const std::string& func,
            const RequestParamType& request,
            ResultParamType& result,
            unsigned int timeout = 2) // xxx
    {
        async_call(func, request, timeout);

        join(result);
    }

protected:
    void init()
    {

    }

    void registerWorker(const ServerInfo& srvInfo)
    {
        if (isLocalWorker(srvInfo)) {
            // xxx
        }
        else {
            createWorkerClient(srvInfo);
        }
    }

    bool isLocalWorker(const ServerInfo& srvInfo)
    {
        if (srvInfo.host_ == srvInfo_.host_
                && srvInfo.port_ == srvInfo.port_)
        {
            return true;
        }
        return false;
    }

    void createWorkerClient(const ServerInfo& srvInfo)
    {
        try
        {
            boost::shared_ptr<RPCClient> workerClt( new RPCClient(srvInfo.host_, srvInfo.port_) );
            workerClientList_.push_back(workerClt);
        }
        catch(std::exception& e)
        {
            std::cerr<<"Failed connecting to: "<<srvInfo.host_<<":"<<srvInfo.port_<<std::endl
                     <<e.what()<<std::endl;
        }
    }

protected:
    template <typename RequestParamType>
    void async_call(const std::string& func, const RequestParamType& param, unsigned int timeout)
    {
        workerReplyList_.clear();

        worker_iterator witer = workerClientList_.begin();
        for (; witer != workerClientList_.end(); witer++)
        {
            (*witer)->set_timeout(timeout);
            RPCFutureReply frep = (*witer)->call(func, param);
            workerReplyList_.push_back( frep );
        }
    }

    template <typename ResultParamType>
    void join(ResultParamType& result)
    {
        worker_ret_iterator wrIter = workerReplyList_.begin();
        for (; wrIter != workerReplyList_.end(); wrIter++)
        {
            try
            {
                ResultParamType workerResult = wrIter->get<ResultParamType>(); // inner join
                static_cast<ConcreteAggregator*>(this)->join_impl(result, workerResult);
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
                cout << "method not found!" <<endl;
            }
            catch (msgpack::rpc::argument_error& e)
            {
                // "argument mismatch"
                cout << "argument mismatch!" <<endl;
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
