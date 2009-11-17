#ifndef LOG_MERGER_H
#define LOG_MERGER_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class MultiWayMerger : public IndexMerger
{
public:
    class Generation
    {
    public:
        Generation(int32_t g,int32_t nMaxSize)
            :generation_(g)
            ,mergeTimes_(0)
        {
            std::string s = "_mid_";
            s = append(s,generation_);
            s += "_";
            s = append(s,mergeTimes_);
            pMergeBarrel_ = new MergeBarrel(s.c_str(),nMaxSize);
        }
        ~Generation()
        {
                delete pMergeBarrel_;
                pMergeBarrel_ = NULL;
        }
    public:
        void add(MergeBarrelEntry* pEntry)
        {
            pMergeBarrel_->put(pEntry);
        }				

        void increaseMergeTimes()
       {
            mergeTimes_++;
            std::string s = "_mid_";
            s = append(s,generation_);
            s += "_";
            s = append(s,mergeTimes_);
            pMergeBarrel_->setIdentifier(s);
       }
    private:
        int generation_; ///generation of this sub-index
        MergeBarrel* pMergeBarrel_; ///index merge barrel
        int mergeTimes_;	 ///merge times
        friend class MultiWayMerger;
    }; 
	
public:
    MultiWayMerger(Indexer* pIndexer);
    virtual ~MultiWayMerger(void);
public:
    void addBarrel(MergeBarrelEntry* pEntry);

    void endMerge();
protected:
    void triggerMerge(Generation* pGen,int32_t nGen);

protected:
    const static int32_t MAX_TRIGGERS = 5;
    int curGeneration_;
    map<int,Generation*> generationMap_;
};

}

NS_IZENELIB_IR_END

#endif

