/**
* @file        BTPolicy.h
* @version     SF1 v5.0
* @brief BT index merge algorithm
*/
#ifndef BT_POLICY_H
#define BT_POLICY_H

#include <ir/index_manager/index/IndexMergePolicy.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <util/izene_log.h>

#include <utility> //pair
#include <cassert>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class BTPolicy : public IndexMergePolicy
{
public:
    class BTLayer
    {
    public:
        BTLayer(int l,int nLevelSize,int nMaxSize)
                :nLevel_(l)
                ,nMergeTimes_(0)
                ,nLevelSize_(nLevelSize)
        {
            std::string s = "_mid_";
            s = append(s,l);
            s += "_";
            s = append(s,nMergeTimes_);
            pBarrelQueue_ = new MergeBarrelQueue(s.c_str(),nMaxSize);
        }

        ~BTLayer()
        {
            delete pBarrelQueue_;
            pBarrelQueue_ = NULL;
        }

        void add(MergeBarrelEntry* pEntry)
        {
            DVLOG(2) << "BTLayer::add() => pEntry barrel name: " << pEntry->barrelName()
                     << ", pEntry doc count: " << pEntry->numDocs()
                     << ", nLevel_: " << nLevel_
                     << ", nLevelSize_: " << nLevelSize_
                     << ", nMergeTimes_: " << nMergeTimes_;
            pBarrelQueue_->put(pEntry);
        }

        void increaseMergeTimes()
        {
            nMergeTimes_++;
            std::string s = "_mid_";
            s = append(s,nLevel_);
            s += "_";
            s = append(s,nMergeTimes_);
            pBarrelQueue_->setIdentifier(s);
        }

        /**
         * For all barrels in @c pBarrelQueue_, get the range of their base doc id.
         * @return a doc id pair, @c pair.first is the minimum base doc id, and @c pair.second the maximum base doc id
         * @note if @c pBarrelQueue_ contains no barrel, @c pair<BAD_DOCID, BAD_DOCID> is returned
         */
        std::pair<docid_t, docid_t> getBaseDocIDRange() const
        {
            const size_t count = pBarrelQueue_->size();
            if(count == 0)
                return std::make_pair(BAD_DOCID, BAD_DOCID);

            docid_t baseMin, baseMax;
            baseMin = baseMax = pBarrelQueue_->getAt(0)->baseDocID();
            for(size_t i=1; i<count; ++i)
            {
                docid_t base = pBarrelQueue_->getAt(i)->baseDocID();
                if(base < baseMin)
                    baseMin = base;
                else if(base > baseMax)
                    baseMax = base;
            }

            return std::make_pair(baseMin, baseMax);
        }

    private:
        int nLevel_; ///level of this node

        MergeBarrelQueue* pBarrelQueue_; ///index merge barrel

        int nMergeTimes_; ///merge times

        int nLevelSize_; ///size of level

        friend class BTPolicy;
    };

public:
    BTPolicy();

    virtual ~BTPolicy();

    virtual void addBarrel(MergeBarrelEntry* pEntry);

    virtual void endMerge();

private:
    /**
     * For @p nLevelSize within [3^n, 3^n+1), return n.
     * @param nLevelSize the doc num
     * @return the level
     * @note 0 is returned if @p nLevelSize <= 0.
     */
    int getLevel(int64_t nLevelSize);

    void triggerMerge(BTLayer* pLevel);

    int getC(int nLevel);

    /**
     * Combine @p pLayer1 and @p pLayer2.
     * It moves the elements in the lower level to higher level,
     * and returns the higher level.
     * @param pLayer1 layer 1
     * @param pLayer2 layer 2
     * @return the combined layer
     * @pre the level of @p pLayer1 and @p pLayer2 should be different.
     */
    BTLayer* combineLayer(BTLayer* pLayer1, BTLayer* pLayer2) const;

private:
    const static int MAX_LAYER_SIZE = 100; ///< max size for each layer

    std::map<int,int> nCMap_; ///collision factor, when there are nC_ barrels in a same level, a merge will be trigged.

    std::map<int,BTLayer*> nodesMap_;
};

}

NS_IZENELIB_IR_END


#endif

