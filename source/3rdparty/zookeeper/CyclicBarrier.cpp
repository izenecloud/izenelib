#include <CyclicBarrier.h>

using namespace izenelib::zookeeper;

boost::timed_mutex CyclicBarrier::t_mutex_;

CyclicBarrier::CyclicBarrier(int parties, const std::string& host, const std::string& root, const std::string& name)
: parties_(parties), root_(root), name_(name), isBarrierComplete_(false)
{
    zk_ = new ZooKeeper(host, 2000);
    zk_->registerEventHandler(this);

    // ensure init
    if (!zk_->isZNodeExists(root_))
    {
        zk_->createZNode(root_);
    }
}

CyclicBarrier::~CyclicBarrier()
{
    if (zk_)
        delete zk_;
}

int CyclicBarrier::await()
{
    std::string mynode = root_+"/"+name_;
    if (!zk_->isZNodeExists(mynode))
        zk_->createZNode(mynode);

    std::vector<std::string> childrenList;
    zk_->getZNodeChildren(root_, childrenList);
    int arrivalIndex = childrenList.size();

    zk_->isZNodeExists(root_+"/ready"); // set watcher
    if (arrivalIndex < parties_)
    {
        while (!isBarrierComplete_)
        {
            sleep(1);
        }
    }
    else
    {
        zk_->createZNode(root_+"/ready");
    }

    return arrivalIndex;
}


int CyclicBarrier::getParties()
{
    return parties_;
}

int CyclicBarrier::getNumberWaiting()
{
    std::vector<std::string> childrenList;
    zk_->getZNodeChildren(root_, childrenList);
    return childrenList.size();
}

void CyclicBarrier::reset()
{
    std::vector<std::string> childrenList;
    zk_->getZNodeChildren(root_, childrenList);
    for (size_t i = 0; i < childrenList.size(); i++)
    {
        zk_->deleteZNode(childrenList[i]);
    }
}

void CyclicBarrier::process(ZooKeeperEvent& zkEvent)
{
    if (zkEvent.path_ == root_+"/ready")
    {
        isBarrierComplete_ = true;
    }
}


