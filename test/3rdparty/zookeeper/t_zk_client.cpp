#include <boost/thread.hpp>

#include <3rdparty/zookeeper/ZooKeeper.hpp>

using namespace std;
using namespace boost;
using namespace zookeeper;


int main(int argv, char* argc[])
{
    string hosts = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
    int recvTimeout = 2000;

    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);

    cli.showZKNamespace();

    cli.deleteZNode("/zk_test", true);

    cli.showZKNamespace();

    return 0;
}
