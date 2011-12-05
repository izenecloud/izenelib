/**
 * @file Aggregator.h
 * @author Zhongxia Li
 * @date Jun 23, 2011
 * @brief Aggregator distribute requests to multiple Workers and gather results.
 */
#ifndef IZENE_NET_AGGREGATOR_H_
#define IZENE_NET_AGGREGATOR_H_

#include "AggregatorConfig.h"
#include "Typedef.h"
#include "WorkerCaller.h"

#include <boost/shared_ptr.hpp>
#include <3rdparty/msgpack/rpc/client.h>


namespace net{
namespace aggregator{

/**
 * Base class for Aggregator
 * @brief Aggregator can distribute requests to Workers & gather results returned by Workers. <br>
 * To send requests, we can ues \b distributeRequest to broadcast, and use \b singleRequest to a specified Worker. <br>
 *
 * To gather and merge results (optional), we can implement multiple \b aggregate methods in a Concrete Aggregator, <br>
 * the parameters for a \a aggregate method will be a list of results in concrete type, and a ref to a result object <br>
 * where to save merged result. The prototype of \b aggregate method is: <br>
 *     aggregate(ResultType& result, const std::vector<std::pair<workerid_t, ResultType> >& resultList);             <br>
 *
 * For sending requests, rpc (remote procedure call) will be performed to call remote Workers,                       <br>
 * to support directly call (in-process) to local Worker, we can pass a WorkerCaller<WorkerT> in constructor to      <br>
 * initialize localWorkerCaller_ for Aggregator.
 *
 * @example

typedef WorkerCaller<WorkerT> LocalWorkerCaller;

class SearchAggregator : public Aggregator<SearchAggregator, LocalWorkerCaller>
{
public:
    SearchAggregator()
    {
    }

    SearchAggregator(SearchService* searchService)
    {
        localWorkerCaller_.reset(new LocalWorkerCaller(searchService));

        // add functions to worker caller
        ADD_FUNC_TO_WORKER_CALLER(LocalWorkerCaller, localWorkerCaller_, SearchService, getKeywordSearchResult);
    }

public:
    void aggregate(const std::string& func, ResultType1& result, const std::vector<std::pair<workerid_t, ResultType1> >& resultList)
    {
        // merge results of resultList to result
    }

    void aggregate(const std::string& func, ResultType2& result, const std::vector<std::pair<workerid_t, ResultType2> >& resultList)
    {
        // merge results of resultList to result
    }
};

 *
 */

class AggregatorBase
{
public:
    virtual void setAggregatorConfig(const AggregatorConfig& aggregatorConfig) = 0;
};

template <typename ConcreteAggregator, typename LocalWorkerCaller = MockWorkerCaller>
class Aggregator : public AggregatorBase
{
public:
    Aggregator()
    : debug_(false)
    , hasLocalWorker_(false)
    , localWorkerId_(0)
    , isCallLocalWorkerLocally_(true)
    {
        init();
    }

public:
    virtual void setAggregatorConfig(const AggregatorConfig& aggregatorConfig)
    {
        const std::vector<WorkerServerInfo>& workerSrvList = aggregatorConfig.getWorkerList();

        workeridList_.clear();
        workerSessionList_.clear();
        hasLocalWorker_ = false;
        localWorkerId_ = 0;
        for (size_t i = 0; i < workerSrvList.size(); i ++)
        {
            const WorkerServerInfo& workerSrv = workerSrvList[i];

            if (isCallLocalWorkerLocally_ && workerSrv.isLocal_
                    && !hasLocalWorker_ /*only 1 local*/)
            {
                hasLocalWorker_ = true;
                localWorkerId_ = workerSrv.workerid_;
                continue;
            }

            // create worker session (client)
            WorkerSessionPtr workerSession(
                    new WorkerSession(workerSrv.host_, workerSrv.port_, workerSrv.workerid_));
            workeridList_.push_back(workerSrv.workerid_);
            workerSessionList_.push_back(workerSession);
        }
    }

    std::vector<workerid_t>& getWorkerIdList()
    {
        return workeridList_;
    }

    size_t getWorkerNum()
    {
        return workeridList_.size();
    }

    void setDebug(bool debug)
    {
        debug_ = debug;
    }

