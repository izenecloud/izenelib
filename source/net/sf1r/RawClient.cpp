/* 
 * File:   RawClient.cpp
 * Author: Paolo D'Apice
 * 
 * Created on December 31, 2011, 2:46 PM
 */

#include "net/sf1r/Errors.hpp"
#include "RawClient.hpp"
#include <boost/array.hpp>
#include <glog/logging.h>
#include <sys/time.h>
#include <boost/bind.hpp>


NS_IZENELIB_SF1R_BEGIN

namespace bs = boost::system;

using ba::ip::tcp;
using std::string;

#define CONNECT_TIMEOUT 5
#define RW_TIMEOUT 20

namespace {

/**
 * Size of unsigned integer (32 bits).
 */
const size_t UINT_SIZE = sizeof(uint32_t);

/**
 * Header size, i.e. two unsigned values for:
 * - sequence number
 * - message length
 */
const size_t HEADER_SIZE = 2 * UINT_SIZE;

}


uint32_t 
RawClient::idSequence = 0;


RawClient::RawClient(ba::io_service& service, 
                     const std::string& host, const std::string& port,
                     const size_t timeout, const string& zkpath) 
        : socket(service), deadline_(service), io_service_(service),
        status(Idle), is_aborted_(false), path(zkpath), id(++idSequence) {
    try {
        deadline_.expires_at(boost::posix_time::pos_infin);
        check_deadline();

        DLOG(INFO) << "configuring socket ...";
        // TODO: windows?
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        setsockopt(socket.native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv));
        setsockopt(socket.native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv));
        
        DLOG(INFO) << "connecting to [" << host << ":" << port << "] (" << id << ") ...";
        connect_with_timeout(host, port);
        
        DLOG(INFO) << "connected (" << id << ")";
    } catch (bs::system_error& e) {
        status = Invalid;
        io_service_.reset();
        LOG(ERROR) << "create rawclient error";
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
    }

    CHECK_EQ(Idle, status) << "not Idle (" << id << ")";
    DLOG(INFO) << "Correctly instantiated (" << id << ")";
}


RawClient::~RawClient() {
    if (!is_aborted_)
    {
        // delete while socket is still open, we need wake up timeout event
        // to make sure timeout handler removed.
        deadline_.expires_at(deadline_timer::traits_type::now());
        io_service_.run_one();
    }
    // no active async handler, reset io_service to get ready for next run_one.
    io_service_.reset();
    CHECK_NE(Busy, status);
    try {
        DLOG(INFO) << "closing (" << id << ") ...";
        
        if (socket.is_open())
            socket.shutdown(socket.shutdown_both);
        socket.close();
        
        LOG(INFO) << "connection closed (" << id << ")";
    } catch (bs::system_error& e) {
        LOG(WARNING) << e.what();
    }
    
    LOG(INFO) << "Correctly destroyed (" << id << ")";
}

void async_cb(const bs::error_code& error, std::size_t bytes, bs::error_code* ret_ec)
{
    *ret_ec = error;
}

void RawClient::connect_with_timeout(const std::string& host, const std::string& port)
{
    try
    {
        ba::ip::tcp::resolver resolver(io_service_);
        ba::ip::tcp::resolver::query query(host, port);
        tcp::resolver::iterator iter = resolver.resolve(query);

        deadline_.expires_from_now(boost::posix_time::seconds(CONNECT_TIMEOUT));
        bs::error_code ec = ba::error::would_block;

        ba::async_connect(socket, iter,
            boost::bind(&async_cb, _1, 0, &ec));

        do io_service_.run_one(); while (ec == ba::error::would_block);

        if (ec || !socket.is_open())
        {   
            throw bs::system_error(ec ? ec : ba::error::operation_aborted);
        }
        deadline_.expires_at(boost::posix_time::pos_infin);
    }
    catch(const bs::system_error& e)
    {
        if (!is_aborted_)
        {
            deadline_.expires_at(deadline_timer::traits_type::now());
            io_service_.run_one();
        }
        throw;
    }
}

size_t RawClient::read_with_timeout(const ba::mutable_buffers_1& buf)
{
    deadline_.expires_from_now(boost::posix_time::seconds(RW_TIMEOUT + ba::buffer_size(buf)/1024/1024));
    bs::error_code ec = ba::error::would_block;
    ba::async_read(socket, buf, boost::bind(&async_cb, _1, _2, &ec));
    do io_service_.run_one(); while(ec == ba::error::would_block);

    if (ec)
    {
        if (!is_aborted_)
        {
            deadline_.expires_at(deadline_timer::traits_type::now());
            io_service_.run_one();
        }
        throw bs::system_error(ec);
    }
    deadline_.expires_at(boost::posix_time::pos_infin);
    return ba::buffer_size(buf);
}

size_t RawClient::write_with_timeout(const boost::array<ba::const_buffer,3>& buffers)
{
    deadline_.expires_from_now(boost::posix_time::seconds(RW_TIMEOUT));
    bs::error_code ec = ba::error::would_block;
    ba::async_write(socket, buffers, boost::bind(&async_cb, _1, _2, &ec));
    do io_service_.run_one(); while(ec == ba::error::would_block);
    if (ec)
    {
        if (!is_aborted_)
        {
            deadline_.expires_at(deadline_timer::traits_type::now());
            io_service_.run_one();
        }
        throw bs::system_error(ec);
    }
    deadline_.expires_at(boost::posix_time::pos_infin);
    return ba::buffer_size(buffers);
}

