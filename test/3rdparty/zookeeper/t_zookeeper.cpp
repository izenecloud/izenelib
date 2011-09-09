#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <iostream>
#include <sstream>

#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>

using namespace std;
using namespace boost;
using namespace boost::lambda;
using namespace zookeeper;


class WorkerSearch : public ZooKeeperEventHandler
{
public:
    virtual void onSessionConnected()
    {
    }

    virtual void onNodeCreated(const std::string& path)
    {
        cout << "[WorkerSearch] onNodeCreated " << path <<endl;
        path_ = path;
    }

    virtual void onNodeDeleted(const std::string& path)
    {
        cout << "[WorkerSearch] onNodeDeleted " << path <<endl;
    }

    virtual void onDataChanged(const std::string& path)
    {
        cout << "[WorkerSearch] onDataChanged " << path <<endl;
        path_ = path;
    }

    std::string path_;
};

class WorkerMining : public ZooKeeperEventHandler
{
public:
    virtual void onSessionConnected()
    {
    }

    virtual void onNodeCreated(const std::string& path)
    {
        cout << "[WorkerMining] onNodeCreated " << path <<endl;
    }

    virtual void onNodeDeleted(const std::string& path)
    {
        cout << "[WorkerMining] onNodeDeleted " << path <<endl;
    }

    virtual void onDataChanged(const std::string& path)
    {
        cout << "[WorkerMining] onDataChanged " << path <<endl;
    }
};

BOOST_AUTO_TEST_SUITE( t_zookeeper )

BOOST_AUTO_TEST_CASE( check_zookeeper_service )
{
    std::cout << "---> Note: start ZooKeeper Service (servers at 127.0.0.1:2181~2183) firstly before test." << std::endl;
    std::cout << "        " << std::endl;
}

BOOST_AUTO_TEST_CASE( zookeeper_client_basic )
{
    std::cout << "---> Test ZooKeeper Client basic functions" << std::endl;

    std::string hosts = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
    int recvTimeout = 3000;

    // Zookeeper Client
    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);
    BOOST_CHECK_EQUAL(cli.isConnected(), true);

    // remove all
    BOOST_CHECK_EQUAL(cli.deleteZNode("/SF1", true), true);

    // create
    std::string path = "/SF1";
    std::string data = "distributed search";
    cli.createZNode(path, data);
    BOOST_CHECK_EQUAL(cli.isZNodeExists(path), true);

    // get
    std::string data_get;
    cli.getZNodeData(path, data_get);
    BOOST_CHECK_EQUAL(data_get, data);

    // set
    std::string data2 = "distributed search (sf1-kite)";
    BOOST_CHECK_EQUAL(cli.setZNodeData(path, data2), true);
    cli.getZNodeData(path, data_get);
    BOOST_CHECK_EQUAL(data_get, data2);

    // children
    std::string master = "/SF1/Master";
    std::string worker1 = "/SF1/Worker1";
    std::string worker2 = "/SF1/Worker2";
    BOOST_CHECK_EQUAL(cli.createZNode(master, "this is master node"), true);
    BOOST_CHECK_EQUAL(cli.createZNode(worker1, "remote worker1"), true);
    BOOST_CHECK_EQUAL(cli.createZNode(worker2, "remote worker2"), true);
    std::vector<std::string> children;
    cli.getZNodeChildren("/SF1", children);
    BOOST_CHECK_EQUAL(children.size(), 3);
    BOOST_CHECK_EQUAL(children[0], master);
    BOOST_CHECK_EQUAL(children[1], worker1);
    BOOST_CHECK_EQUAL(children[2], worker2);

    // display
    cli.showZKNamespace("/SF1");
}

BOOST_AUTO_TEST_CASE( zookeeper_watch )
{
    std::cout << "---> Test ZooKeeper Watcher" << std::endl;

    // Client
    std::string hosts = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
    int recvTimeout = 2000;
    ZooKeeper cli(hosts, recvTimeout);
    sleep(1);

    // set event handlers for watcher
    WorkerSearch wkSearch;
    WorkerMining wkMining;
    cli.registerEventHandler(&wkSearch);
    cli.registerEventHandler(&wkMining);

    // 1. get and watch znode for changes
    std::string path = "/SF1/Master";
    std::string data_get;
    cli.getZNodeData(path, data_get);
    BOOST_CHECK_EQUAL(data_get, "this is master node"); // set in former test case

    cli.setZNodeData(path, "master data changed!");
    sleep(1);

    // master was notified by watcher on znode changed
    BOOST_CHECK_EQUAL(wkSearch.path_, path);
    cli.getZNodeData(wkSearch.path_, data_get);
    BOOST_CHECK_EQUAL(data_get, "master data changed!");

    // 2. check exists and watch znode for creation
    std::string path2 = "/NotExistedNode";
    cli.deleteZNode(path2, true);
    BOOST_CHECK_EQUAL(cli.isZNodeExists(path2), false);

    cli.createZNode(path2, "nodata");
    sleep(1);

    // master was notified by watcher on znode created
    BOOST_CHECK_EQUAL(wkSearch.path_, path2);
    cli.getZNodeData(wkSearch.path_, data_get);
    BOOST_CHECK_EQUAL(data_get, "nodata");
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
