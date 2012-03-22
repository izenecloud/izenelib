/**
 * @file WorkerController.h
 * @brief a base class for ControllerType in WorkerHandler
 * @author Zhongxia Li, Jun Jiang
 * @date 2012-03-20
 */

#ifndef IZENE_NET_AGGREGATOR_WORKER_CONTROLLER_H_
#define IZENE_NET_AGGREGATOR_WORKER_CONTROLLER_H_

#include "WorkerRouter.h"
#include "WorkerHandler.h"
#include <3rdparty/msgpack/rpc/request.h>

#include <string>
#include <memory>

namespace net{
namespace aggregator{

class WorkerController
{
public:
    virtual ~WorkerController() {}

    virtual bool preprocess(msgpack::rpc::request& req, std::string& error) { return true; }

    virtual bool addWorkerHandler(WorkerRouter& router) { return true; }
};

/**
 * in @c WorkerController subclass, use below macros in @c addWorkerHandler() to add @c WorkerHandler:
 */

#define ADD_WORKER_HANDLER_BEGIN(ConcreteWorkerController)  \
    typedef ConcreteWorkerController WorkerControllerType;

#define ADD_WORKER_HANDLER(Method)                                                  \
    {                                                                               \
        typedef net::aggregator::WorkerHandler<WorkerControllerType> handler_type;  \
        std::auto_ptr<handler_type> ptr(new handler_type(                           \
            *this, &WorkerControllerType::Method));                                 \
        if (!router.add(#Method, ptr.get()))                                        \
            return false;                                                           \
        ptr.release();                                                              \
    }

#define ADD_WORKER_HANDLER_END()    \
    return true;

/**
 * in @c WorkerController subclass, use below macros to define method of signature:
 * <code> void method(msgpack::rpc::request& req) </code>
 *
 * please note that, as @c Aggregator::distributeRequest() sets the "identity" value
 * as @c req.params[0], the method parameters would start from @c req.params()[1].
 */

/** "1_RET" means 1 param, and use the return value of @c Call as rpc result */
#define WORKER_CONTROLLER_METHOD_1_RET(Method, Call, ParamT, ResultT)   \
void Method(msgpack::rpc::request& req)                                 \
{                                                                       \
    msgpack::type::tuple<msgpack::object, ParamT> params;               \
    req.params().convert(&params);                                      \
    ParamT& param = params.get<1>();                                    \
    ResultT result = Call(param);                                       \
    req.result(result);                                                 \
}

/** "1_OUT" means 1 param, and use this param as rpc result */
#define WORKER_CONTROLLER_METHOD_1_OUT(Method, Call, ParamT)    \
void Method(msgpack::rpc::request& req)                         \
{                                                               \
    msgpack::type::tuple<msgpack::object, ParamT> params;       \
    req.params().convert(&params);                              \
    ParamT& param = params.get<1>();                            \
    Call(param);                                                \
    req.result(param);                                          \
}

/** "2_OUT" means 2 params, and use the last param as rpc result */
#define WORKER_CONTROLLER_METHOD_2_OUT(Method, Call, ParamT1, ParamT2)  \
void Method(msgpack::rpc::request& req)                                 \
{                                                                       \
    msgpack::type::tuple<msgpack::object, ParamT1, ParamT2> params;     \
    req.params().convert(&params);                                      \
    ParamT1& param1 = params.get<1>();                                  \
    ParamT2& param2 = params.get<2>();                                  \
    Call(param1, param2);                                               \
    req.result(param2);                                                 \
}

/** "3_OUT" means 3 params, and use the last param as rpc result */
#define WORKER_CONTROLLER_METHOD_3_OUT(Method, Call, ParamT1, ParamT2, ParamT3) \
void Method(msgpack::rpc::request& req)                                         \
{                                                                               \
    msgpack::type::tuple<msgpack::object, ParamT1, ParamT2, ParamT3> params;    \
    req.params().convert(&params);                                              \
    ParamT1& param1 = params.get<1>();                                          \
    ParamT2& param2 = params.get<2>();                                          \
    ParamT3& param3 = params.get<3>();                                          \
    Call(param1, param2, param3);                                               \
    req.result(param3);                                                         \
}

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_WORKER_CONTROLLER_H_
