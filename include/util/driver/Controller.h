#ifndef IZENELIB_DRIVER_CONTROLLER_H
#define IZENELIB_DRIVER_CONTROLLER_H
/**
 * @file izenelib/driver/Controller.h
 * @author Ian Yang
 * @date Created <2010-06-18 15:50:13>
 */
#include "Request.h"
#include "Response.h"
#include "Poller.h"
#include <boost/function.hpp>

#include <boost/assert.hpp>

namespace izenelib {
namespace driver {

class Controller
{
public:
    typedef boost::function<void()> callback_t;
    Controller()
    : request_(0),
      response_(0)
    {}

    void setPoller(Poller poller)
    {
        poller_ = poller;
    }

    void set_callback(callback_t cb)
    {
        cb_ = cb;
    }

    void initializeRequestContext(Request& request,
                                  Response& response)
    {
        request_ = &request;
        response_ = &response;
    }

    /// @brief hook for steps should be performed before the handler.
    /// @return \c true if the request process should continue, \c false to
    ///         return immediately.
    bool preprocess()
    {
        return true;
    }

    void postprocess()
    {
        // empty
        if (cb_)
        {
            cb_();
        }
    }

protected:
    Poller& poller()
    {
        return poller_;
    }
    const Poller& poller() const
    {
        return poller_;
    }

    callback_t& callback()
    {
        return cb_;
    }
    const callback_t& callback() const
    {
        return cb_;
    }

    Request& request()
    {
        BOOST_ASSERT(request_);
        return *request_;
    }
    const Request& request() const
    {
        BOOST_ASSERT(request_);
        return *request_;
    }

    Response& response()
    {
        BOOST_ASSERT(response_);
        return *response_;
    }
    const Response& response() const
    {
        BOOST_ASSERT(response_);
        return *response_;
    }

private:
    Poller poller_;
    Request* request_;
    Response* response_;
    callback_t cb_;
};

// helper macros

#define IZENELIB_DRIVER_BEFORE_HOOK(func_name) \
if (!(func_name)) \
{ \
    return; \
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_CONTROLLER_H
