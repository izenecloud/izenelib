/**
 * @file WorkerHandler.h
 * @author Zhongxia Li
 * @date Jun 27, 2011
 * @brief 
 */
#ifndef IZENE_NET_AGGREGATOR_WORKER_HANDLER_H_
#define IZENE_NET_AGGREGATOR_WORKER_HANDLER_H_

namespace net{
namespace aggregator{

template <typename ConcreteWorker>
struct WorkerHandler
{
    typedef void (ConcreteWorker::*callback_type)(request_t&);

    callback_type callback_;

    WorkerHandler(callback_type callback)
    : callback_(callback)
    {}
};


/// Help macros for adding handlers to Worker

#define ADD_WORKER_HANDLER_LIST_BEGIN( ConcreteWorker )    \
    typedef ConcreteWorker ConcreteWorkerType;

#define ADD_WORKER_HANDLER(handlerFunction)                \
    {                                                      \
        WorkerHandler<ConcreteWorkerType>                  \
            handler(&ConcreteWorkerType::handlerFunction); \
        addHandler(#handlerFunction, handler);             \
    }

#define ADD_WORKER_HANDLER_LIST_END()                      \


/// Help macros for handling request, xxx

#define WORKER_HANDLE_1_1(request, ParamT1, FUNCTION, ResultT1)  \
{                                                                \
        msgpack::type::tuple<ParamT1, ResultT1> params;          \
        req.params().convert(&params);                           \
        ParamT1 param = params.get<0>();                         \
        ResultT1 result = params.get<1>();                       \
        FUNCTION(param, result);                                 \
        req.result(result);                                      \
}

#define WORKER_HANDLE_1_1_RE(request, ParamT1, FUNCTION, ResultT1)  \
{                                                                \
        msgpack::type::tuple<ParamT1, ResultT1> params;          \
        req.params().convert(&params);                           \
        ParamT1 param = params.get<0>();                         \
        ResultT1 result = params.get<1>();                       \
        result = FUNCTION(param);                                \
        req.result(result);                                      \
}

#define WORKER_HANDLE_2_1(request, ParamT1, ParamT2, FUNCTION, ResultT1)  \
{                                                                \
        msgpack::type::tuple<ParamT1, ParamT2, ResultT1> params; \
        req.params().convert(&params);                           \
        ParamT1 param1 = params.get<0>();                        \
        ParamT2 param2 = params.get<1>();                        \
        ResultT1 result = params.get<2>();                       \
        FUNCTION(param1, param2, result);                        \
        req.result(result);                                      \
}

#define WORKER_HANDLE_2_1_RE(request, ParamT1, ParamT2, FUNCTION, ResultT1)  \
{                                                                \
        msgpack::type::tuple<ParamT1, ParamT2, ResultT1> params; \
        req.params().convert(&params);                           \
        ParamT1 param1 = params.get<0>();                        \
        ParamT2 param2 = params.get<1>();                        \
        ResultT1 result = params.get<2>();                       \
        result = FUNCTION(param1, param2);                       \
        req.result(result);                                      \
}

#define WORKER_HANDLE_1_1_(request, ParamT1, ResultT1, SERVICE, FUNCTION)  \
{                                                                \
        msgpack::type::tuple<ParamT1, ResultT1> params;          \
        req.params().convert(&params);                           \
        ParamT1 param = params.get<0>();                         \
        ResultT1 result = params.get<1>();                       \
        SERVICE->FUNCTION(param, result);                        \
        req.result(result);                                      \
}

}} // end - namespace

#endif /* IZENE_NET_AGGREGATOR_WORKER_HANDLER_H_ */
