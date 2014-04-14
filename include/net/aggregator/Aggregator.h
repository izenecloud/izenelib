/**
 * @file Aggregator.h
 * @author Zhongxia Li
 * @date Jun 23, 2011
 * @brief Aggregator distribute requests to multiple Workers and gather results.
 */
#ifndef IZENE_NET_AGGREGATOR_H_
#define IZENE_NET_AGGREGATOR_H_

#include "Typedef.h"
#include "AggregatorConfig.h"
#include "AggegatorParam.h"
#include "WorkerResults.h"
#include <3rdparty/msgpack/rpc/session_pool.h>

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/unordered_set.hpp>
#include <iostream>

namespace net{
namespace aggregator{

/**
 * @brief Aggregator can distribute requests to Workers & gather results returned by Workers.                           <br>
 * To send requests, we can use @c distributeRequest() to broadcast, and use @c singleRequest() to a specified Worker.  <br>
 *
 * To merge results, we can implement a Merger class, an example could be found in "SearchMerger.h",                    <br>
 * the prototype of Merger's method is:                                                                                 <br>
 *      void funcName(const net::aggregator::WorkerResults<In>& workerResults, Out& mergeResult);                       <br>
 *
 * For sending requests, rpc (remote procedure call) will be performed to call remote Workers,                          <br>
 * to support directly call (in-process) to local Worker, we can pass a LocalWorkerProxy in Aggregator constructor.     <br>
 *
 * Construct example: (the details could be found in "t_aggregator.cpp")
 * <code>
 * typedef CallProxy<SearchMerger> SearchMergerProxy;
 * typedef CallProxy<SearchWorker> SearchWorkerProxy;
 * typedef Aggregator<SearchMergerProxy, SearchWorkerProxy> SearchAggregator;
 *
 * SearchMerger* searchMerger = new SearchMerger;
 * SearchMergerProxy* mergerProxy = new SearchMergerProxy(searchMerger);
 * searchMerger->bindCallProxy(*mergerProxy));
 *
 * SearchWorker* localWorker = new SearchWorker;
 * SearchWorkerProxy* workerProxy = new SearchWorkerProxy(localWorker);
 * localWorker->bindCallProxy(*workerProxy);
 *
 * SearchAggregator* aggregator = new SearchAggregator(mergerProxy, workerProxy);
 * </code>
 */

class AggregatorBase
{
public:
    AggregatorBase(const std::string& service, const std::string& collection)
        : service_(service), collection_(collection) {}

    virtual ~AggregatorBase(){}

    virtual void setAggregatorConfig(const AggregatorConfig& aggregatorConfig, bool force_update = false) = 0;
    virtual void setBusyAggregatorList(const std::vector<ServerInfo>& server_list) = 0;

    const std::string& collection() const { return collection_; }

    const std::string& service() const {return service_;}

    virtual bool isNeedDistribute() const = 0;

protected:
    std::string service_;
    std::string collection_;
};

template <class MergerProxy, class LocalWorkerProxy>
class Aggregator : public AggregatorBase
{
    typedef std::pair<workerid_t, future_t> worker_future_pair_t;
    typedef std::vector<worker_future_pair_t> worker_future_list_t;
    typedef std::vector<WorkerSessionPtr>::const_iterator worker_const_iterator_t;

public:
    /**
     * Constructor.
     * @param mergerProxy merger proxy
     * @param localWorkerProxy local worker proxy, it could be NULL if there is no local worker
     * @param collection collection name
     * @note Aggregator would take the ownership for @p mergerProxy and @p localWorkerProxy
     */
    Aggregator(
        MergerProxy* mergerProxy,
        LocalWorkerProxy* localWorkerProxy,
        const std::string& service,
        const std::string& collection);

    virtual void setAggregatorConfig(const AggregatorConfig& aggregatorConfig, bool force_update = false);

    virtual void setBusyAggregatorList(const std::vector<ServerInfo>& server_list)
    {
        busy_workers_.clear();
        for (std::size_t i = 0; i < server_list.size(); ++i)
        {
            busy_workers_.insert(server_list[i].getStrDescription());
        }
    }

