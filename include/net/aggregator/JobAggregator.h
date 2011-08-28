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
template <typename ConcreteAggregator, typename LocalWorkerCaller = MockWorkerCaller>
class JobAggregator
{
public:
    JobAggregator()
    : debug_(false)
    , localWorkerEnabled_(true)
    {
        init();
    }

public:
    void setAggregatorConfig(const AggregatorConfig& aggregatorConfig)
    {
        localWorkerEnabled_ = aggregatorConfig.enableLocalWorker_;

        // assign unique id for each worker, 0 for local worker
        workerid_t workerId = REMOTE_WORKER_ID_START;

        const std::vector<ServerInfo>& remoteWorkerList = aggregatorConfig.getWorkerList();

        for (size_t i = 0; i < remoteWorkerList.size(); i ++)
        {
            createWorkerClient(workerId, remoteWorkerList[i]);
            workeridList_.push_back(workerId);
            workerId ++;
        }
    }

    std::vector<workerid_t>& getWorkerIdList()
    {
        return workeridList_;
    }

    size_t getRemoteWorkerNum()
    {
        return workeridList_.size();
    }

    size_t getWorkerNum()
    {
        if (localWorkerEnabled_)
            return getRemoteWorkerNum()+1;
        else
            return getRemoteWorkerNum();
    }

    WorkerSessionPtr& getWorkerSessionById(const workerid_t workerid)
    {
        for (size_t i = 0; i < workerSessionList_.size(); i++)
        {
            if (workerid == workerSessionList_[i]->getWorkerId())
            {
                return workerSessionList_[i];
            }
        }

        return kDefaultWorkerSession_;
    }

    /**
     * Send request to all workers with the same request data, their results will be \b aggregated to resultItem.
     *
     * @param identity, identity info, pass empty string if not processed.
     * @param func, remote function name
     * @param requestItem [IN]
     * @param resultItem [IN OUT]  Note that resultItem is also a INput parameter.
     * @param timeoutSec
     */
    template <typename RequestType, typename ResultType>
    bool distributeRequest(
            const std::string& identity,
            const std::string& func,
            const RequestType& requestItem,
            ResultType& resultItem,
            timeout_t timeoutSec = DEFAULT_TIME_OUT);

    /**
     * Send a group of requests in same type to corresponding workers, each request may carry different request data,
     * their results will be \b aggregated to resultItem.
     *
     * @param identity, identity info, pass empty string if not processed.
     * @param func, remote function name
     * @param requestGroup [IN]
     * @param resultItem [IN OUT]  Note that resultItem is also a INput parameter.
     * @param timeoutSec
     */
    template <typename RequestType, typename ResultType>
    bool distributeRequest(
            const std::string& identity,
            const std::string& func,
            RequestGroup<RequestType, ResultType>& requestGroup,
            ResultType& resultItem,
            timeout_t timeoutSec = DEFAULT_TIME_OUT);


