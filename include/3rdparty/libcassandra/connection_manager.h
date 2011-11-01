#ifndef __LIBCASSANDRA_CASSANDRA_CONNECTION_MANAGER_H
#define __LIBCASSANDRA_CASSANDRA_CONNECTION_MANAGER_H


#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <string>
#include <list>

namespace libcassandra
{

class MyCassandraClient;
class CassandraConnectionManager
{
public:
    CassandraConnectionManager(
        const std::string& host,
        int port,
        size_t pool_size);

    ~CassandraConnectionManager();

    MyCassandraClient * borrowClient();

    void releaseClient(MyCassandraClient * client);

private:
    size_t pool_size_;

    std::list<MyCassandraClient *> clients_;

    boost::mutex mutex_;

    boost::condition_variable cond_;
};

}
#endif
