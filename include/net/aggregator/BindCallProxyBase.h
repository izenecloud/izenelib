/**
 * @file BindCallProxyBase.h
 * @brief when you need to bind a class to CallProxy, you could make it
 * inherits from BindCallProxyBase, and implement the interface
 * "bindCallProxy()", it has some macros to ease of the binding.
 *
 * Usage example:
 * <code>
 * class Example : public BindCallProxyBase<Example>
 * {
 * public:
 *
 * void get(std::string& out);
 * void size(const std::string& in, int& out);
 * void concat(const std::string& in1, int in2, std::string& out);
 *
 * virtual bool bindCallProxy(CallProxyType& proxy)
 * {
 *   BIND_CALL_PROXY_BEGIN(Example, proxy)
 *   BIND_CALL_PROXY_1(get, std::string)
 *   BIND_CALL_PROXY_2(size, std::string, int)
 *   BIND_CALL_PROXY_3(concat, std::string, int, std::string)
 *   BIND_CALL_PROXY_END()
 * }
 * ...
 * </code>
 *
 * @author Jun Jiang
 * @date 2012-04-03
 */

#ifndef IZENE_NET_AGGREGATOR_BIND_CALL_PROXY_BASE_H_
#define IZENE_NET_AGGREGATOR_BIND_CALL_PROXY_BASE_H_

#include "CallProxy.h"

namespace net{
namespace aggregator{

template <class ConcreteType>
class BindCallProxyBase
{
public:
    typedef CallProxy<ConcreteType> CallProxyType;

    virtual ~BindCallProxyBase() {}

    virtual bool bindCallProxy(CallProxyType& proxy) = 0;
};

#define BIND_CALL_PROXY_BEGIN(BindClassName, ProxyInstance) \
    typedef BindClassName BindClassType;                    \
    CallProxyType& _proxy = ProxyInstance;

#define BIND_CALL_PROXY_1(Method, Out)                          \
    {                                                           \
        if (!_proxy.bind<Out>(#Method, &BindClassType::Method)) \
            return false;                                       \
    }

#define BIND_CALL_PROXY_2(Method, In, Out)                          \
    {                                                               \
        if (!_proxy.bind<In, Out>(#Method, &BindClassType::Method)) \
            return false;                                           \
    }

#define BIND_CALL_PROXY_3(Method, In1, In2, Out)                            \
    {                                                                       \
        if (!_proxy.bind<In1, In2, Out>(#Method, &BindClassType::Method))   \
            return false;                                                   \
    }

#define BIND_CALL_PROXY_END()   \
    return true;

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_BIND_CALL_PROXY_BASE_H_
