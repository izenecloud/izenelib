/**
 * @file    type_defs.h
 * @brief   defines basic types that are used among the modules in sf1v5
 * @author  MyungHyun Lee (Kent)
 * @date    2008-12-30
 * @details
 *  - Log
 *      - 2008.02.05 : Added ID related types which is used in ranking-manager and document-manager, by Dohyun Yun
 */


#ifndef _IR_TYPE_DEFINITIONS_H_
#define _IR_TYPE_DEFINITIONS_H_


NS_IZENELIB_IR_BEGIN
namespace irdb
{


    typedef uint32_t termid_t;
    typedef uint32_t docid_t;
    typedef uint32_t propertyid_t;
    typedef uint32_t labelid_t;
    typedef uint32_t loc_t;
    typedef uint32_t count_t;
    typedef uint32_t collectionid_t;

} // end - namespace
NS_IZENELIB_IR_END
#endif  //_SF1V5_TYPE_DEFINITIONS_H_

