/*
 * LibCassandra
 * Copyright (C) 2010 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#include <string>
#include <set>
#include <sstream>
#include <iostream>

#include <protocol/TBinaryProtocol.h>
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>

#include "libcassandra/genthrift/Cassandra.h"

#include "libcassandra/cassandra.h"
#include "libcassandra/cassandra_factory.h"

using namespace libcassandra;
using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace org::apache::cassandra;
using namespace boost;


CassandraFactory::CassandraFactory(const string& in_host, int in_port)
    :host(in_host)
    ,port(in_port)
{}


CassandraFactory::~CassandraFactory() {}


boost::shared_ptr<Cassandra> CassandraFactory::create()
{
    CassandraClient *thrift_client= createThriftClient();
    boost::shared_ptr<Cassandra> ret(new Cassandra(thrift_client, host, port));
    return ret;
}


boost::shared_ptr<Cassandra> CassandraFactory::create(const string& keyspace)
{
    CassandraClient *thrift_client= createThriftClient();
    boost::shared_ptr<Cassandra> ret(new Cassandra(thrift_client, host, port, keyspace));
    return ret;
}


CassandraClient *CassandraFactory::createThriftClient()
{
    boost::shared_ptr<TTransport> socket(new TSocket(host, port));
    boost::shared_ptr<TTransport> transport= boost::shared_ptr<TTransport>(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    CassandraClient *client= new(std::nothrow) CassandraClient(protocol);

    transport->open(); /* throws an exception */

    return client;
}


const string &CassandraFactory::getHost() const
{
    return host;
}


int CassandraFactory::getPort() const
{
    return port;
}