    const std::vector<workerid_t>& getWorkerIdList() const { return workeridList_; }

    size_t getWorkerNum() const { return workeridList_.size(); }

    void setDebug(bool debug) { debug_ = debug; }

    bool isLocalWorker(workerid_t wid)
    {
        return hasLocalWorker_ && wid == localWorkerId_;
    }

    bool isReadOnly()
    {
        return is_readonly_;
    }
    bool hasLocalWorker()
    {
        return hasLocalWorker_;
    }
    workerid_t getLocalWorker()
    {
        return localWorkerId_;
    }
    /**
     * Send request to all workers, their results will be \b aggregated to @p out.
     *
     * @param identity identity info, pass empty string if not necessary.
     * @param func remote function name
     * @param[in,out] out output parameter
     */
    template <typename Out>
    bool distributeRequest(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        Out& out);

    template <typename In, typename Out>
    bool distributeRequest(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        const In& in,
        Out& out);

    template <typename In1, typename In2, typename Out>
    bool distributeRequest(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        const In1& in1,
        const In2& in2,
        Out& out);

    /**
     * Send a group of requests to corresponding workers, each request may carry different request data,
     * their results will be \b aggregated to @p out.
     *
     * @param identity, identity info, pass empty string if not processed.
     * @param func, remote function name
     * @param[in] requestGroup request group
     * @param[in,out] out output parameter
     */
    template <typename In, typename Out>
    bool distributeRequest(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        RequestGroup<In, Out>& requestGroup,
        Out& out);

    /**
     * Send request to a specified worker, without aggregating action.
     */
    template <typename Out>
    bool singleRequest(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        Out& out,
        workerid_t workerid);

    template <typename In, typename Out>
    bool singleRequest(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        const In& in,
        Out& out,
        workerid_t workerid);

    template <typename In1, typename In2, typename Out>
    bool singleRequest(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        const In1& in1,
        const In2& in2,
        Out& out,
        workerid_t workerid);

    /**
     * Send request to all workers except local self, their results will be \b aggregated to @p out.
     *
     * @param identity identity info, pass empty string if not necessary.
     * @param func remote function name
     * @param[in,out] out output parameter
     */
    template <typename Out>
    bool distributeRequestWithoutLocal(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        Out& out);

    template <typename In, typename Out>
    bool distributeRequestWithoutLocal(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        const In& in,
        Out& out);

    template <typename In1, typename In2, typename Out>
    bool distributeRequestWithoutLocal(
        const std::string& identity,
        uint32_t ro_index,
        const std::string& func,
        const In1& in1,
        const In2& in2,
        Out& out);

    bool isNeedDistribute() const
    {
        return !(hasLocalWorker_ && workerSessionList_.empty());
    }

protected:
    session_t getMsgPackSession_(const WorkerSessionPtr& workerSessionPtr);

    WorkerSessionPtr getWorkerSessionById_(const workerid_t workerid, uint32_t ro_index) const;

    void printWorkerError_(workerid_t workerid, uint32_t ro_index, const char* error) const;

    template <typename AggregatorParamT>
    bool distributeRequestImpl_(
        AggregatorParamT& param, uint32_t ro_index, bool include_self);

    template <typename AggregatorParamT>
    bool singleRequestImpl_(
        AggregatorParamT& param,
        uint32_t ro_index,
        workerid_t workerid);

    template <typename Out>
    bool mergeResults_(
        uint32_t ro_index,
        const std::string& func,
        worker_future_list_t& futureList,
        WorkerResults<Out>& workerResults,
        Out& totalOut);

protected:
    bool debug_;

    bool is_readonly_;
    bool hasLocalWorker_;
    workerid_t localWorkerId_; // available when hasLocalWorker is true

    boost::scoped_ptr<MergerProxy> mergerProxy_;
    boost::scoped_ptr<LocalWorkerProxy> localWorkerProxy_;

