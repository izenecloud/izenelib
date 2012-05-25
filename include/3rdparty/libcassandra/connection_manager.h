#ifndef __LIBCASSANDRA_CASSANDRA_CONNECTION_MANAGER_H
#define __LIBCASSANDRA_CASSANDRA_CONNECTION_MANAGER_H


#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <util/singleton.h>

#include <list>

namespace libcassandra
{

class MyCassandraClient;
class CassandraConnectionManager
{
public:
    CassandraConnectionManager();

    ~CassandraConnectionManager();

    /// for singleton
    static CassandraConnectionManager* instance()
    {
        return ::izenelib::util::Singleton<CassandraConnectionManager>::get();
    }

    void init(const std::string& host = "localhost", int port = 9160, size_t pool_size = 16);

    void clear();

    bool reconnect();

    MyCassandraClient * borrowClient();

    void releaseClient(MyCassandraClient * client);

private:
    time_t createTimeStamp() const;

private:
    std::string host_;
    int port_;
    size_t pool_size_;

    time_t last_connect_;
    bool connected_;

    std::list<MyCassandraClient *> clients_;

    boost::mutex mutex_;

    boost::condition_variable cond_;
};

}
#endif
