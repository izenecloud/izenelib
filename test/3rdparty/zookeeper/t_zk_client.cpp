#include <cstdlib>
#include <ctime>

#include <boost/thread.hpp>

#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>

#include <3rdparty/zookeeper/DoubleBarrier.h>
#include <3rdparty/zookeeper/CyclicBarrier.h>

using namespace std;
using namespace boost;
using namespace zookeeper;


void t_zk_client(string& hosts)
{
    int recvTimeout = 2000;
    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);

    cli.isConnected();

    cli.deleteZNode("/b", true);
    cli.createZNode("/b");

    cli.showZKNamespace();
}

void t_DoubleBarrier(string& hosts)
{
    cout << "enter process name:"<<endl;

    std::string name;
    std::cin >> name;
    DoubleBarrier barrier(hosts, "/barrier", 3, name);

    cout << "try to enter barrier.."<<endl;
    while (!barrier.enter())
    {
        sleep(1);
    }
    cout << "entered!"<<endl;

    srand(int(name[2]));
    int i = rand() % 10 + 1;
    sleep(i);
    cout << "slept "<<i<<" seconds in barrier."<<endl;

    cout << "try to leave barrier.."<<endl;
    while (!barrier.leave())
    {
        sleep(2);
    }
    cout << "left!" <<endl;
}

void t_CyclicBarrier(string& hosts)
{
    cout << "enter process name:"<<endl;

    std::string name;
    std::cin >> name;
    CyclicBarrier cbarrier(3, hosts, "/cyclic", name);

    cout << "try to lock.."<<endl;
    time_t t1 = time(NULL); cout << t1 << endl;
    cbarrier.await();
    time_t t2 = time(NULL); cout <<"finished "<<t2 << endl;

    //sleep(1);
    cbarrier.reset();
}

int main(int argv, char* argc[])
{
    string hosts = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";

    t_zk_client(hosts);

    //t_DoubleBarrier(hosts);

    //t_CyclicBarrier(hosts);

    return 0;
}