    std::map<workerid_t, std::vector<WorkerSessionPtr> > ro_workers_;
    std::vector<workerid_t> workeridList_;
    std::vector<WorkerSessionPtr> workerSessionList_;

    unsigned int timeout_; // second
    boost::scoped_ptr<session_pool_t> sessionPool_;

    typedef boost::shared_mutex MutexType;
    typedef boost::shared_lock<MutexType> ScopedReadLock;
    typedef boost::unique_lock<MutexType> ScopedWriteLock;
    MutexType mutex_;

    std::set<std::string> busy_workers_; 
};

template <class MergerProxy, class LocalWorkerProxy>
Aggregator<MergerProxy, LocalWorkerProxy>::Aggregator(
    MergerProxy* mergerProxy,
    LocalWorkerProxy* localWorkerProxy,
    const std::string& service,
    const std::string& collection)
: AggregatorBase(service, collection)
, debug_(false)
, is_readonly_(false)
, hasLocalWorker_(false)
, localWorkerId_(0)
, mergerProxy_(mergerProxy)
, localWorkerProxy_(localWorkerProxy)
, timeout_(20)
{
}

template <class MergerProxy, class LocalWorkerProxy>
void Aggregator<MergerProxy, LocalWorkerProxy>::setAggregatorConfig(const AggregatorConfig& aggregatorConfig, bool force_update)
{
    ScopedWriteLock lock(mutex_);

    is_readonly_ = aggregatorConfig.isReadOnly();
    if (!is_readonly_ && !force_update && timeout_ == aggregatorConfig.getTimeout())
    {
        const std::vector<WorkerServerInfo>& workerSrvList = aggregatorConfig.getWorkerList();
        bool need_reset = false;
        size_t compared_num = 0;
        for (size_t i = 0; i < workerSrvList.size(); i ++)
        {
            const WorkerServerInfo& workerSrv = workerSrvList[i];

            // only 1 local is allowed
            if (workerSrv.isLocal_ && localWorkerProxy_)
            {
                if (!hasLocalWorker_ || localWorkerId_ != workerSrv.workerid_)
                {
                    need_reset = true;
                    break;
                }
                continue;
            }
            if (compared_num >= workeridList_.size() || compared_num >= workerSessionList_.size())
            {
                need_reset = true;
                break;
            }
            if (workeridList_[compared_num] != workerSrv.workerid_)
            {
                need_reset = true;
                break;
            }
            if (workerSessionList_[compared_num]->getWorkerId() != workerSrv.workerid_)
            {
                need_reset = true;
                break;
            }
            if (workerSessionList_[compared_num]->getServerInfo().host_ != workerSrv.host_)
            {
                need_reset = true;
                break;
            }
            if (workerSessionList_[compared_num]->getServerInfo().port_ != workerSrv.port_)
            {
                need_reset = true;
                break;
            }
            ++compared_num;
        }
        if (!need_reset)
        {
            std::cout << "#[Aggregator] no need reset worker connection : " << aggregatorConfig.toString() << std::endl;
            return;
        }
    }

    workeridList_.clear();
    workerSessionList_.clear();
    ro_workers_.clear();
    hasLocalWorker_ = false;
    localWorkerId_ = 0;
    timeout_ = aggregatorConfig.getTimeout();
    sessionPool_.reset(new session_pool_t);
    sessionPool_->start(aggregatorConfig.getSessionPoolThreadNum());

    if (is_readonly_)
    {
        // read only worker list is used for the master node without local worker.
        // So that the master can balance the load over all remote workers.
        // If the node work as local worker, it should not set read only worker list.
        const std::vector<WorkerServerInfo>& workerSrvList = aggregatorConfig.getReadOnlyWorkerList();
        for (size_t i = 0; i < workerSrvList.size(); i ++)
        {
            const WorkerServerInfo& workerSrv = workerSrvList[i];
            // create worker session (client)
            WorkerSessionPtr workerSession(
                new WorkerSession(workerSrv.host_, workerSrv.port_, workerSrv.workerid_));
            ro_workers_[workerSrv.workerid_].push_back(workerSession);
        }
        return;
    }
    const std::vector<WorkerServerInfo>& workerSrvList = aggregatorConfig.getWorkerList();
    for (size_t i = 0; i < workerSrvList.size(); i ++)
    {
        const WorkerServerInfo& workerSrv = workerSrvList[i];

        // only 1 local is allowed
        if (workerSrv.isLocal_ && localWorkerProxy_ && !hasLocalWorker_)
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

template <class MergerProxy, class LocalWorkerProxy>
template <typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequest(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    Out& out)
{
    AggregatorParam<LocalWorkerProxy, Out> param(
        localWorkerProxy_.get(), identity, func, out);

    return distributeRequestImpl_(param, ro_index, true);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename In, typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequest(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    const In& in,
    Out& out)
{
    AggregatorParam<LocalWorkerProxy, Out, In> param(
        localWorkerProxy_.get(), identity, func, out, in);

    return distributeRequestImpl_(param, ro_index, true);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename In1, typename In2, typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequest(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    const In1& in1,
    const In2& in2,
    Out& out)
{
    AggregatorParam<LocalWorkerProxy, Out, In1, In2> param(
        localWorkerProxy_.get(), identity, func, out, in1, in2);

    return distributeRequestImpl_(param, ro_index, true);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename In, typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequest(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    RequestGroup<In, Out>& requestGroup,
    Out& out)
{
    ScopedReadLock lock(mutex_);

    if (debug_)
        std::cout << "#[Aggregator] distribute request: " << func << "[" << identity << "]" << std::endl;

    In* pLocalIn = NULL;
    Out* pLocalOut = NULL;
    worker_future_list_t futureList;

    for (std::size_t i = 0; i < requestGroup.workeridList_.size(); ++i)
    {
        workerid_t workerid = requestGroup.workeridList_[i];
        In* pIn = requestGroup.requestList_[i];
        Out* pOut = requestGroup.resultList_[i];

        if (pOut == NULL)
            pOut = &out;

        if (workerid == localWorkerId_)
        {
            pLocalIn = pIn;
            pLocalOut = pOut;
            continue;
        }

        WorkerSessionPtr workerSession = getWorkerSessionById_(workerid, ro_index);
        if (!workerSession)
        {
            std::cout << "#[Aggregator] Error: not found worker" << workerid << std::endl;
            continue;
        }

        session_t session = getMsgPackSession_(workerSession);
        future_t future = session.call(func, identity, *pIn, *pOut);
        futureList.push_back(worker_future_pair_t(workerid, future));
    }

    WorkerResults<Out> workerResults;
    if (pLocalIn && hasLocalWorker_)
    {
        if (debug_)
            std::cout << "#[Aggregator] call local worker" << localWorkerId_ << endl;

        if (localWorkerProxy_->call(func, *pLocalIn, *pLocalOut))
        {
            workerResults.add(localWorkerId_, *pLocalOut);
        }
    }

    return mergeResults_(ro_index, func, futureList, workerResults, out);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequestWithoutLocal(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    Out& out)
{
    AggregatorParam<LocalWorkerProxy, Out> param(
        localWorkerProxy_.get(), identity, func, out);

    return distributeRequestImpl_(param, ro_index, false);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename In, typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequestWithoutLocal(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    const In& in,
    Out& out)
{
    AggregatorParam<LocalWorkerProxy, Out, In> param(
        localWorkerProxy_.get(), identity, func, out, in);

    return distributeRequestImpl_(param, ro_index, false);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename In1, typename In2, typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequestWithoutLocal(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    const In1& in1,
    const In2& in2,
    Out& out)
{
    AggregatorParam<LocalWorkerProxy, Out, In1, In2> param(
        localWorkerProxy_.get(), identity, func, out, in1, in2);

    return distributeRequestImpl_(param, ro_index, false);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::singleRequest(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    Out& out,
    workerid_t workerid)
{
    AggregatorParam<LocalWorkerProxy, Out> param(
        localWorkerProxy_.get(), identity, func, out);

    return singleRequestImpl_(param, ro_index, workerid);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename In, typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::singleRequest(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    const In& in,
    Out& out,
    workerid_t workerid)
{
    AggregatorParam<LocalWorkerProxy, Out, In> param(
        localWorkerProxy_.get(), identity, func, out, in);

    return singleRequestImpl_(param, ro_index, workerid);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename In1, typename In2, typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::singleRequest(
    const std::string& identity,
    uint32_t ro_index,
    const std::string& func,
    const In1& in1,
    const In2& in2,
    Out& out,
    workerid_t workerid)
{
    AggregatorParam<LocalWorkerProxy, Out, In1, In2> param(
        localWorkerProxy_.get(), identity, func, out, in1, in2);

    return singleRequestImpl_(param, ro_index, workerid);
}

template <class MergerProxy, class LocalWorkerProxy>
session_t Aggregator<MergerProxy, LocalWorkerProxy>::getMsgPackSession_(const WorkerSessionPtr& workerSessionPtr)
{
    const ServerInfo& serverInfo = workerSessionPtr->getServerInfo();
    session_t session = sessionPool_->get_session(serverInfo.host_, serverInfo.port_, timeout_);

    if (debug_)
    {
        std::cout << "#[Aggregator] send to worker " << workerSessionPtr->getWorkerId()
                  << " [" << serverInfo.host_ << ":" << serverInfo.port_ << "]" << std::endl;
    }

    return session;
}

template <class MergerProxy, class LocalWorkerProxy>
WorkerSessionPtr Aggregator<MergerProxy, LocalWorkerProxy>::getWorkerSessionById_(const workerid_t workerid, uint32_t ro_index) const
{
    if (is_readonly_)
    {
        std::map<workerid_t, std::vector<WorkerSessionPtr> >::const_iterator cit = ro_workers_.find(workerid);
        if (cit != ro_workers_.end())
        {
            if (!cit->second.empty())
            {
                std::size_t inc = 0;
                WorkerSessionPtr selected_worker;
                while(inc < cit->second.size()) 
                {
                    selected_worker = cit->second[(ro_index + inc) % cit->second.size()];
                    ++inc;
                    if (busy_workers_.find(selected_worker->getServerInfo().getStrDescription()) == busy_workers_.end())
                    {
                        break;
                    }
                }
                return selected_worker;
            }
        }
        return WorkerSessionPtr();
    }
    for (size_t i = 0; i < workerSessionList_.size(); i++)
    {
        if (workerid == workerSessionList_[i]->getWorkerId())
            return workerSessionList_[i];
    }

    return WorkerSessionPtr();
}

template <class MergerProxy, class LocalWorkerProxy>
void Aggregator<MergerProxy, LocalWorkerProxy>::printWorkerError_(workerid_t workerid, uint32_t ro_index, const char* error) const
{
    std::cerr << "#[Aggregator] Got error (" << error << ") from worker" << workerid;

    WorkerSessionPtr workerSession = getWorkerSessionById_(workerid, ro_index);
    if (workerSession)
    {
        const ServerInfo& workerSrv = workerSession->getServerInfo();
        std::cerr << " [" << workerSrv.host_ << ":" << workerSrv.port_ << "] " << std::endl;
    }
    else
    {
        std::cerr <<", not found worker!" << std::endl;
    }
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename AggregatorParamT>
bool Aggregator<MergerProxy, LocalWorkerProxy>::distributeRequestImpl_(
    AggregatorParamT& param, uint32_t ro_index, bool include_self)
{
    ScopedReadLock lock(mutex_);

    if (debug_)
    {
        std::cout << "#[Aggregator] distribute request: " << param.funcName_
                  << "[" << param.identity_ << "]" << std::endl;
    }

    worker_future_list_t futureList;
    if (is_readonly_)
    {
        for (std::map<workerid_t, std::vector<WorkerSessionPtr> >::const_iterator cit = ro_workers_.begin();
            cit != ro_workers_.end(); ++cit)
        {
            if (cit->second.empty())
            {
                std::cout << __FUNCTION__ << ":" << __LINE__ << ", one of worker id missing : " << cit->first;
                continue;
            }

            std::size_t inc = 0;
            WorkerSessionPtr selected_worker;
            while(inc < cit->second.size()) 
            {
                selected_worker = cit->second[(ro_index + inc) % cit->second.size()];
                ++inc;
                if (busy_workers_.find(selected_worker->getServerInfo().getStrDescription()) == busy_workers_.end())
                {
                    break;
                }
            }

            session_t session = getMsgPackSession_(selected_worker);
            future_t future = param.getFuture(session);
            futureList.push_back(worker_future_pair_t(cit->first, future));
        }
    }
    else
    {
        if (workerSessionList_.empty())
            std::cout << "#[Aggregator] no remote worker(s)." << std::endl;

        for (worker_const_iterator_t workerIt = workerSessionList_.begin();
            workerIt != workerSessionList_.end(); ++workerIt)
        {
            workerid_t workerid = (*workerIt)->getWorkerId();
            session_t session = getMsgPackSession_(*workerIt);
            future_t future = param.getFuture(session);
            futureList.push_back(worker_future_pair_t(workerid, future));
        }
    }

    typedef typename AggregatorParamT::out_type Out;
    WorkerResults<Out> workerResults;
    if (hasLocalWorker_ && include_self)
    {
        if (debug_)
            std::cout << "#[Aggregator] call local worker" << localWorkerId_ << endl;

        Out localOut(param.out_);
        if (param.getLocalResult(localOut))
        {
            workerResults.add(localWorkerId_, localOut);
        }
    }

    return mergeResults_(ro_index, param.funcName_, futureList, workerResults, param.out_);
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename AggregatorParamT>
bool Aggregator<MergerProxy, LocalWorkerProxy>::singleRequestImpl_(
    AggregatorParamT& param,
    uint32_t ro_index,
    workerid_t workerid)
{
    ScopedReadLock lock(mutex_);

    if (debug_)
    {
        std::cout << "#[Aggregator] distribute request: " << param.funcName_
                  << "[" << param.identity_ << "]" << std::endl;
    }

    if (workerid == localWorkerId_)
    {
        if (hasLocalWorker_)
            return param.getLocalResult(param.out_);

        std::cerr << "#[Aggregator] Error: local worker was required but not enabled." << std::endl;
        return false;
    }

    WorkerSessionPtr workerSession = getWorkerSessionById_(workerid, ro_index);
    if (!workerSession)
    {
        std::cout << "#[Aggregator] Error: not found worker" << workerid << std::endl;
        return false;
    }

    session_t session = getMsgPackSession_(workerSession);
    future_t future = param.getFuture(session);

    try
    {
        param.out_ = future.get<typename AggregatorParamT::out_type>();
    }
    catch (std::exception& e)
    {
        printWorkerError_(workerid, ro_index, std::string(std::string(e.what()) + " in " + param.funcName_).c_str());
        return false;
    }

    return true;
}

template <class MergerProxy, class LocalWorkerProxy>
template <typename Out>
bool Aggregator<MergerProxy, LocalWorkerProxy>::mergeResults_(
    uint32_t ro_index,
    const std::string& func,
    worker_future_list_t& futureList,
    WorkerResults<Out>& workerResults,
    Out& totalOut)
{
    for (worker_future_list_t::iterator it = futureList.begin();
        it != futureList.end(); ++it)
    {
        workerid_t workerid = it->first;
        future_t& future = it->second;

        try
        {
            const Out& futureOut = future.get<Out>();
            workerResults.add(workerid, futureOut);
        }
        catch (std::exception& e)
        {
            printWorkerError_(workerid, ro_index, std::string(std::string(e.what()) + " in " + func).c_str());
        }
    }

    if (workerResults.empty())
        return false;

    return mergerProxy_->call(func, workerResults, totalOut);
}

}} // end - namespace

#endif /* IZENE_NET_AGGREGATOR_H_ */
