/**
* @file       t_BarrelsInfo.h
* @author     Jun
* @version    SF1 v5.0
* @brief Test cases on @c BarrelsInfo.
*
*/

#ifndef T_BARRELS_INFO_H
#define T_BARRELS_INFO_H

#include <string>

#include "IndexerTestFixture.h"

namespace t_BarrelsInfo
{

inline void index(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_BarrelsInfo::index";

    IndexerTestFixture fixture;
    IndexerTestConfig newConfig(config);
    // not to merge when offline mode, in order to check each barrel
    if(newConfig.indexMode_.find("default") != std::string::npos)
        newConfig.isMerge_ = false;
    fixture.configTest(newConfig);

    fixture.checkBarrel(config.iterNum_);

    VLOG(2) << "<= t_BarrelsInfo::index";
}

inline void optimize(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_BarrelsInfo::optimize";

    IndexerTestFixture fixture;
    fixture.configTest(config);

    fixture.optimizeBarrel(config.iterNum_);

    VLOG(2) << "<= t_BarrelsInfo::optimize";
}

inline void createAfterOptimize(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_BarrelsInfo::createAfterOptimize";

    IndexerTestFixture fixture;
    fixture.configTest(config);

    fixture.createAfterOptimizeBarrel(config.iterNum_);

    VLOG(2) << "<= t_BarrelsInfo::createAfterOptimize";
}

inline void pauseResumeMerge(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_BarrelsInfo::pauseResumeMerge";

    IndexerTestFixture fixture;
    fixture.configTest(config);

    fixture.pauseResumeMerge(config.iterNum_);

    VLOG(2) << "<= t_BarrelsInfo::pauseResumeMerge";
}

}
#endif
