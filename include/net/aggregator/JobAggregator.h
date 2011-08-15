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
 * join_impl(ResultType& result, const std::vector<std::pair<workerid_t, ResultType> >& resultList);
 * join_impl() used to merge result, it's not virtual, because base class won't known the ResultType
 * of a concrete aggregator. (Overload this function with different ResultTypes)
 *
 * Implement get_local_result(), in which call local worker, if needed.
 * template <typename RequestType, typename ResultType>
 * bool get_local_result(const std::string& func, const RequestType& request, ResultType& result, std::string& error)
 *
 * @example
 *
class SearchAggregator : public JobAggregator<SearchAggregator>
{
public:
    void join_impl(ResultType1& result, const std::vector<std::pair<workerid_t, ResultType1> >& resultList)
    {
        // merge resultList to result
    }

    void join_impl(ResultType2& result, const std::vector<std::pair<workerid_t, ResultType2> >& resultList)
    {
        // merge resultList to result
    }

    ...
};
 *
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
    void setWorkerListConfig(const AggregatorConfig& aggregatorConfig)
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

    /**
     * asynchronous interface
     * @brief RequestType have to be serializable using MsgPack
     * @param futureHolder[OUT] request holder for getting results in the future
     * @param func[IN] function name to be requested
     * @param request[IN] request data
     * @param workeridList[IN] spedified workers to request, request all if empty.
     * @param timeout[IN] timeout in seconds
     */
    template <typename RequestType, typename ResultType>
    void sendRequest(
            WorkerFutureHolder& futureHolder,
            const std::string& key,
            const std::string& func,
            const RequestType& request,
            const ResultType& result,
            const std::vector<workerid_t>& workeridList = NullWorkeridList,
            unsigned int timeout = 7)
    {
        std::string func_plus = key+REQUEST_FUNC_DELIMETER+func;
        cout << "#[Aggregator] send request: " << func_plus << endl;

        worker_iterator_t worker = workerSessionList_.begin();
        for ( ; worker != workerSessionList_.end(); worker++ )
        {
            workerid_t workerid = (*worker)->getWorkerId();
            if ( !checkWorkerById(workeridList, workerid) )
                continue;

            cout << "#[Aggregator] send to worker" << workerid <<" ["
                 << (*worker)->getServerInfo().host_<<":"<<(*worker)->getServerInfo().port_
                 <<"]"<<endl;

            WorkerFuture workerFuture(
                    workerid,
                    (*worker)->getServerInfo(),
                    (*worker)->sendRequest(futureHolder.getSessionPool(), func_plus, request, result, timeout));

            futureHolder.addWorkerFuture(workerFuture);
        }
    }

    /**
     * get result after asynchronous call
     * @brief ResultType have to be serializable using MsgPack
     * @param futureHolder[IN] holder for getting results
     * @param result[OUT] aggregated result
     * @param workeridList[IN] WARN: make sure this parameter is consistent with sendRequest.
     */
    template <typename RequestType, typename ResultType>
    void getResult(
            WorkerFutureHolder& futureHolder,
            const std::string& func,
            const RequestType& request,
            ResultType& result,
            const std::vector<workerid_t>& workeridList = NullWorkeridList)
    {

        join(futureHolder, func, request, result, checkWorkerById(workeridList, 0));
    }


    /**
     * synchronous interface
     * @brief a merged interface for sendRequest & getResult
     */
    template <typename RequestType, typename ResultType>
    void sendRequest(
            const std::string& key,
            const std::string& func,
            const RequestType& request,
            ResultType& result,
            const std::vector<workerid_t>& workeridList = NullWorkeridList,
            unsigned int timeout = 10)
    {
        WorkerFutureHolder futureHolder;

        // send requests
        sendRequest(futureHolder, key, func, request, result, workeridList, timeout);

        // get result
        join(futureHolder, func, request, result, checkWorkerById(workeridList, 0));
    }

    /**
     * Send a group of requests, requests may be have different values to different workers,
     * but the results will be merged to a same resultItem set to requestGroup.
     * @param key
     * @param func
     * @param requestGroup
     * @param timeout
     */
    template <typename RequestType, typename ResultType>
    void sendRequest(
            const std::string& key,
            const std::string& func,
            RequestGroup<RequestType, ResultType>& requestGroup,
            unsigned int timeout = 10)
    {
        if (!requestGroup.check())
        {
            cout << "#[Aggregator:error] requestGroup may not fully initialized." <<endl;
            return;
        }

        std::string func_plus = key+REQUEST_FUNC_DELIMETER+func;
        cout << "#[Aggregator] send request: " << func_plus << endl;

        WorkerFutureHolder futureHolder;
        boost::shared_ptr<ResultType>& resultItem = requestGroup.resultItem_;

        boost::shared_ptr<RequestType> localRequest;

        typename RequestGroup<RequestType, ResultType>::request_iterator_t it;
        for (it = requestGroup.requestList_.begin(); it != requestGroup.requestList_.end(); it++)
        {
            workerid_t workerid = it->first;
            boost::shared_ptr<RequestType>& request = it->second;

            // request for local worker
            if (workerid == 0)
            {
                localRequest = request;
                continue;
            }

            // request for remote workers
            WorkerSessionPtr& workerSession = getWorkerSession(workerid);
            if (!workerSession.get())
            {
                cout << "#[Aggregator:error] Not found worker" << workerid <<endl;
                continue;
            }

            cout << "#[Aggregator] send to worker" << workerid <<" ["
                 << workerSession->getServerInfo().host_<<":"<<workerSession->getServerInfo().port_
                 <<"]"<<endl;

            WorkerFuture workerFuture(
                    workerid,
                    workerSession->getServerInfo(),
                    workerSession->sendRequest(futureHolder.getSessionPool(), func_plus, *request, *resultItem, timeout));

            futureHolder.addWorkerFuture(workerFuture);
        }

        if (localRequest.get())
        {
            join(futureHolder, func, *localRequest, *resultItem, true);
        }
        else
        {
            localRequest.reset(new RequestType()); // fake
            join(futureHolder, func, *localRequest, *resultItem, false);
        }
    }

