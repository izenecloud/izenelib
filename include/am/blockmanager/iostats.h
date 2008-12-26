/***************************************************************************
 *
 *  Copyright (C) 2002-2004 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2008 Andreas Beckmann <beckmann@mpi-inf.mpg.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifndef IOSTATS_H
#define IOSTATS_H


#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cmath>


#include "types.h"
#include "singleton.h"

using namespace std;

NS_IZENELIB_AM_BEGIN


inline double timestamp()
{
    boost::posix_time::ptime MyTime = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration Duration =
        MyTime - boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
    double sec = double(Duration.hours()) * 3600. +
                 double(Duration.minutes()) * 60. +
                 double(Duration.seconds()) +
                 double(Duration.fractional_seconds()) / (pow(10., Duration.num_fractional_digits()));
    return sec;
}


//! \addtogroup iolayer
//!
//! \{

//! \brief Collects various I/O statistics
//! \remarks is a singleton
class stats : public singleton<stats>
{
    friend class singleton<stats>;

    unsigned reads, writes;                     // number of operations
    int64_t volume_read, volume_written;          // number of bytes read/written
    double t_reads, t_writes;                   // seconds spent in operations
    double p_reads, p_writes;                   // seconds spent in parallel operations
    double p_begin_read, p_begin_write;         // start time of parallel operation
    double p_ios;                               // seconds spent in all parallel I/O operations (read and write)
    double p_begin_io;
    double t_waits, p_waits;                    // seconds spent waiting for completion of I/O operations
    double p_begin_wait;
    int acc_reads, acc_writes;                  // number of requests, participating in parallel operation
    int acc_ios;
    int acc_waits;
    double last_reset;
    boost::mutex read_mutex, write_mutex, io_mutex, wait_mutex;

    stats() {}
    ~stats() {}
public:
    class scoped_write_timer
    {
        typedef unsigned_type size_type;

        bool running;

    public:
        scoped_write_timer(size_type size)
                :running(false)
        {
            start(size);
        }

        ~scoped_write_timer()
        {
            stop();
        }

        void start(size_type size)
        {
            if (!running)
            {
                running = true;
                stats::get_instance()->write_started(size);
            }
        }

        void stop()
        {
            if (running)
            {
                stats::get_instance()->write_finished();
                running = false;
            }
        }
    };

    class scoped_read_timer
    {
        typedef unsigned_type size_type;

        bool running;

    public:
        scoped_read_timer(size_type size)
                :running(false)
        {
            start(size);
        }

        ~scoped_read_timer()
        {
            stop();
        }

        void start(size_type size)
        {
            if (!running)
            {
                running = true;
                stats::get_instance()->read_started(size);
            }
        }

        void stop()
        {
            if (running)
            {
                stats::get_instance()->read_finished();
                running = false;
            }
        }
    };

    class scoped_wait_timer
    {
        bool running;

    public:
        scoped_wait_timer()

                : running(false)
        {
            start();
        }

        ~scoped_wait_timer()
        {
            stop();
        }

        void start()
        {
            if (!running)
            {
                running = true;
                stats::get_instance()->wait_started();
            }
        }

        void stop()
        {
            if (running)
            {
                stats::get_instance()->wait_finished();
                running = false;
            }
        }
    };

public:
    //! \brief Returns total number of reads
    //! \return total number of reads
    unsigned get_reads() const
    {
        return reads;
    }

    //! \brief Returns total number of writes
    //! \return total number of writes
    unsigned get_writes() const
    {
        return writes;
    }

    //! \brief Returns number of bytes read from disks
    //! \return number of bytes read
    int64_t get_read_volume() const
    {
        return volume_read;
    }

    //! \brief Returns number of bytes written to the disks
    //! \return number of bytes written
    int64_t get_written_volume() const
    {
        return volume_written;
    }

    //! \brief Time that would be spent in read syscalls if all parallel reads were serialized.
    //! \return seconds spent in reading
    double get_read_time() const
    {
        return t_reads;
    }

    //! \brief Time that would be spent in write syscalls if all parallel writes were serialized.
    //! \return seconds spent in writing
    double get_write_time() const
    {
        return t_writes;
    }

    //! \brief Period of time when at least one I/O thread was executing a read.
    //! \return seconds spent in reading
    double get_pread_time() const
    {
        return p_reads;
    }

    //! \brief Period of time when at least one I/O thread was executing a write.
    //! \return seconds spent in writing
    double get_pwrite_time() const
    {
        return p_writes;
    }

    //! \brief Period of time when at least one I/O thread was executing a read or a write.
    //! \return seconds spent in I/O
    double get_pio_time() const
    {
        return p_ios;
    }

    //! \brief I/O wait time counter
    //! \return number of seconds spent in I/O waiting functions
    //!  \link request::wait request::wait \endlink,
    //!  \c wait_any and
    //!  \c wait_all
    double get_io_wait_time() const
    {
        return t_waits;
    }

    //! \brief Return time of the last reset
    //! \return seconds passed from the last reset()
    double get_last_reset_time() const
    {
        return last_reset;
    }

    //! \brief Resets I/O time counters (including I/O wait counter)
    void reset()
    {
        {
            boost::mutex::scoped_lock ReadLock(read_mutex);

            //assert(acc_reads == 0);
            if (acc_reads)
                cout<<"Warning: " << acc_reads <<
                    " read(s) not yet finished"<<endl;

            reads = 0;

            volume_read = 0;
            t_reads = 0;
            p_reads = 0.0;
        }
        {
            boost::mutex::scoped_lock WriteLock(write_mutex);

            //assert(acc_writes == 0);
            if (acc_writes)
                cout<<"Warning: " << acc_writes <<
                    " write(s) not yet finished"<<endl;

            writes = 0;

            volume_written = 0;
            t_writes = 0.0;
            p_writes = 0.0;
        }
        {
            boost::mutex::scoped_lock IOLock(io_mutex);

            //assert(acc_ios == 0);
            if (acc_ios)
                cout<<"Warning: " << acc_ios <<
                    " io(s) not yet finished"<<endl;

            p_ios = 0.0;
        }
        {
            boost::mutex::scoped_lock WaitLock(wait_mutex);

            //assert(acc_waits == 0);
            if (acc_waits)
                cout<<"Warning: " << acc_waits <<
                    " wait(s) not yet finished"<<endl;

            t_waits = 0.0;
            p_waits = 0.0;
        }

        last_reset = timestamp();
    }


    // for library use
    void write_started(unsigned size_)
    {
        double now = timestamp();
        {
            boost::mutex::scoped_lock WriteLock(write_mutex);

            ++writes;
            volume_written += size_;
            double diff = now - p_begin_write;
            t_writes += double(acc_writes) * diff;
            p_begin_write = now;
            p_writes += (acc_writes++) ? diff : 0.0;
        }
        {
            boost::mutex::scoped_lock IOLock(io_mutex);

            double diff = now - p_begin_io;
            p_ios += (acc_ios++) ? diff : 0.0;
            p_begin_io = now;
        }
    }
    void write_finished()
    {
        double now = timestamp();
        {
            boost::mutex::scoped_lock WriteLock(write_mutex);

            double diff = now - p_begin_write;
            t_writes += double(acc_writes) * diff;
            p_begin_write = now;
            p_writes += (acc_writes--) ? diff : 0.0;
        }
        {
            boost::mutex::scoped_lock IOLock(io_mutex);

            double diff = now - p_begin_io;
            p_ios += (acc_ios--) ? diff : 0.0;
            p_begin_io = now;
        }
    }
    void read_started(unsigned size_)
    {
        double now = timestamp();
        {
            boost::mutex::scoped_lock ReadLock(read_mutex);

            ++reads;
            volume_read += size_;
            double diff = now - p_begin_read;
            t_reads += double(acc_reads) * diff;
            p_begin_read = now;
            p_reads += (acc_reads++) ? diff : 0.0;
        }
        {
            boost::mutex::scoped_lock IOLock(io_mutex);

            double diff = now - p_begin_io;
            p_ios += (acc_ios++) ? diff : 0.0;
            p_begin_io = now;
        }
    }
    void read_finished()
    {
        double now = timestamp();
        {
            boost::mutex::scoped_lock ReadLock(read_mutex);

            double diff = now - p_begin_read;
            t_reads += double(acc_reads) * diff;
            p_begin_read = now;
            p_reads += (acc_reads--) ? diff : 0.0;
        }
        {
            boost::mutex::scoped_lock IOLock(io_mutex);

            double diff = now - p_begin_io;
            p_ios += (acc_ios--) ? diff : 0.0;
            p_begin_io = now;
        }
    }
    void wait_started()
    {
        double now = timestamp();
        {
            boost::mutex::scoped_lock WaitLock(wait_mutex);

            double diff = now - p_begin_wait;
            t_waits += double(acc_waits) * diff;
            p_begin_wait = now;
            p_waits += (acc_waits++) ? diff : 0.0;
        }
    }
    void wait_finished()
    {
        double now = timestamp();
        {
            boost::mutex::scoped_lock WaitLock(wait_mutex);

            double diff = now - p_begin_wait;
            t_waits += double(acc_waits) * diff;
            p_begin_wait = now;
            p_waits += (acc_waits--) ? diff : 0.0;
        }
    }

};


NS_IZENELIB_AM_END

#endif