    void setIsCallLocalWorkerLocally(bool isCallLocalWorkerLocally)
    {
        isCallLocalWorkerLocally_ = isCallLocalWorkerLocally;
    }

    const WorkerSessionPtr& getWorkerSessionById(const workerid_t workerid)
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

    template <typename RequestType0, typename RequestType1, typename ResultType>
    bool distributeRequest(
            const std::string& identity,
            const std::string& func,
            const RequestType0& requestItem0,
            const RequestType1& requestItem1,
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
    template<typename RequestType, typename ResultType>
    bool singleRequest(
            const std::string& identity,
            const std::string& func,
            const RequestType& requestItem,
            ResultType& resultItem,
            workerid_t workerid,
            timeout_t timeoutSec  = DEFAULT_TIME_OUT);

    template <typename RequestType0, typename RequestType1, typename ResultType>
    bool singleRequest(
            const std::string& identity,
            const std::string& func,
            const RequestType0& requestItem0,
            const RequestType1& requestItem1,
            ResultType& resultItem,
            workerid_t workerid,
            timeout_t timeoutSec = DEFAULT_TIME_OUT);

protected:
    void init()
    {
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
public:
    bool debug_;
protected:
    typedef std::vector<WorkerSessionPtr>::iterator worker_iterator_t;

    bool hasLocalWorker_;
    workerid_t localWorkerId_; // available when hasLocalWorker is true
    bool isCallLocalWorkerLocally_; // xxx reserved (true as default)
                                    // whether request local worker by in-process call,
                                    // rpc bring network communication cost.
    boost::shared_ptr<LocalWorkerCaller> localWorkerCaller_;

    std::vector<workerid_t> workeridList_;
    std::vector<WorkerSessionPtr> workerSessionList_;

    static const std::vector<workerid_t> NullWorkeridList;
    static const WorkerSessionPtr kDefaultWorkerSession_;
};

template <class ConcreteAggregator, class LocalWorkerCaller>
const std::vector<workerid_t> Aggregator<ConcreteAggregator, LocalWorkerCaller>::NullWorkeridList;

template <class ConcreteAggregator, class LocalWorkerCaller>
const WorkerSessionPtr Aggregator<ConcreteAggregator, LocalWorkerCaller>::kDefaultWorkerSession_;

template <class ConcreteAggregator, class LocalWorkerCaller>
template<typename RequestType, typename ResultType>
bool Aggregator<ConcreteAggregator, LocalWorkerCaller>::distributeRequest(
        const std::string& identity,
        const std::string& func,
        const RequestType& requestItem,
        ResultType& resultItem,
        timeout_t timeoutSec)
{
    std::string func_plus = identity+REQUEST_FUNC_DELIMETER+func;
    if (debug_)
        cout << "#[Aggregator] distribute request: " << func_plus << endl;

    if (workerSessionList_.empty())
    {
        cout << "#[Aggregator] no remote worker(s)." << endl;
    }

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
    if (hasLocalWorker_ && localWorkerCaller_)
    {
        ResultType localResult = resultItem;
        std::string error;
        if (debug_)
            cout << "#[Aggregator] call local worker"<<localWorkerId_<<endl;
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

template <class ConcreteAggregator, class LocalWorkerCaller>
template<typename RequestType0, typename RequestType1, typename ResultType>
bool Aggregator<ConcreteAggregator, LocalWorkerCaller>::distributeRequest(
        const std::string& identity,
        const std::string& func,
        const RequestType0& requestItem0,
        const RequestType1& requestItem1,
        ResultType& resultItem,
        timeout_t timeoutSec)
{
    std::string func_plus = identity+REQUEST_FUNC_DELIMETER+func;
    if (debug_)
        cout << "#[Aggregator] distribute request: " << func_plus << endl;

    if (workerSessionList_.empty())
    {
        cout << "#[Aggregator] no remote worker(s)." << endl;
    }

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

        future_t future = session.call(func_plus, requestItem0, requestItem1, resultItem);
        futureList.push_back(std::make_pair(workerid, future));

        if (debug_)
            cout << "#[Aggregator] send to worker" << workerid <<" ["
                 << workerSrv.host_<<":"<<workerSrv.port_<<"]"<<endl;
    }

    // join (aggregate)
    if (hasLocalWorker_ && localWorkerCaller_)
    {
        ResultType localResult = resultItem;
        std::string error;
        if (debug_)
            cout << "#[Aggregator] call local worker"<<localWorkerId_<<endl;
        if (localWorkerCaller_->call(func, requestItem0, requestItem1, localResult, error))
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

template <class ConcreteAggregator, class LocalWorkerCaller>
template<typename RequestType, typename ResultType>
bool Aggregator<ConcreteAggregator, LocalWorkerCaller>::distributeRequest(
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
        if (workerid == localWorkerId_)
        {
            pLocalRequest = pRequest;
            pLoacalResult = pResult;
            continue;
        }

        const WorkerSessionPtr& workerSession = getWorkerSessionById(workerid);
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
    if (pLocalRequest != NULL && hasLocalWorker_ && localWorkerCaller_)
    {
        std::string error;
        if (debug_)
            cout << "#[Aggregator] call local worker"<<localWorkerId_<<endl;
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
        if (!hasLocalWorker_) {
            std::cerr << "#[Aggregator] Error: local worker was required but not enabled." <<endl;
        }

        return join(func, futureList, resultItem);
    }
}

template <class ConcreteAggregator, class LocalWorkerCaller>
template<typename RequestType, typename ResultType>
bool Aggregator<ConcreteAggregator, LocalWorkerCaller>::singleRequest(
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


    if (workerid == localWorkerId_)
    {
        if (hasLocalWorker_ && localWorkerCaller_)
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

    const WorkerSessionPtr& workerSession = getWorkerSessionById(workerid);
    if (!workerSession.get())
    {
        cout << "#[Aggregator] Error: not found worker" << workerid <<endl;
        return false;
    }

    session_pool_t sessionPool;
    const ServerInfo& workerSrv = workerSession->getServerInfo();
    session_t session = sessionPool.get_session(workerSrv.host_, workerSrv.port_);
    session.set_timeout(timeoutSec);

    future_t future = session.call(func_plus, requestItem, resultItem);
    resultItem = future.get<ResultType>();

    return true;
}


template <class ConcreteAggregator, class LocalWorkerCaller>
template <typename RequestType0, typename RequestType1, typename ResultType>
bool Aggregator<ConcreteAggregator, LocalWorkerCaller>::singleRequest(
        const std::string& identity,
        const std::string& func,
        const RequestType0& requestItem0,
        const RequestType1& requestItem1,
        ResultType& resultItem,
        workerid_t workerid,
        timeout_t timeoutSec)
{
    if (workerid == localWorkerId_)
    {
        if (hasLocalWorker_ && localWorkerCaller_)
        {
            std::string error;
            return localWorkerCaller_->call(func, requestItem0, requestItem1, resultItem, error);
        }
        else
        {
            std::cerr << "#[Aggregator] Error: local worker was required but not enabled." <<endl;
            return false;
        }
    }

    std::string func_plus = identity+REQUEST_FUNC_DELIMETER+func;
    if (debug_)
        cout << "#[Aggregator] distribute request: " << func_plus << endl;

    const WorkerSessionPtr& workerSession = getWorkerSessionById(workerid);
    if (!workerSession.get())
    {
        cout << "#[Aggregator] Error: not found worker" << workerid <<endl;
        return false;
    }

    session_pool_t sessionPool;
    const ServerInfo& workerSrv = workerSession->getServerInfo();
    session_t session = sessionPool.get_session(workerSrv.host_, workerSrv.port_);
    session.set_timeout(timeoutSec);

    future_t future = session.call(func_plus, requestItem0, requestItem1, resultItem);
    resultItem = future.get<ResultType>();

    return true;
}

template <class ConcreteAggregator, class LocalWorkerCaller>
template<typename ResultType>
bool Aggregator<ConcreteAggregator, LocalWorkerCaller>::join(
        const std::string& func,
        std::vector<std::pair<workerid_t, future_t> >& futureList,
        ResultType& resultItem,
        ResultType* localResult)
{
    std::vector<std::pair<workerid_t, ResultType> > resultList;
    if (NULL != localResult)
    {
        resultList.push_back(std::make_pair(localWorkerId_, *localResult));
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

#endif /* IZENE_NET_AGGREGATOR_H_ */
