#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <iostream>
#include <sstream>

#include <3rdparty/zookeeper/ZooKeeper.hpp>

using namespace boost;
using namespace boost::lambda;
using namespace zookeeper;


BOOST_AUTO_TEST_SUITE( t_zookeeper )

BOOST_AUTO_TEST_CASE( notify )
{
    std::cout << "---> Note: start ZooKeeper Service (servers at 127.0.0.1:2181~2183) before run tests..\n" << std::endl;

}

BOOST_AUTO_TEST_CASE( zookeeper_client )
{
    std::cout << "---> Test ZooKeeper Client basic functions" << std::endl;

    std::string hosts = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
    int recvTimeout = 3000;

    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);
    BOOST_CHECK_EQUAL(cli.isConnected(), true);

    // create
    std::string path = "/zk_test1";
    std::string data = "hello";
    cli.createZNode(path, data);
    BOOST_CHECK_EQUAL(cli.isZNodeExists(path), true);

    // get
    std::string data_get;
    cli.getZNodeData(path, data_get);
    BOOST_CHECK_EQUAL(data_get, data);

    // set
    std::string data2 = "world";
    cli.setZNodeData(path, data2);
    cli.getZNodeData(path, data_get);
    BOOST_CHECK_EQUAL(data_get, data2);
}

BOOST_AUTO_TEST_CASE( barriers )
{
    std::cout << "---> Test Barriers" << std::endl;

}

BOOST_AUTO_TEST_CASE( producer_consumer_queues )
{
    std::cout << "---> Test Producer-Consumer Queues" << std::endl;

}

BOOST_AUTO_TEST_SUITE_END()