protected:
    void init()
    {

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
            workerSessionList_.push_back(workerSession);
            std::cout << "#[Aggregator] set session to worker: " <<srvInfo.host_<<":"<<srvInfo.port_<<std::endl;
        }
        catch(std::exception& e)
        {
            std::cerr<<"Failed connecting to: "<<srvInfo.host_<<":"<<srvInfo.port_<<std::endl
                     <<e.what()<<std::endl;
        }
    }

    bool checkWorkerById(const std::vector<workerid_t>& workeridList, workerid_t workerid)
    {
        if (workeridList.size() == 0)
            return true;

        for (size_t i = 0; i < workeridList.size(); i++)
        {
            if (workerid == workeridList[i])
                return true;
        }

        return false;
    }

protected:
    template <typename RequestType, typename ResultType>
    void join(WorkerFutureHolder& futureHolder,
              const std::string& func,
              const RequestType& request,
              ResultType& result,
              bool isCallLocalWorker)
    {
        std::vector<std::pair<workerid_t, ResultType> > resultList;

        //cout << "#[Aggregator] join " << endl;
        // local worker
        try
        {
            if (isCallLocalWorker)
            {
                cout << "#[Aggregator] call worker0 (local)" << endl;
                std::string error;
                ResultType resultCopy = result; // avoid to change result
                if (static_cast<ConcreteAggregator*>(this)->get_local_result(func, request, resultCopy, error))
                {
                    resultList.push_back(std::make_pair(0, resultCopy));
                }
                else {
                    cout << "#[Aggregator:error] worker0 " << error << endl;
                }
            }
        }
        catch(std::exception& e)
        {
            cout << e.what() <<endl;
        }

        // remote worker
        std::vector<WorkerFuture>& futureList = futureHolder.getFutureList();
        std::vector<WorkerFuture>::iterator workerFuture = futureList.begin();
        time_t t1 = time(NULL);
        for (; workerFuture != futureList.end(); workerFuture++)
        {
            ///time_t t1 = time(NULL); //test
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

            ///time_t t2 = time(NULL); //test

            if ( !workerFuture->getState() )
            {
                cout <<"#[Aggregator:error] worker"<<workerFuture->getWorkerId()
                     <<" ["<<workerFuture->getServerInfo().host_
                     <<":"<<workerFuture->getServerInfo().port_
                     <<"] "<<workerFuture->getError()
                     ///<<" , cost "<<(t2-t1)
                     << endl;
            }
        }
        time_t t2 = time(NULL);
        cout << "#[Aggregator] got remote worker results, cost(s): " << (t2-t1) << endl;

        if (resultList.size() > 0)
        {
            static_cast<ConcreteAggregator*>(this)->join_impl(func, result, resultList);
        }
        else
        {
            ;
        }
    }

public:
    WorkerSessionPtr& getWorkerSession(const workerid_t workerid)
    {
        worker_iterator_t worker = workerSessionList_.begin();
        for ( ; worker != workerSessionList_.end(); worker++ )
        {
            if ( (*worker)->getWorkerId() == workerid )
            {
                return *worker;
            }
        }

        return kDefaultWorkerSession_;
    }

protected:
    ServerInfo srvInfo_;

    typedef std::vector<WorkerSessionPtr>::iterator worker_iterator_t;
    std::vector<WorkerSessionPtr> workerSessionList_;

    static const std::vector<workerid_t> NullWorkeridList;
    static WorkerSessionPtr kDefaultWorkerSession_;
};

template <typename ConcreteAggregator>
const std::vector<workerid_t> JobAggregator<ConcreteAggregator>::NullWorkeridList;

template <typename ConcreteAggregator>
WorkerSessionPtr JobAggregator<ConcreteAggregator>::kDefaultWorkerSession_;


}} // end - namespace

#endif /* JOB_AGGREGATOR_H_ */
