#include <boost/thread.hpp>

#include <ir/index_manager/index/IndexMergerAgent.h>
#include <ir/index_manager/index/GPartitionMerger.h>

using namespace izenelib::ir::indexmanager;

IndexMergerAgent::IndexMergerAgent(Indexer* pIndexer)
    :pIndexer_(pIndexer)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    nBarrelCounter_ = pBarrelsInfo_->getBarrelCounter();

    pIndexMerger_ = new GPartitionMerger(pIndexer_);
}

IndexMergerAgent::~IndexMergerAgent()
{
    if(!waitingBarrels_.empty())
    {
        for(vector<BarrelInfo*>::iterator iter = waitingBarrels_.begin(); iter != waitingBarrels_.end(); ++iter)
        {
            pBarrelsInfo_->addBarrel((*iter),true);
                delete (*iter);
        }
        triggerMerge();
    }
    delete pIndexMerger_;
}


void IndexMergerAgent::mergeBarrelsInfo()
{
    BarrelsInfo newBarrelsInfo;
    newBarrelsInfo.read(pIndexer_->getDirectory(),"barrels.tmp");
    assert(newBarrelsInfo.getBarrelCount() == 1);

    BarrelInfo* pNewBarrel = new BarrelInfo(newBarrelsInfo[0]);
    pNewBarrel->setName(getCurrentBarrelName());

    if(pBarrelsInfo_->getLock())
    {
        waitingBarrels_.push_back(pNewBarrel);
    }
    else
    {
	if(!waitingBarrels_.empty())
	{
            for(vector<BarrelInfo*>::iterator iter = waitingBarrels_.begin(); iter != waitingBarrels_.end(); ++iter)
            {
                pBarrelsInfo_->addBarrel((*iter),true);
                delete (*iter);
            }
            waitingBarrels_.clear();
	}
	pBarrelsInfo_->addBarrel(pNewBarrel,false);
    }

    nBarrelCounter_++;
}

void IndexMergerAgent::triggerMerge()
{
    if(pBarrelsInfo_->getLock())
        return;
    boost::thread mergethread(boost::bind(&IndexMergerAgent::mergeIndex, this));
    mergethread.detach();
}

void IndexMergerAgent::mergeIndex()
{
    boost::mutex::scoped_lock lock(this->mutex_);
    pBarrelsInfo_->setLock(true);
    pIndexMerger_->merge(pBarrelsInfo_);
    pBarrelsInfo_->setLock(false);
    pBarrelsInfo_->write(pIndexer_->getDirectory());
}

string IndexMergerAgent::getCurrentBarrelName()
{
    string sName = "_";
    sName = append(sName,nBarrelCounter_);
    return sName;
}

