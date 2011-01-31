#ifndef IZENELIB_DRIVER_RESPONSE_H
#define IZENELIB_DRIVER_RESPONSE_H
/**
 * @file izenelib/driver/Response.h
 * @author Ian Yang
 * @date Created <2010-06-10 15:23:56>
 */
#include "Value.h"
#include "RestrictedObjectValue.h"
#include "Keys.h"

namespace izenelib {
namespace driver {

class Response : public RestrictedObjectValue
{
public:
    Response();
    Response(const Response& other);
    Response& operator=(const Response& other);

    Value& header()
    {
        return *header_;
    }

    const Value& header() const
    {
        return *header_;
    }

    void addError(const std::string& errorMessage)
    {
        setSuccess(false);
        if (!errorMessage.empty())
        {
            (*this)[Keys::errors]() = errorMessage;
        }
    }
    void addWarning(const std::string& warningMessage)
    {
        if (!warningMessage.empty())
        {
            (*this)[Keys::warnings]() = warningMessage;
        }
    }

    /// @brief Gets success bit.
    /// @return True for success, false for failure.
    bool success() const
    {
        return asBool(header()[Keys::success]);
    }

    /// @brief set success bit.
    /// @param success Set to true to indicate success. Set to false for
    ///                failure.
    void setSuccess(bool success = true)
    {
        header()[Keys::success] = success;
    }

    void swap(Response& other)
    {
        using std::swap;
        RestrictedObjectValue::swap(other);
        updateHeaderPtr();
        other.updateHeaderPtr();
    }

    void assignTmp(Value& value)
    {
        RestrictedObjectValue::assignTmp(value);
        updateHeaderPtr();
    }

private:
    void updateHeaderPtr()
    {
        header_ = &((*this)[Keys::header]);
    }

    /// @brief shortcuts to header object value
    Value* header_;
};

inline void swap(Response& a, Response& b)
{
    a.swap(b);
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_RESPONSE_H
