/**
 * @file core/driver/Request.cpp
 * @author Ian Yang
 * @date Created <2010-06-10 15:16:13>
 */
#include <util/driver/Request.h>

namespace izenelib {
namespace driver {

const std::string Request::kDefaultAction;

Request::Request()
: RestrictedObjectValue(), header_(0), calltype_(FromAPI)
{
    updateHeaderPtr();
}

Request::Request(const Request& other)
: RestrictedObjectValue(other),
  header_(0),
  controller_(other.controller_),
  action_(other.action_),
  aclTokens_(other.aclTokens_),
  calltype_(other.calltype_)
{
    updateHeaderPtr();
}

Request& Request::operator=(const Request& other)
{
    RestrictedObjectValue::operator=(other);
    updateHeaderPtr();

    controller_ = other.controller_;
    action_ = other.action_;
    aclTokens_ = other.aclTokens_;
    calltype_ = other.calltype_;

    return *this;
}

void Request::parseHeader()
{
    controller_ = asString(header()[Keys::controller]);
    action_ = asString(header()[Keys::action]);
    if (action_.empty())
    {
        action_ = "index";
    }
    aclTokens_ = asString(header()[Keys::acl_tokens]);
}

}} // namespace izenelib::driver
