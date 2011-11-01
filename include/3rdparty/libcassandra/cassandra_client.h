#ifndef __LIBCASSANDRA_CASSANDRA_CLIENT_H
#define __LIBCASSANDRA_CASSANDRA_CLIENT_H

#include <protocol/TBinaryProtocol.h>
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>

#include "libcassandra/genthrift/Cassandra.h"

#include <boost/shared_ptr.hpp>
#include <string>

using org::apache::cassandra::CassandraClient;
using namespace apache::thrift::transport;

namespace libcassandra
{
class MyCassandraClient
{
public:
    MyCassandraClient(const std::string& host, int port);

    ~MyCassandraClient();

    CassandraClient* getCassandra(const std::string& keyspace);

    CassandraClient* getCassandra();

    void open();

    bool isOpen();

    void close();

    std::string key_space_;
    std::string host_;
    int port_;

    boost::shared_ptr<TTransport> transport_;
    CassandraClient* thrift_client_;
};

}

#endif