    /**
     * Send request to a specified worker, without aggregating action.
     */
    template <typename RequestType, typename ResultType>
    bool singleRequest(
            const std::string& identity,
            const std::string& func,
            const RequestType& requestItem,
            ResultType& resultItem,
            workerid_t workerid,
            timeout_t timeoutSec = DEFAULT_TIME_OUT);

public:
    /**
     * asynchronous interface XXX
     * @brief RequestType have to be serializable using MsgPack
     * @param futureHolder[OUT] request holder for getting results in the future
     * @param key[IN] addtional information
     * @param func[IN] function name to be requested
     * @param request[IN] request param
     * @param result[IN] result type param
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

        if (debug_)
        {
            cout << "#[Aggregator] send request: " << func_plus << endl;
        }

        worker_iterator_t worker;
        for (worker = workerSessionList_.begin(); worker != workerSessionList_.end(); worker++ )
        {
            workerid_t workerid = (*worker)->getWorkerId();
            if (!checkWorkerById(workeridList, workerid))
                continue;

            WorkerFuture workerFuture(
                    workerid,
                    (*worker)->getServerInfo(),
                    (*worker)->sendRequest(futureHolder.getSessionPool(), func_plus, request, result, timeout));

            futureHolder.addWorkerFuture(workerFuture);

            if (debug_)
            {
                cout << "#[Aggregator] send to worker" << workerid <<" ["
                     << (*worker)->getServerInfo().host_<<":"<<(*worker)->getServerInfo().port_
                     <<"]"<<endl;
            }
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

        join(futureHolder, func, request, result, result, checkWorkerById(workeridList, 0));
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
        join(futureHolder, func, request, result, result, checkWorkerById(workeridList, 0));
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
        ResultType* resultItem = requestGroup.resultItem_;

        RequestType* localRequest = NULL;
        ResultType* localResult = NULL;

        for (size_t i = 0; i < requestGroup.workeridList_.size(); i++)
        {
            workerid_t workerid = requestGroup.workeridList_[i];
            RequestType* request = requestGroup.requestList_[i];
            ResultType* result = requestGroup.resultList_[i];
            if (result == NULL)
            {
                result = resultItem;
            }

            // request for local worker
            if (workerid == 0)
            {
                localRequest = request;
                localResult = result;
                continue;
            }

            // request for remote workers
            WorkerSessionPtr& workerSession = getWorkerSessionById(workerid);
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
                    workerSession->sendRequest(futureHolder.getSessionPool(), func_plus, *request, *result, timeout));

            futureHolder.addWorkerFuture(workerFuture);
        }

        if (localRequest != NULL)
        {
            join(futureHolder, func, *localRequest, *localResult, *resultItem, true);
        }
        else
        {
            localRequest = new RequestType; // fake
            localResult = new ResultType;
            join(futureHolder, func, *localRequest, *localResult, *resultItem, false);
        }
    }

protected:
    void init()
    {
    }

    void createWorkerClient(const workerid_t workerId, const ServerInfo& srvInfo)
    {
        WorkerSessionPtr workerSession(new WorkerSession(srvInfo.host_, srvInfo.port_, workerId));
        workerSessionList_.push_back(workerSession);

        std::cout << "#[Aggregator] set up session to worker"<<workerId<<" ["
                <<srvInfo.host_<<":"<<srvInfo.port_<<"]"<<std::endl;
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
    /**
     * Join and aggregate results
     * @param futureList [IN]
     * @param resultItem [INOUT]
     * @param localResult [IN]
     * @return
     */
    template <typename ResultType>
    bool join(
            const std::string& func,
            std::vector<std::pair<workerid_t, future_t> >& futureList,
            ResultType& resultItem,
            ResultType* localResult = NULL);

