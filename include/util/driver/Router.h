#ifndef IZENELIB_DRIVER_ROUTER_H
#define IZENELIB_DRIVER_ROUTER_H
/**
 * @file izenelib/driver/Router.h
 * @author Ian Yang
 * @date Created <2010-05-27 10:14:21>
 * @brief Routes request to controller
 */
#include "ActionHandler.h"

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <string>

namespace izenelib {
namespace driver {

class Request;
class Response;

struct RouterKey
{
    std::string controller;
    std::string action;

    RouterKey();
    RouterKey(const std::string& t, const std::string& a);
};
inline bool operator==(const RouterKey& a, const RouterKey& b);
inline std::size_t hash_value(const RouterKey& key);

class Router
{
public:
    typedef RouterKey key_type;
    typedef ActionHandlerBase* handler_ptr;
    typedef boost::unordered_map<key_type, handler_ptr> map_type;

    Router();

    ~Router();

    /// @brief Find registered action to handle the request
    /// @return Register action if found, empty function if not found. The
    ///         returned value can be convert to bool. The empty function is
    ///         converted to false, other are converted to true.
    handler_ptr find(const std::string& controller,
                           const std::string& action) const
    {
        key_type key(controller, action);
        return find(key);
    }

    /// @brief Find action using key
    handler_ptr find(const key_type& key) const;

    /// @brief Register a action to handle the specified type of request
    void map(
        const std::string& controller,
        const std::string& action,
        handler_ptr handler
    )
    {
        key_type key(controller, action);

        // insert first, then assign to ensure exception safe
        std::pair<map_type::iterator, bool> insertResult =
            table_.insert(std::make_pair(key, kEmptyHandler_));
        insertResult.first->second = handler;
    }

    void setSuperHandler(const handler_ptr superHandler)
    {
        superHandler_ = superHandler;
    }

private:
    map_type table_;
    handler_ptr superHandler_;
    static const handler_ptr kEmptyHandler_;
};

bool operator==(const RouterKey& a, const RouterKey& b)
{
    return a.controller == b.controller
        && a.action == b.action;
}

std::size_t hash_value(const RouterKey& key)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, key.controller);
    boost::hash_combine(seed, key.action);

    return seed;
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_ROUTER_H
