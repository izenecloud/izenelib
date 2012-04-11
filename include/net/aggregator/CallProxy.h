/**
 * @file CallProxy.h
 * @brief a proxy class to call member function by using its function name as
 * string parameter.
 *
 * Note:
 * 1. the member function to bind should have one of below signatures:
 *
 * void (Out&)
 * void (const In1&, Out&)
 * void (const In1&, const In2&, Out&)
 *
 * 2. owing to the implicit conversion, such signature is also allowed:
 *
 * void (In1, const Out&) // here the "const Out&" parameter works as an input parameter
 *
 * 3. in above signatures, you could also declare the return value as any type,
 * but this is useless, as the return value would be ignored in CallProxy::call(),
 * normally you should use the last parameter "Out&" as return value.
 *
 * Usage example:
 * <code>
 * class Foo
 * {
 * public:
 *     void bar(const std::string& in, int& out);
 * };
 *
 * Foo foo;
 * CallProxy<Foo> proxy(&foo);
 *
 * // step 1: bind the member function with its function name
 * bool bindResult = proxy.bind<std::string, int>("bar", &Foo::bar);
 *
 * // hint: you could make "Foo" class inherits from BindCallProxyBase,
 * // it has some macros to ease of the binding, then step 1 would be:
 * bool bindResult = foo.bindCallProxy(proxy);
 *
 * // step 2: call the member function
 * std::string in("abc");
 * int out = 0;
 * bool callResult = proxy.call("bar", in, out);
 * </code>
 *
 * @author Zhongxia Li, Jun Jiang
 * @date 2012-03-29
 */

#ifndef IZENE_NET_AGGREGATOR_CALL_PROXY_H_
#define IZENE_NET_AGGREGATOR_CALL_PROXY_H_

#include <string>
#include <map>
#include <boost/any.hpp>
#include <boost/function.hpp>

#include <glog/logging.h>

namespace net{
namespace aggregator{

template <class ClassT>
class CallProxy
{
public:
    CallProxy(ClassT* target) : target_(target) {}

    template <typename Out>
    bool bind(
        const std::string& name,
        const boost::function<void (ClassT*, Out&)>& func)
    {
        return bindImpl_(name, func);
    }

    template <typename In1, typename Out>
    bool bind(
        const std::string& name,
        const boost::function<void (ClassT*, const In1&, Out&)>& func)
    {
        return bindImpl_(name, func);
    }

    template <typename In1, typename In2, typename Out>
    bool bind(
        const std::string& name,
        const boost::function<void (ClassT*, const In1&, const In2&, Out&)>& func)
    {
        return bindImpl_(name, func);
    }

    template <typename Out>
    bool call(
        const std::string& name,
        Out& out);

    template <typename In1, typename Out>
    bool call(
        const std::string& name,
        const In1& in1, Out& out);

    template <typename In1, typename In2, typename Out>
    bool call(
        const std::string& name,
        const In1& in1, const In2& in2, Out& out);

private:
    template <typename FuncT>
    bool bindImpl_(const std::string& name, const FuncT& func);

    template <typename FuncT>
    const FuncT* findFunc_(const std::string& name) const;

private:
    ClassT* target_;

    typedef std::map<std::string, boost::any> FuncMap;
    FuncMap funcMap_;
};

template <class ClassT>
template <typename Out>
bool CallProxy<ClassT>::call(
    const std::string& name,
    Out& out)
{
    typedef boost::function<void (ClassT*, Out&)> FuncT;
    const FuncT* func = findFunc_<FuncT>(name);

    if (! func)
        return false;

    (*func)(target_, out);
    return true;
}

template <class ClassT>
template <typename In1, typename Out>
bool CallProxy<ClassT>::call(
    const std::string& name,
    const In1& in1, Out& out)
{
    typedef boost::function<void (ClassT*, const In1&, Out&)> FuncT;
    const FuncT* func = findFunc_<FuncT>(name);

    if (! func)
        return false;

    (*func)(target_, in1, out);
    return true;
}

template <class ClassT>
template <typename In1, typename In2, typename Out>
bool CallProxy<ClassT>::call(
    const std::string& name,
    const In1& in1, const In2& in2, Out& out)
{
    typedef boost::function<void (ClassT*, const In1&, const In2&, Out&)> FuncT;
    const FuncT* func = findFunc_<FuncT>(name);

    if (! func)
        return false;

    (*func)(target_, in1, in2, out);
    return true;
}

template <class ClassT>
template <typename FuncT>
bool CallProxy<ClassT>::bindImpl_(const std::string& name, const FuncT& func)
{
    FuncMap::value_type methodPair(name, func);
    if (funcMap_.insert(methodPair).second)
        return true;

    LOG(ERROR) << "failed to bind duplicated function name: " << name;
    return false;
}

template <class ClassT>
template <typename FuncT>
const FuncT* CallProxy<ClassT>::findFunc_(const std::string& name) const
{
    FuncMap::const_iterator it = funcMap_.find(name);
    if (it == funcMap_.end())
    {
        LOG(ERROR) << "no member function is bound to \"" << name << "\"";
        return NULL;
    }

    const FuncT* func = boost::any_cast<FuncT>(&it->second);
    if (! func)
    {
        LOG(ERROR) << "the signature is not matched with member function \"" << name << "\"";
        return NULL;
    }

    return func;
}

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_CALL_PROXY_H_
