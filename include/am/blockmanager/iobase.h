/***************************************************************************
 *
 *  Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2008 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifndef IOBASE_H
#define IOBASE_H


#include <iostream>
#include <algorithm>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <stdexcept>


// Boost.Threads headers
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "semaphore.h"
#include "types.h"
#include "iostats.h"
#include "completion_handler.h"

using namespace std;

NS_IZENELIB_AM_BEGIN


//! \defgroup iolayer I/O primitives layer
//! Group of classes which enable abstraction from operating system calls and support
//! system-independent interfaces for asynchronous I/O.
//! \{

#define BLOCK_ALIGN 4096

typedef void * (*thread_function_t)(void *);

class request;
class request_ptr;

//! \brief Default completion handler class

struct default_completion_handler
{
    //! \brief An operator that does nothing
    void operator () (request *) { }
};

//! \brief Defines interface of file

//! It is a base class for different implementations that might
//! base on various file systems or even remote storage interfaces
class file : private boost::noncopyable
{
protected:

    //! \brief Initializes file object
    //! \param _id file identifier
    //! \remark Called in implementations of file
    file() { }

public:
    //! \brief Definition of acceptable file open modes

    //! Various open modes in a file system must be
    //! converted to this set of acceptable modes
    enum open_mode
    {
        RDONLY = 1,                         //!< only reading of the file is allowed
        WRONLY = 2,                         //!< only writing of the file is allowed
        RDWR = 4,                           //!< read and write of the file are allowed
        CREAT = 8,                          //!< in case file does not exist no error occurs and file is newly created
        DIRECT = 16,                        //!< I/Os proceed bypassing file system buffers, i.e. unbuffered I/O
        TRUNC = 32                          //!< once file is opened its length becomes zero
    };

    //! \brief Schedules asynchronous read request to the file
    //! \param buffer pointer to memory buffer to read into
    //! \param pos starting file position to read
    //! \param bytes number of bytes to transfer
    //! \param on_cmpl I/O completion handler
    //! \return \c request_ptr object, that can be used to track the status of the operation
    virtual request_ptr aread(void * buffer, int64_t pos, size_t bytes,
                              completion_handler on_cmpl) = 0;
    //! \brief Schedules asynchronous write request to the file
    //! \param buffer pointer to memory buffer to write from
    //! \param pos starting file position to write
    //! \param bytes number of bytes to transfer
    //! \param on_cmpl I/O completion handler
    //! \return \c request_ptr object, that can be used to track the status of the operation
    virtual request_ptr awrite(void * buffer, int64_t pos, size_t bytes,
                               completion_handler on_cmpl) = 0;

    //! \brief Changes the size of the file
    //! \param newsize value of the new file size
    virtual void set_size(int64_t newsize) = 0;
    //! \brief Returns size of the file
    //! \return file size in bytes
    virtual int64_t size() = 0;

    //! \brief Locks file for reading and writing
    virtual void lock() { }

    //! \brief Some specialized file types may need to know freed regions
    virtual void delete_region(int64_t offset, unsigned_type size)
    {
        UNUSED(offset);
        UNUSED(size);
    }

    virtual ~file() { }
};

class disk_queue;


class onoff_switch : private boost::noncopyable
{
    boost::mutex mutex;
    boost::condition cond;
    bool _on;

public:
    onoff_switch(bool flag = false) : _on(flag) { }
    ~onoff_switch() {}
    void on()
    {
        boost::mutex::scoped_lock Lock(mutex);
        _on = true;
        Lock.unlock();
        cond.notify_one();
    }
    void off()
    {
        boost::mutex::scoped_lock Lock(mutex);
        _on = false;
        Lock.unlock();
        cond.notify_one();
    }
    void wait_for_on()
    {
        boost::mutex::scoped_lock Lock(mutex);
        if (!_on)
            cond.wait(Lock);

    }
    void wait_for_off()
    {
        boost::mutex::scoped_lock Lock(mutex);
        if (_on)
            cond.wait(Lock);
    }
    bool is_on()
    {
        boost::mutex::scoped_lock Lock(mutex);
        return _on;
    }
};

//! \brief Defines interface of request

//! Since all library I/O operations are asynchronous,
//! one needs to keep track of their status: whether
//! an I/O completed or not.
class request : private boost::noncopyable
{
    friend int wait_any(request_ptr req_array[], int count);
    template <class request_iterator_>
    friend request_iterator_ wait_any(request_iterator_ reqs_begin, request_iterator_ reqs_end);
    friend class file;
    friend class disk_queue;
    friend class request_ptr;

protected:
    virtual bool add_waiter(onoff_switch * sw) = 0;
    virtual void delete_waiter(onoff_switch * sw) = 0;
    //virtual void enqueue () = 0;
    virtual void serve() = 0;
    //virtual unsigned size() const;

    completion_handler on_complete;
    int ref_cnt;
    std::unique_ptr<std::ios_base::failure> error;

    boost::mutex ref_cnt_mutex;

public:
    enum request_type { READ, WRITE };

protected:
    file * file_;
    void * buffer;
    int64_t offset;
    size_t bytes;
    request_type type;

    void completed()
    {
        on_complete(this);
    }

    // returns number of references
    int nref()
    {
        boost::mutex::scoped_lock Lock(ref_cnt_mutex);
        return ref_cnt;
    }

public:
    request(completion_handler on_compl,file * file__,void * buffer_,int64_t offset_,size_t bytes_,request_type type_)
            :on_complete(on_compl),
            ref_cnt(0),
            file_(file__),
            buffer(buffer_),
            offset(offset_),
            bytes(bytes_),
            type(type_)
    {
    }
    //! \brief Suspends calling thread until completion of the request
    virtual void wait() = 0;
    //! \brief Polls the status of the request
    //! \return \c true if request is completed, otherwise \c false
    virtual bool poll() = 0;

    virtual ~request()
    {
    }
    file * get_file() const
    {
        return file_;
    }
    void * get_buffer() const
    {
        return buffer;
    }
    int64_t get_offset() const
    {
        return offset;
    }
    size_t get_size() const
    {
        return bytes;
    }
    size_t size() const
    {
        return bytes;
    }
    request_type get_type() const
    {
        return type;
    }

    virtual std::ostream & print(std::ostream & out) const
    {
        out << "File object address: " << (void *)get_file();
        out << " Buffer address: " << (void *)get_buffer();
        out << " File offset: " << get_offset();
        out << " Transfer size: " << get_size() << " bytes";
        out << " Type of transfer: " << ((get_type() == READ) ? "READ" : "WRITE");
        return out;
    }

    //! \brief Inform the request object that an error occurred
    //! during the I/O execution
    void error_occured(const char * msg)
    {
        error.reset(new std::ios_base::failure(msg));
    }

    //! \brief Inform the request object that an error occurred
    //! during the I/O execution
    void error_occured(const std::string & msg)
    {
        error.reset(new std::ios_base::failure(msg));
    }

    //! \brief Rises an exception if there were error with the I/O
    void check_errors() throw (std::ios_base::failure)
    {
        if (error.get())
            throw * (error.get());
    }

private:
    void add_ref()
    {
        boost::mutex::scoped_lock Lock(ref_cnt_mutex);
        ref_cnt++;
    }

    bool sub_ref()
    {
        int val;
        {
            boost::mutex::scoped_lock Lock(ref_cnt_mutex);
            val = --ref_cnt;
        }
        assert(val >= 0);
        return (val == 0);
    }
};

inline std::ostream & operator << (std::ostream & out, const request & req)
{
    return req.print(out);
}

//! \brief A smart wrapper for \c request pointer.

//! Implemented as reference counting smart pointer.
class request_ptr
{
    request * ptr;
    void add_ref()
    {
        if (ptr)
        {
            ptr->add_ref();
        }
    }
    void sub_ref()
    {
        if (ptr)
        {
            if (ptr->sub_ref())
            {
                delete ptr;
                ptr = NULL;
            }
        }
    }

public:
    //! \brief Constructs an \c request_ptr from \c request pointer
    request_ptr(request * ptr_ = NULL) : ptr(ptr_)
    {
        add_ref();
    }
    //! \brief Constructs an \c request_ptr from a \c request_ptr object
    request_ptr(const request_ptr & p) : ptr(p.ptr)
    {
        add_ref();
    }
    //! \brief Destructor
    ~request_ptr()
    {
        sub_ref();
    }
    //! \brief Assignment operator from \c request_ptr object
    //! \return reference to itself
    request_ptr & operator = (const request_ptr & p)
    {
        // assert(p.ptr);
        return (*this = p.ptr);
    }
    //! \brief Assignment operator from \c request pointer
    //! \return reference to itself
    request_ptr & operator = (request * p)
    {
        if (p != ptr)
        {
            sub_ref();
            ptr = p;
            add_ref();
        }
        return *this;
    }
    //! \brief "Star" operator
    //! \return reference to owned \c request object
    request & operator * () const
    {
        assert(ptr);
        return *ptr;
    }
    //! \brief "Arrow" operator
    //! \return pointer to owned \c request object
    request * operator -> () const
    {
        assert(ptr);
        return ptr;
    }
    //! \brief Access to owned \c request object (synonym for \c operator->() )
    //! \return reference to owned \c request object
    //! \warning Creation another \c request_ptr from the returned \c request or deletion
    //!  causes unpredictable behaviour. Do not do that!
    request * get() const
    {
        return ptr;
    }

    //! \brief Returns true if object is initialized
    bool valid() const
    {
        return ptr;
    }

    //! \brief Returns true if object is not initialized
    bool empty() const
    {
        return !ptr;
    }
};

//! \brief Collection of functions to track statuses of a number of requests

//! \brief Suspends calling thread until \b any of requests is completed
//! \param req_array array of \c request_ptr objects
//! \param count size of req_array
//! \return index in req_array pointing to the \b first completed request
inline int wait_any(request_ptr req_array[], int count);
//! \brief Suspends calling thread until \b all requests are completed
//! \param req_array array of request_ptr objects
//! \param count size of req_array
inline void wait_all(request_ptr req_array[], int count);
//! \brief Polls requests
//! \param req_array array of request_ptr objects
//! \param count size of req_array
//! \param index contains index of the \b first completed request if any
//! \return \c true if any of requests is completed, then index contains valid value, otherwise \c false
inline bool poll_any(request_ptr req_array[], int count, int & index);


void wait_all(request_ptr req_array[], int count)
{
    for (int i = 0; i < count; i++)
    {
        req_array[i]->wait();
    }
}

template <class request_iterator_>
void wait_all(request_iterator_ reqs_begin, request_iterator_ reqs_end)
{
    while (reqs_begin != reqs_end)
    {
        (request_ptr(*reqs_begin))->wait();
        ++reqs_begin;
    }
}

bool poll_any(request_ptr req_array[], int count, int & index)
{
    index = -1;
    for (int i = 0; i < count; i++)
    {
        if (req_array[i]->poll())
        {
            index = i;
            return true;
        }
    }
    return false;
}

template <class request_iterator_>
request_iterator_ poll_any(request_iterator_ reqs_begin, request_iterator_ reqs_end)
{
    while (reqs_begin != reqs_end)
    {
        if ((request_ptr(*reqs_begin))->poll())
            return reqs_begin;

        ++reqs_begin;
    }
    return reqs_end;
}


int wait_any(request_ptr req_array[], int count)
{
    stats::scoped_wait_timer wait_timer;

    onoff_switch sw;
    int i = 0, index = -1;

    for ( ; i < count; i++)
    {
        if (req_array[i]->add_waiter(&sw))
        {
            // already done
            index = i;

            while (--i >= 0)
                req_array[i]->delete_waiter(&sw);

            req_array[index]->check_errors();

            return index;
        }
    }

    sw.wait_for_on();

    for (i = 0; i < count; i++)
    {
        req_array[i]->delete_waiter(&sw);
        if (index < 0 && req_array[i]->poll())
            index = i;
    }

    return index;
}

template <class request_iterator_>
request_iterator_ wait_any(request_iterator_ reqs_begin, request_iterator_ reqs_end)
{
    stats::scoped_wait_timer wait_timer;

    onoff_switch sw;

    request_iterator_ cur = reqs_begin, result = reqs_end;

    for ( ; cur != reqs_end; cur++)
    {
        if ((request_ptr(*cur))->add_waiter(&sw))
        {
            // already done
            result = cur;

            if (cur != reqs_begin)
            {
                while (--cur != reqs_begin)
                    (request_ptr(*cur))->delete_waiter(&sw);

                (request_ptr(*cur))->delete_waiter(&sw);
            }

            (request_ptr(*result))->check_errors();

            return result;
        }
    }

    sw.wait_for_on();

    for (cur = reqs_begin; cur != reqs_end; cur++)
    {
        (request_ptr(*cur))->delete_waiter(&sw);
        if (result == reqs_end && (request_ptr(*cur))->poll())
            result = cur;
    }

    return result;
}

class disk_queue : public singleton<disk_queue>
{

public:
    enum priority_op { READ, WRITE, NONE };

private:
    boost::mutex write_mutex;
    boost::mutex read_mutex;
    std::queue<request_ptr> write_queue;
    std::queue<request_ptr> read_queue;

    semaphore sem;

    priority_op _priority_op;

    //boost::thread thread;
    pthread_t thread;

    static void * worker(void * arg)
    {
        disk_queue * pthis = static_cast<disk_queue *>(arg);
        request_ptr req;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

        bool write_phase = true;
        for ( ; ; )
        {
            pthis->sem--;
            //pthread_testcancel();

            if (write_phase)
            {
                boost::mutex::scoped_lock WriteLock(pthis->write_mutex);
                if (!pthis->write_queue.empty())
                {
                    req = pthis->write_queue.front();
                    pthis->write_queue.pop();

                    WriteLock.unlock();

                    req->serve();
                }
                else
                {
                    WriteLock.unlock();

                    pthis->sem++;

                    if (pthis->_priority_op == WRITE)
                        write_phase = false;
                }

                if (pthis->_priority_op == NONE
                        || pthis->_priority_op == READ)
                    write_phase = false;
            }
            else
            {
                boost::mutex::scoped_lock ReadLock(pthis->read_mutex);

                if (!pthis->read_queue.empty())
                {
                    req = pthis->read_queue.front();
                    pthis->read_queue.pop();

                    ReadLock.unlock();

                    req->serve();
                }
                else
                {
                    ReadLock.unlock();

                    pthis->sem++;

                    if (pthis->_priority_op == READ)
                        write_phase = true;
                }

                if (pthis->_priority_op == NONE
                        || pthis->_priority_op == WRITE)
                    write_phase = true;
            }
        }

        return NULL;
    }

public:
    disk_queue()
            : sem(0),
            _priority_op(WRITE)
            //,thread(boost::bind(worker, static_cast<void *>(this))) // max number of requests simultaneously submitted to disk
    {
        pthread_create(&thread, NULL,(thread_function_t)worker, static_cast<void *>(this));
    }

    void set_priority_op(priority_op op)
    {
        _priority_op = op;
    }

    void add_readreq(request_ptr & req)
    {
        {
            boost::mutex::scoped_lock Lock(read_mutex);
            read_queue.push(req);
        }

        sem++;
    }
    void add_writereq(request_ptr & req)
    {
        {
            boost::mutex::scoped_lock Lock(write_mutex);
            write_queue.push(req);
        }

        sem++;
    }
    ~disk_queue() 
    {
        pthread_cancel(thread);
        pthread_join(thread, NULL);
    }
};
class state : private boost::noncopyable
{
    boost::mutex mutex;
    boost::condition cond;
    int _state;

public:
    state(int s = 0) : _state(s) {}
    ~state() {}
    void set_to(int new_state)
    {
        boost::mutex::scoped_lock Lock(mutex);
        _state = new_state;
        Lock.unlock();
        cond.notify_all();
    }
    void wait_for(int needed_state)
    {
        boost::mutex::scoped_lock Lock(mutex);
        while (needed_state != _state)
            cond.wait(Lock);
    }
    int operator () ()
    {
        boost::mutex::scoped_lock Lock(mutex);
        return _state;
    }
};

class boostfd_file : public file
{
public:
    typedef boost::iostreams::file_descriptor fd_type;

protected:
    fd_type file_des;
    int mode_;

public:
    boostfd_file(const std::string & filename,int mode)
            : file(), mode_(mode)
    {
        BOOST_IOS::openmode boostfd_mode;

        if (mode & RDONLY)
        {
            boostfd_mode = BOOST_IOS::in;
        }

        if (mode & WRONLY)
        {
            boostfd_mode = BOOST_IOS::out;
        }

        if (mode & RDWR)
        {
            boostfd_mode = BOOST_IOS::out | BOOST_IOS::in;
        }

        const boost::filesystem::path fspath(filename,boost::filesystem::native);

        if (mode & TRUNC)
        {
            if (boost::filesystem::exists(fspath))
            {
                boost::filesystem::remove(fspath);
                boost::filesystem::ofstream f(fspath);
                f.close();
                assert(boost::filesystem::exists(fspath));
            }
        }

        if (mode & CREAT)
        {
            // need to be emulated:
            if (!boost::filesystem::exists(fspath))
            {
                boost::filesystem::ofstream f(fspath);
                f.close();
                assert(boost::filesystem::exists(fspath));
            }
        }

        file_des.open(filename, boostfd_mode, boostfd_mode);
    }

    fd_type get_file_des() const
    {
        return file_des;
    }

    ~boostfd_file()
    {
        file_des.close();
    }
    int64_t size()
    {
        int64_t size_ = file_des.seek(0, BOOST_IOS::end);
        return size_;
    }

    void set_size(int64_t newsize)
    {
        int64_t oldsize = size();
        file_des.seek(newsize, BOOST_IOS::beg);
        file_des.seek(0, BOOST_IOS::beg); // not important ?
        assert(size() >= oldsize);
    }

    request_ptr aread(void * buffer,int64_t pos,size_t bytes,completion_handler on_cmpl);
    request_ptr awrite(void * buffer,int64_t pos,size_t bytes,completion_handler on_cmpl);
};

//! \brief Implementation based on boost::iostreams::file_decriptor
class boostfd_request : public request
{
    friend class boostfd_file;

protected:
    // states of request
    enum { OP = 0, DONE = 1, READY2DIE = 2 }; // OP - operating, DONE - request served,
    // READY2DIE - can be destroyed

    state _state;
    mutable boost::mutex waiters_mutex;
    std::set<onoff_switch *> waiters;

    boostfd_request(boostfd_file * f,void * buf,int64_t off,size_t b,request_type t,completion_handler on_cmpl)
            :request(on_cmpl, f, buf, off, b, t),
            _state(OP)
    {}

    bool add_waiter(onoff_switch * sw)
    {
        boost::mutex::scoped_lock Lock(waiters_mutex);

        if (poll())   // request already finished
        {
            return true;
        }

        waiters.insert(sw);

        return false;
    }
    void delete_waiter(onoff_switch * sw)
    {
        boost::mutex::scoped_lock Lock(waiters_mutex);
        waiters.erase(sw);
    }

    int nwaiters() // returns the number of waiters
    {
        boost::mutex::scoped_lock Lock(waiters_mutex);
        return waiters.size();
    }

    void serve()
    {
        if (nref() < 2)
        {
            cout<<"WARNING: serious error, reference to the request is lost before serve (nref="
                << nref() << ") " <<
                " this=" << long(this) << " offset=" << offset << " buffer=" << buffer << " bytes=" << bytes
                << " type=" << ((type == READ) ? "READ" : "WRITE")<<endl;
        }

        boostfd_file::fd_type fd = static_cast<boostfd_file *>(file_)->get_file_des();

        try
        {
            fd.seek(offset, BOOST_IOS::beg);
        }
        catch (const std::exception & ex)
        {
            std::ostringstream msg;
            msg<<"seek() in boostfd_request::serve() offset=" << offset
            << " this=" << long(this) << " buffer=" <<
            buffer << " bytes=" << bytes
            << " type=" << ((type == READ) ? "READ" : "WRITE") << " : " << ex.what()<<endl;

            error_occured(msg.str());
        }

        {
            if (type == READ)
            {
                stats::scoped_read_timer read_timer(size());

                try
                {
                    fd.read((char *)buffer, bytes);
                }
                catch (const std::exception & ex)
                {
                    std::ostringstream msg;
                    msg<<"read() in boostfd_request::serve() offset=" << offset
                    << " this=" << long(this) << " buffer=" <<
                    buffer << " bytes=" << bytes
                    << " type=" << ((type == READ) ? "READ" : "WRITE") <<
                    " nref= " << nref() << " : " << ex.what()<<endl;

                    error_occured(msg.str());
                }
            }
            else
            {
                stats::scoped_write_timer write_timer(size());

                try
                {
                    fd.write((char *)buffer, bytes);
                }
                catch (const std::exception & ex)
                {
                    std::ostringstream msg;
                    msg<<"write() in boostfd_request::serve() offset=" << offset
                    << " this=" << long(this) << " buffer=" <<
                    buffer << " bytes=" << bytes
                    << " type=" << ((type == READ) ? "READ" : "WRITE") <<
                    " nref= " << nref() << " : " << ex.what()<<endl;

                    error_occured(msg.str());
                }

            }
        }

        if (nref() < 2)
        {
            cout<<"WARNING: reference to the request is lost after serve (nref=" << nref() << ") " <<
                " this=" << long(this) <<
                " offset=" << offset << " buffer=" << buffer << " bytes=" << bytes <<
                " type=" << ((type == READ) ? "READ" : "WRITE")<<endl;
        }

        _state.set_to(DONE);

        {
            boost::mutex::scoped_lock Lock(waiters_mutex);

            // << notification >>
            std::for_each(
                waiters.begin(),
                waiters.end(),
                std::mem_fun(&onoff_switch::on));
        }

        completed();
        _state.set_to(READY2DIE);
    }

public:
    virtual ~boostfd_request()
    {
        assert(_state() == DONE || _state() == READY2DIE);
    }
    void wait()
    {

        stats::scoped_wait_timer wait_timer;

        _state.wait_for(READY2DIE);

        check_errors();
    }

    bool poll()
    {

        const bool s = _state() >= DONE;

        check_errors();

        return s;
    }
};

request_ptr boostfd_file::aread(void * buffer,int64_t pos,size_t bytes,completion_handler on_cmpl)
{
    request_ptr req = new boostfd_request(this,buffer, pos, bytes,request::READ, on_cmpl);

    if (!req.get())
        throw std::ios_base::failure("error on aread");

    disk_queue::get_instance()->add_readreq(req);
    return req;
}
request_ptr boostfd_file::awrite(void * buffer,int64_t pos,size_t bytes,completion_handler on_cmpl)
{
    request_ptr req = new boostfd_request(this, buffer, pos, bytes,request::WRITE, on_cmpl);

    if (!req.get())
        throw std::ios_base::failure("error on awrite");

    disk_queue::get_instance()->add_writereq(req);
    return req;
}

NS_IZENELIB_AM_END


#endif

