/**
 * @file core/driver/Response.cpp
 * @author Ian Yang
 * @date Created <2010-06-10 15:52:07>
 */
#include <util/driver/Response.h>

namespace izenelib {
namespace driver {

Response::Response()
: RestrictedObjectValue(), header_(0)
{
    updateHeaderPtr();
}

Response::Response(const Response& other)
: RestrictedObjectValue(other), header_(0)
{
    updateHeaderPtr();
}

Response& Response::operator=(const Response& other)
{
    RestrictedObjectValue::operator=(other);
    updateHeaderPtr();

    return *this;
}

}} // namespace izenelib::driver
