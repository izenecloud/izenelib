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
 * join_impl(DataType& result, const std::vector<std::pair<workerid_t, ResultType> >& resultList);
 * join_impl() used to merge result, it's not virtual, because base class won't known the ResultType
 * of a concrete aggregator.
 * @example

class SearchAggregator : public JobAggregator<SearchAggregator>
{
public:
    void join_impl(ResultType& result, const ResultType& singleWorkerResult)
    {
        // merge singleWorkerResult to result
    }
};

 */
template <typename ConcreteAggregator>
class JobAggregator
{
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
        // assign unique id for each worker, 0 for local worker
        workerid_t workerId = 1;

        const std::vector<ServerInfo>& workerInfoList = aggregatorConfig.getWorkerList();

        for (size_t i = 0; i < workerInfoList.size(); i ++)
        {
            if (isLocalWorker(workerInfoList[i])) {
                // xxx
            }
            else {
                createWorkerClient(workerInfoList[i], workerId);
                workerId ++;
            }
        }
    }


    template <typename RequestType>
    void sendRequest(
            WorkerFutureHolder& futureHolder,
            const std::string& func,
            const RequestType& request,
            unsigned int timeout = 2) // xxx
    {
        worker_iterator_t worker = workerSessionPool_.begin();
        for ( ; worker != workerSessionPool_.end(); worker++ )
        {
            WorkerFuture workerFuture(
                    (*worker)->getWorkerId(),
                    (*worker)->getServerInfo(),
                    (*worker)->sendRequest(func, request));

            futureHolder.addWorkerFuture(workerFuture);
        }

    }

    template <typename ResultType>
    void getResult(
            WorkerFutureHolder& futureHolder,
            ResultType& result)
    {
        join(futureHolder, result);
    }

protected:
    void init()
    {

    }

    /// reserved
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
                && srvInfo.port_ == srvInfo_.port_)
        {
            return true;
        }
        return false;
    }

    void createWorkerClient(const ServerInfo& srvInfo, const workerid_t workerId)
    {
        try
        {
            WorkerSessionPtr workerSession(new WorkerSession(srvInfo.host_, srvInfo.port_, workerId));
            workerSessionPool_.push_back(workerSession);
        }
        catch(std::exception& e)
        {
            std::cerr<<"Failed connecting to: "<<srvInfo.host_<<":"<<srvInfo.port_<<std::endl
                     <<e.what()<<std::endl;
        }
    }

protected:
    template <typename ResultType>
    void join(WorkerFutureHolder& futureHolder, ResultType& result)
    {
        std::vector<std::pair<workerid_t, ResultType> > resultList;

        std::vector<WorkerFuture>& futureList = futureHolder.getFutureList();
        std::vector<WorkerFuture>::iterator workerFuture = futureList.begin();
        for (; workerFuture != futureList.end(); workerFuture++)
        {
            try
            {
                ResultType workerResult = workerFuture->getResult<ResultType>();
                unsigned int workerId = workerFuture->getWorkerId();
                resultList.push_back(std::make_pair(workerId, workerResult));
            }
            catch (msgpack::rpc::connect_error& e)
            {
                // xxx
                workerFuture->setError("connecting failed!");
            }
            catch (msgpack::rpc::timeout_error& e)
            {
                workerFuture->setError("request timed out!");
            }
            catch (msgpack::rpc::no_method_error& e)
            {
                workerFuture->setError("method not found!");
            }
            catch (msgpack::rpc::argument_error& e)
            {
                workerFuture->setError("argument mismatch!");
            }
            catch (msgpack::rpc::remote_error& e)
            {
                workerFuture->setError("remote_error");
            }
            catch (std::exception& e)
            {
                workerFuture->setError(e.what());
            }

            if ( !workerFuture->getState() )
            {
                cout <<"["<<workerFuture->getServerInfo().host_
                     <<":"<<workerFuture->getServerInfo().port_
                     <<"] "<<workerFuture->getError() << endl;
            }
        }

        if (resultList.size() > 0)
        {
            static_cast<ConcreteAggregator*>(this)->join_impl(result, resultList);
        }
        else
        {
            ;
        }
    }

private:
    ServerInfo srvInfo_;

    typedef std::vector<WorkerSessionPtr>::iterator worker_iterator_t;
    std::vector<WorkerSessionPtr> workerSessionPool_;
};

}} // end - namespace

#endif /* JOB_AGGREGATOR_H_ */
