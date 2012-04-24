/**
 * @file AggregatorParam.h
 * @brief for each Aggregator methods such as distributeRequest() and singleRequest(),
 * the paramters number are different, AggregatorParam is the interface to encapsulate
 * these parameters.
 * @author Jun Jiang
 * @date 2012-04-01
 */

#ifndef IZENE_NET_AGGREGATOR_PARAM_H_
#define IZENE_NET_AGGREGATOR_PARAM_H_

#include "Typedef.h"
#include <string>

namespace net{
namespace aggregator{

template <class LocalWorkerProxy, typename Out, typename In1 = void, typename In2 = void, typename In3 = void>
struct AggregatorParam;

template <class LocalWorkerProxy, typename Out>
struct AggregatorParam<LocalWorkerProxy, Out>
{
    typedef Out out_type;

    LocalWorkerProxy* localWorkerProxy_;
    const std::string& identity_;
    const std::string& funcName_;

    Out& out_;

    AggregatorParam(
        LocalWorkerProxy* localWorkerProxy,
        const std::string& identity,
        const std::string& funcName,
        Out& out)
    : localWorkerProxy_(localWorkerProxy)
    , identity_(identity)
    , funcName_(funcName)
    , out_(out)
    {
    }

    bool getLocalResult(Out& localResult)
    {
        return localWorkerProxy_->call(funcName_, localResult);
    }

    future_t getFuture(session_t& session)
    {
        return session.call(funcName_, identity_, out_);
    }
};

template <class LocalWorkerProxy, typename Out, typename In1>
struct AggregatorParam<LocalWorkerProxy, Out, In1>
{
    typedef Out out_type;

    LocalWorkerProxy* localWorkerProxy_;
    const std::string& identity_;
    const std::string& funcName_;

    Out& out_;
    const In1& in1_;

    AggregatorParam(
        LocalWorkerProxy* localWorkerProxy,
        const std::string& identity,
        const std::string& funcName,
        Out& out,
        const In1& in1)
    : localWorkerProxy_(localWorkerProxy)
    , identity_(identity)
    , funcName_(funcName)
    , out_(out)
    , in1_(in1)
    {
    }

    bool getLocalResult(Out& localResult)
    {
        return localWorkerProxy_->call(funcName_, in1_, localResult);
    }

    future_t getFuture(session_t& session)
    {
        return session.call(funcName_, identity_, in1_, out_);
    }

};

template <class LocalWorkerProxy, typename Out, typename In1, typename In2>
struct AggregatorParam<LocalWorkerProxy, Out, In1, In2>
{
    typedef Out out_type;

    LocalWorkerProxy* localWorkerProxy_;
    const std::string& identity_;
    const std::string& funcName_;

    Out& out_;
    const In1& in1_;
    const In2& in2_;

    AggregatorParam(
        LocalWorkerProxy* localWorkerProxy,
        const std::string& identity,
        const std::string& funcName,
        Out& out,
        const In1& in1,
        const In2& in2)
    : localWorkerProxy_(localWorkerProxy)
    , identity_(identity)
    , funcName_(funcName)
    , out_(out)
    , in1_(in1)
    , in2_(in2)
    {
    }

    bool getLocalResult(Out& localResult)
    {
        return localWorkerProxy_->call(funcName_, in1_, in2_, localResult);
    }

    future_t getFuture(session_t& session)
    {
        return session.call(funcName_, identity_, in1_, in2_, out_);
    }

};

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_PARAM_H_
