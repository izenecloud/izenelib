/**
* @file        MultiWayPolicy.h
* @author     Yingfeng Zhang
* @brief   MultiWayPolicy with log merge policy
*/
#ifndef MULTIWAY_POLICY_H
#define MULTIWAY_POLICY_H

#include <ir/index_manager/index/IndexMergePolicy.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class MultiWayPolicy : public IndexMergePolicy
{
public:
    class Generation
    {
    public:
        Generation(int g,int nMaxSize)
            :generation_(g)
            ,mergeTimes_(0)
        {
            std::string s = "_mid_";
            s = append(s,generation_);
            s += "_";
            s = append(s,mergeTimes_);
            pBarrelQueue_ = new MergeBarrelQueue(s.c_str(),nMaxSize);
        }
        ~Generation()
        {
                delete pBarrelQueue_;
                pBarrelQueue_ = NULL;
        }
    public:
        void add(MergeBarrelEntry* pEntry)
        {
            pBarrelQueue_->put(pEntry);
        }				

        void increaseMergeTimes()
       {
            mergeTimes_++;
            std::string s = "_mid_";
            s = append(s,generation_);
            s += "_";
            s = append(s,mergeTimes_);
            pBarrelQueue_->setIdentifier(s);
       }
    private:
        int generation_; ///generation of this sub-index
        MergeBarrelQueue* pBarrelQueue_; ///index merge barrel
        int mergeTimes_;	 ///merge times
        friend class MultiWayPolicy;
    }; 
	
public:
    MultiWayPolicy();
    virtual ~MultiWayPolicy(void);
public:
    virtual void addBarrel(MergeBarrelEntry* pEntry);

    virtual void endMerge();
protected:
    void triggerMerge(Generation* pGen,int nGen);

protected:
    const static int MAX_TRIGGERS = 5;
    int curGeneration_;
    map<int,Generation*> generationMap_;
};

}

NS_IZENELIB_IR_END

#endif

