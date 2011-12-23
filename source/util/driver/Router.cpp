/**
 * @file driver/Router.cpp
 * @author Ian Yang
 * @date Created <2010-05-27 10:47:46>
 */
#include <util/driver/Router.h>
#include <util/driver/Keys.h>

namespace izenelib {
namespace driver {

RouterKey::RouterKey()
{}

RouterKey::RouterKey(const std::string& c, const std::string& a)
: controller(c),
  action(a)
{}

Router::Router()
: superHandler_(0)
{
}

Router::~Router()
{
    typedef map_type::const_iterator iterator;
    for (iterator it = table_.begin(); it != table_.end(); ++it)
    {
        delete it->second;
    }

    if (superHandler_)
        delete superHandler_;
}

Router::handler_ptr Router::find(const key_type& key) const
{
    typedef map_type::const_iterator iterator;

    // high priority
    if (superHandler_)
    {
        return superHandler_;
    }

    iterator findResult = table_.find(key);
    if (findResult != table_.end())
    {
        return findResult->second;
    }
    else
    {
        return kEmptyHandler_;
    }
}

const Router::handler_ptr Router::kEmptyHandler_ = 0;

}} // namespace izenelib::driver

