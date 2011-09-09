#include <cstdlib>
#include <ctime>

#include <boost/thread.hpp>

#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>

#include <3rdparty/zookeeper/dsync/DoubleBarrier.h>

using namespace std;
using namespace boost;
using namespace zookeeper;


int main(int argv, char* argc[])
{
    string hosts = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
//    int recvTimeout = 2000;

//    ZooKeeper cli(hosts, recvTimeout);
//    sleep(2);
//
//    cli.showZKNamespace();
//
//    cli.deleteZNode("/zk_test", true);
//
//    cli.showZKNamespace();



    cout << "Enter name:"<<endl;

    std::string name;
    std::cin >> name;
    Barrier barrier(hosts, "/barrier", 3, name);

    cout << "try to enter barrier.."<<endl;
    while (!barrier.enter())
    {
        sleep(1);
    }
    cout << "entered!"<<endl;

    srand(int(name[2]));
    int i = rand() % 10 + 1;
    sleep(i);
    cout << "sleeped "<<i<<" seconds in barrier."<<endl;

    cout << "try to leave barrier.."<<endl;
    while (!barrier.leave())
    {
        sleep(2);
    }
    cout << "left!" <<endl;

    return 0;
}
