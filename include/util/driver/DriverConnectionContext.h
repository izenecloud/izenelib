#ifndef IZENELIB_DRIVER_CONNECTION_CONTEXT_H
#define IZENELIB_DRIVER_CONNECTION_CONTEXT_H
/**
 * @file util/driver/DriverConnectionContext.h
 * @author Ian Yang
 * @date Created <2010-06-10 16:54:52>
 */
#include <types.h> // uint32_t

#include "Value.h"
#include "Request.h"
#include "Response.h"

#include <util/ClockTimer.h>

#include <boost/asio/buffer.hpp>
#include <boost/array.hpp>

#include <arpa/inet.h> // ntohl, htonl

#include <utility> // swap

namespace izenelib {
namespace driver {


struct DriverConnectionContext
{
    // form header: sequence | size
    boost::array<uint32_t, 2> formHeader;
    std::string formPayload;

    driver::Request request;
    driver::Response response;

    izenelib::util::ClockTimer serverTimer;

    DriverConnectionContext();

    void swap(DriverConnectionContext& other)
    {
        using std::swap;
        swap(formHeader, other.formHeader);
        swap(formPayload, other.formPayload);
        request.swap(other.request);
        response.swap(other.response);
        swap(serverTimer, other.serverTimer);
    }

    typedef boost::asio::mutable_buffers_1 mutable_buffer;
    typedef boost::asio::const_buffers_1 const_buffer;
    typedef boost::array<const_buffer, 2> const_buffers;

    // Because sequence is copy from request to response directly, and the only
    // manually set sequence number is 0 (same in both endian), we don't need
    // endian conversion here.
    uint32_t sequence() const
    {
        return formHeader[0];
    }

    void setSequence(uint32_t sequence)
    {
        formHeader[0] = sequence;
    }

    uint32_t formPayloadSize() const
    {
        return ntohl(formHeader[1]);
    }

    /// @brief Size in formHeader[1] should be in net endian
    void setFormPayloadSize(uint32_t size)
    {
        formHeader[1] = htonl(size);
    }

    mutable_buffer formHeaderBuffer()
    {
        return boost::asio::buffer(formHeader);
    }
    mutable_buffer formPayloadBuffer()
    {
        char* start = 0;
        if (!formPayload.empty())
        {
            start = &(formPayload[0]);
        }
        return boost::asio::buffer(start, formPayload.size());
    }

    const_buffers buffer() const
    {
        const_buffers ret = {{
            boost::asio::buffer(formHeader),
            boost::asio::buffer(formPayload)
        }};

        return ret;
    }
};

inline void swap(DriverConnectionContext& a, DriverConnectionContext& b)
{
    a.swap(b);
}

} 
}

#endif // IZENELIB_DRIVER_CONNECTION_CONTEXT_H