void RawClient::check_deadline()
{
    if (deadline_.expires_at() <= deadline_timer::traits_type::now())
    {
        LOG(INFO) << "deadline for : " << path;
        boost::system::error_code ignored_ec;
        is_aborted_ = true;
        try
        {
            if (socket.is_open() && status != Invalid)
            {
                status = Invalid;
                socket.shutdown(ba::ip::tcp::socket::shutdown_both, ignored_ec);
                socket.close(ignored_ec);
            }
            // There is no longer an active deadline. The expiry is set to infinity.
            deadline_.expires_at(boost::posix_time::pos_infin);
        }
        catch(const bs::system_error& e)
        {
            LOG(ERROR) << "check_deadline error : " << e.what();
        }
    }
    else
    {
        is_aborted_ = false;
        deadline_.async_wait(boost::bind(&RawClient::check_deadline, this));
    }
}

bool
RawClient::isConnected() {
    // is the socket open?
    if (not socket.is_open()) {
        LOG(WARNING) << "Socket is not open";
        return false;
    }
    
    // is the endpoint active?
    try {
        socket.remote_endpoint();
    } catch (bs::system_error& e) {
        LOG(WARNING) << "Endpoint error: " << e.what();
        return false;
    }
    
    return true;
}


void
RawClient::sendRequest(const uint32_t& sequence, const string& data) {
    if (not isConnected()) {
        // TODO: keep alive?
        status = Invalid;
        throw NetworkError("Not connected in RawClient");
    }
    
    CHECK_EQ(Idle, status) << "not Idle (" << id << ")";
    status = Busy;
    
    DLOG(INFO) << "Sending raw request (" << id << "): ["
               << sequence << ", " 
               << data.length() << ", "
               << data << "] ...";
    
    uint32_t seq = htonl(sequence);
    uint32_t len = htonl(data.length());
    
    boost::array<ba::const_buffer,3> buffers;
    buffers[0] = ba::buffer(&seq, UINT_SIZE);
    buffers[1] = ba::buffer(&len, UINT_SIZE);
    buffers[2] = ba::buffer(data);

    size_t n = 0;
    try {
        n += write_with_timeout(buffers);
    } catch (bs::system_error& e) {
        status = Invalid;
        LOG(ERROR) << "write request to socket error";
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
    }
    
    if (n != HEADER_SIZE + data.length()) {
        status = Invalid;
        throw ServerError("write: Write size mismatch");
    }
    
    // do not change the status
    CHECK_EQ(Busy, status) << "not Busy (" << id << ")";
    DLOG(INFO) << "Request sent (" << n << " bytes).";
}


Response
RawClient::getResponse() {
    if (not isConnected()) {
        // TODO: keep alive?
        status = Invalid;
        throw NetworkError("Not connected in RawClient");
    }
    
    CHECK_EQ(Busy, status) << "not Busy (" << id << ")";
    // do not change the status
    DLOG(INFO) << "Receiving response (" << id << ") ...";

    char header[HEADER_SIZE];
    size_t n = 0;

    try {
        n += read_with_timeout(ba::buffer(header));
    } catch (bs::system_error& e) {
        status = Invalid;
        LOG(ERROR) << "get response head from socket error";
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
    }
    
    if (n != sizeof(HEADER_SIZE)) {
        status = Invalid;
        throw ServerError("Read size mismatch");
    }

    uint32_t sequence, length;
    memcpy(&sequence, header, UINT_SIZE);
    memcpy(&length, header + UINT_SIZE, UINT_SIZE);

    sequence = ntohl(sequence);
    length = ntohl(length);

    static const uint32_t small_response_size = 1024*4;
    static char stack_array[small_response_size];
    char *data = NULL;
    if (length > small_response_size)
    {
        data = new char[length];
    }
    else
    {
        data = stack_array;
    }

    try {
        n = read_with_timeout(ba::buffer(data, length));
    } catch (bs::system_error& e) {
        status = Invalid;
        LOG(ERROR) << "get response body from socket error";
        LOG(ERROR) << e.what();
        if (length > small_response_size)
            delete[] data;
        throw NetworkError(e.what());
    } catch(...)
    {
        if (length > small_response_size)
            delete[] data;
        throw;
    }
    
    if (n != length) {
        status = Invalid;
        if (length > small_response_size)
            delete[] data;
        throw ServerError("Read size mismatch");
    }

    string response(data, length - 1); // skip the final '\0'
    DLOG(INFO) << "Response (" << id << "): ["
               << sequence << ", "
               << length << ", "
               << response << "]";

    LOG(INFO) << "Response received (" << n + HEADER_SIZE << " bytes)";
    
    if (length > small_response_size)
        delete[] data;

    status = Idle;
    return boost::make_tuple(sequence, response);
}


NS_IZENELIB_SF1R_END