    template <typename RequestType, typename ResultType>
    void join(WorkerFutureHolder& futureHolder,
              const std::string& func,
              const RequestType& locRequest,
              const ResultType& locResult,
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
                ResultType resultCopy = locResult; // avoid to modify result param
                if (static_cast<ConcreteAggregator*>(this)->get_local_result(func, locRequest, resultCopy, error))
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
    bool debug_;
protected:
    typedef std::vector<WorkerSessionPtr>::iterator worker_iterator_t;

    bool localWorkerEnabled_;
    boost::shared_ptr<LocalWorkerCaller> localWorkerCaller_;

    std::vector<WorkerSessionPtr> workerSessionList_;
    std::vector<workerid_t> workeridList_;

    static const std::vector<workerid_t> NullWorkeridList;
    static WorkerSessionPtr kDefaultWorkerSession_;
};

template <class ConcreteAggregator, class LocalWorkerCaller>
const std::vector<workerid_t> JobAggregator<ConcreteAggregator, LocalWorkerCaller>::NullWorkeridList;

template <class ConcreteAggregator, class LocalWorkerCaller>
WorkerSessionPtr JobAggregator<ConcreteAggregator, LocalWorkerCaller>::kDefaultWorkerSession_;

template <class ConcreteAggregator, class LocalWorkerCaller> template<typename RequestType, typename ResultType>
bool JobAggregator<ConcreteAggregator, LocalWorkerCaller>::distributeRequest(
        const std::string& identity,
        const std::string& func,
        const RequestType& requestItem,
        ResultType& resultItem,
        timeout_t timeoutSec)
{
    std::string func_plus = identity+REQUEST_FUNC_DELIMETER+func;
    if (debug_)
        cout << "#[Aggregator] distribute request: " << func_plus << endl;

    // distribute request to remote workers
    std::vector<std::pair<workerid_t, future_t> > futureList;
    session_pool_t sessionPool; // shared sessoin pool
    worker_iterator_t worker;
    for (worker = workerSessionList_.begin(); worker != workerSessionList_.end(); worker++)
    {
        workerid_t workerid = (*worker)->getWorkerId();
        const ServerInfo& workerSrv = (*worker)->getServerInfo();

        session_t session = sessionPool.get_session(workerSrv.host_, workerSrv.port_);
        session.set_timeout(timeoutSec);

        future_t future = session.call(func_plus, requestItem, resultItem);
        futureList.push_back(std::make_pair(workerid, future));

        if (debug_)
            cout << "#[Aggregator] send to worker" << workerid <<" ["
                 << workerSrv.host_<<":"<<workerSrv.port_<<"]"<<endl;
    }

    // join (aggregate)
    if (localWorkerEnabled_ && localWorkerCaller_)
    {
        ResultType localResult = resultItem;
        std::string error;
        if (localWorkerCaller_->call(func, requestItem, localResult, error))
        {
            return join(func, futureList, resultItem, &localResult);
        }
        else
        {
            return join(func, futureList, resultItem);
        }
    }
    else
    {
        return join(func, futureList, resultItem);
    }
}

template <class ConcreteAggregator, class LocalWorkerCaller> template<typename RequestType, typename ResultType>
bool JobAggregator<ConcreteAggregator, LocalWorkerCaller>::distributeRequest(
        const std::string& identity,
        const std::string& func,
        RequestGroup<RequestType, ResultType>& requestGroup,
        ResultType& resultItem,
        timeout_t timeoutSec)
{
    std::string func_plus = identity+REQUEST_FUNC_DELIMETER+func;
    if (debug_)
        cout << "#[Aggregator] distribute request: " << func_plus << endl;

    // distribute group of requests to remote workers
    RequestType* pLocalRequest = NULL;
    ResultType* pLoacalResult = NULL;
    std::vector<std::pair<workerid_t, future_t> > futureList;
    session_pool_t sessionPool; // shared sessoin pool
    for (size_t i = 0; i < requestGroup.workeridList_.size(); i++)
    {
        workerid_t workerid = requestGroup.workeridList_[i];
        RequestType* pRequest = requestGroup.requestList_[i];
        ResultType* pResult = requestGroup.resultList_[i];
        if (pResult == NULL)  {
            pResult = &resultItem;
        }

        // Local worker is not called remotely
        if (workerid == LOCAL_WORKER_ID)
        {
            pLocalRequest = pRequest;
            pLoacalResult = pResult;
            continue;
        }

        WorkerSessionPtr& workerSession = getWorkerSessionById(workerid);
        if (!workerSession.get())
        {
            cout << "#[Aggregator] Error: not found worker" << workerid <<endl;
            continue;
        }

        const ServerInfo& workerSrv = workerSession->getServerInfo();
        session_t session = sessionPool.get_session(workerSrv.host_, workerSrv.port_);
        session.set_timeout(timeoutSec);

        future_t future = session.call(func_plus, *pRequest, *pResult);
        futureList.push_back(std::make_pair(workerid, future));

        if (debug_)
            cout << "#[Aggregator] send to worker" << workerid <<" ["
                 << workerSrv.host_<<":"<<workerSrv.port_<<"]"<<endl;
    }

    // join (aggregate)
    if (pLocalRequest != NULL && localWorkerEnabled_ && localWorkerCaller_)
    {
        std::string error;
        if (localWorkerCaller_->call(func, *pLocalRequest, *pLoacalResult, error))
        {
            return join(func, futureList, resultItem, pLoacalResult);
        }
        else
        {
            return join(func, futureList, resultItem);
        }
    }
    else
    {
        if (!localWorkerEnabled_) {
            std::cerr << "#[Aggregator] Error: local worker was required but not enabled." <<endl;
        }

        return join(func, futureList, resultItem);
    }
}

template <class ConcreteAggregator, class LocalWorkerCaller> template<typename RequestType, typename ResultType>
bool JobAggregator<ConcreteAggregator, LocalWorkerCaller>::singleRequest(
        const std::string& identity,
        const std::string& func,
        const RequestType& requestItem,
        ResultType& resultItem,
        workerid_t workerid,
        timeout_t timeoutSec)
{
    std::string func_plus = identity+REQUEST_FUNC_DELIMETER+func;
    if (debug_)
        cout << "#[Aggregator] distribute request: " << func_plus << endl;


    if (workerid == LOCAL_WORKER_ID)
    {
        if (localWorkerEnabled_ && localWorkerCaller_)
        {
            std::string error;
            return localWorkerCaller_->call(func, requestItem, resultItem, error);
        }
        else
        {
            std::cerr << "#[Aggregator] Error: local worker was required but not enabled." <<endl;
            return false;
        }
    }

    WorkerSessionPtr& workerSession = getWorkerSessionById(workerid);
    if (!workerSession.get())
    {
        cout << "#[Aggregator] Error: not found worker" << workerid <<endl;
        return false;
    }

    session_pool_t sessionPool;
    ServerInfo& workerSrv = workerSession->getServerInfo();
    session_t session = sessionPool.get_session(workerSrv.host_, workerSrv.port_);
    session.set_timeout(timeoutSec);

    future_t future = session.call(func_plus, requestItem, resultItem);
    resultItem = future.get<ResultType>();

    return true;
}

template <class ConcreteAggregator, class LocalWorkerCaller> template<typename ResultType>
bool JobAggregator<ConcreteAggregator, LocalWorkerCaller>::join(
        const std::string& func,
        std::vector<std::pair<workerid_t, future_t> >& futureList,
        ResultType& resultItem,
        ResultType* localResult)
{
    std::vector<std::pair<workerid_t, ResultType> > resultList;
    if (NULL != localResult)
    {
        resultList.push_back(std::make_pair(LOCAL_WORKER_ID, *localResult));
    }

    std::vector<std::pair<workerid_t, future_t> >::iterator it;
    for (it = futureList.begin(); it != futureList.end(); it++)
    {
        workerid_t workerid = it->first;
        future_t& future = it->second;

        bool state = false;
        std::string info;
        try
        {
            resultList.push_back(std::make_pair(workerid, future.get<ResultType>()));
            state = true;
        }
        catch (msgpack::rpc::connect_error& e)
        {
            info = ("connecting failed!");
        }
        catch (msgpack::rpc::timeout_error& e)
        {
            info = ("request timed out!");
        }
        catch (msgpack::rpc::no_method_error& e)
        {
            info = ("method not found!");
        }
        catch (msgpack::rpc::argument_error& e)
        {
            info = ("argument mismatch!");
        }
        catch (msgpack::rpc::remote_error& e)
        {
            info = ("remote_error");
        }
        catch (std::exception& e)
        {
            info = e.what();
        }

        if (!state)
        {
            cout <<"#[Aggregator] Got error ("<<info<<") from worker"<<workerid;

            WorkerSessionPtr workerSession = getWorkerSessionById(workerid);
            if (workerSession)
            {
                cout << " ["<<workerSession->getServerInfo().host_
                     << ":"<<workerSession->getServerInfo().port_ <<"] "
                     << endl;
            }
            else
            {
                cout <<", not found worker!"<<endl;
            }
        }
    }

    if (resultList.size() > 0)
    {
        return static_cast<ConcreteAggregator*>(this)->aggregate(func, resultItem, resultList);
    }
    else
    {
        return false;
    }
}

}} // end - namespace

#endif /* JOB_AGGREGATOR_H_ */
