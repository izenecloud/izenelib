/**
 * @file WorkerCaller.h
 * @author Zhongxia Li
 * @date Nov 29, 2011
 * @brief 
 */
#ifndef IZENE_NET_AGGREGATOR_WORKER_CALLER_H_
#define IZENE_NET_AGGREGATOR_WORKER_CALLER_H_

#include <boost/unordered_map.hpp>

namespace net{
namespace aggregator{


/// WorkerCaller performs directly call (in procedure) to local  worker,
/// pass a suitable class type as worker.
template <typename WorkerT>
class WorkerCaller
{
public:
    typedef void(WorkerT::*func_t)(void);

public:
    WorkerCaller(WorkerT* worker)
    : worker_(worker)
    {
        assert(worker_);
    }

    void addMethod(const std::string& func_name, func_t func)
    {
        // func_name should be unique
        funcMap_.insert(std::make_pair(func_name, func));
    }

    template <typename RequestType, typename ResultType>
    bool call(
            const std::string& func_name,
            const RequestType& request,
            ResultType& result,
            std::string& error)
    {
        typedef bool(WorkerT::*real_func_t)(const RequestType& , ResultType&);

        cout << "WorkerCaller::call "<<func_name<<endl;

        if (!worker_)
        {
            cout << "WorkerCaller: empty invoker!"<<endl;
            return false;
        }

        if (funcMap_.find(func_name) != funcMap_.end())
        {
            real_func_t func = (real_func_t) funcMap_[func_name];
            return (worker_->*func)(request, result);
        }
        else
        {
            cout << "WorkerCaller: not found method "<<func_name<<endl;
            return false;
        }
    }

    template <typename RequestType0, typename RequestType1, typename ResultType>
    bool call(
            const std::string& func_name,
            RequestType0& request0,
            RequestType1& request1,
            ResultType& result,
            std::string& error)
    {
        typedef bool(WorkerT::*real_func_t)(RequestType0&, RequestType1&, ResultType&);

        cout << "WorkerCaller::call "<<func_name<<endl;

        if (!worker_)
        {
            cout << "WorkerCaller: empty invoker!"<<endl;
            return false;
        }

        if (funcMap_.find(func_name) != funcMap_.end())
        {
            real_func_t func = (real_func_t) funcMap_[func_name];
            return (worker_->*func)(request0, request1, result);
        }
        else
        {
            cout << "WorkerCaller: not found method "<<func_name<<endl;
            return false;
        }
    }

private:
    std::map<std::string, func_t> funcMap_;
    WorkerT* worker_;
};

class EmptyWorkerT {};
typedef WorkerCaller<EmptyWorkerT> MockWorkerCaller;

#define ADD_FUNC_TO_WORKER_CALLER(WorkerCallerType, WorkerCallerObj, WorkerType, Method) \
{                                                                                       \
    WorkerCallerType::func_t func = (WorkerCallerType::func_t) &WorkerType::Method;     \
    WorkerCallerObj->addMethod(#Method,  func);                                         \
}

}} // namespaces

#endif /* IZENE_NET_AGGREGATOR_WORKER_CALLER_H_ */
