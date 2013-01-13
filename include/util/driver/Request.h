#ifndef IZENELIB_DRIVER_REQUEST_H
#define IZENELIB_DRIVER_REQUEST_H
/**
 * @file izenelib/driver/Request.h
 * @author Ian Yang
 * @date Created <2010-06-10 14:37:01>
 */
#include "Value.h"
#include "RestrictedObjectValue.h"
#include "Keys.h"

#include <string>

namespace izenelib {
namespace driver {

class Request : public RestrictedObjectValue
{
public:
    static const std::string kDefaultAction;

    enum kCallType
    {
        FromAPI,
        FromDistribute,
        FromPrimaryWorker,
        FromLog,
    };

    Request();
    Request(const Request& other);
    Request& operator=(const Request& other);

    Value& header()
    {
        return *header_;
    }

    const Value& header() const
    {
        return *header_;
    }

    void swap(Request& other)
    {
        using std::swap;
        RestrictedObjectValue::swap(other);
        swap(controller_, other.controller_);
        swap(action_, other.action_);
        swap(aclTokens_, other.aclTokens_);
        swap(calltype_, other.calltype_);
        swap(addition_from_primary_, other.addition_from_primary_);
        updateHeaderPtr();
        other.updateHeaderPtr();
    }

    void assignTmp(Value& value)
    {
        RestrictedObjectValue::assignTmp(value);
        updateHeaderPtr();
        parseHeader();
    }

    /// @brief Parse header controller, action and acl tokens.
    void parseHeader();

    /// @brief Table field in header.
    ///
    /// It is not automatically updated. It is parsed when call assignTmp() or
    /// parseHeader().
    const Value::StringType& controller() const
    {
        return controller_;
    }

    /// @brief Action field in header.
    ///
    /// It is not automatically updated. It is parsed when call assignTmp() or
    /// parseHeader().
    const Value::StringType& action() const
    {
        return action_;
    }

    /// @brief comma separated user tokens
    /// It is not automatically updated. It is parsed when call assignTmp() or
    /// parseHeader().
    const Value::StringType& aclTokens() const
    {
        return aclTokens_;
    }

    void setCallType(kCallType calltype)
    {
        calltype_ = calltype;
    }
    kCallType callType() const
    {
        return calltype_;
    }
    void setPrimaryAddition(const std::string& data)
    {
        addition_from_primary_ = data;
    }
    const std::string& primaryAddition() const
    {
        return addition_from_primary_;
    }

private:
    void updateHeaderPtr()
    {
        header_ = &((*this)[Keys::header]);
    }

    /// @brief shortcuts to header object value
    Value* header_;

    Value::StringType controller_;
    Value::StringType action_;
    Value::StringType aclTokens_;
    kCallType calltype_;
    std::string addition_from_primary_;
};

inline void swap(Request& a, Request& b)
{
    a.swap(b);
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_REQUEST_H
