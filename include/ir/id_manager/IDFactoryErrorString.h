/// @file IDFactoryErrorString.h
///	@date 6/5/2008
/// @author Nguyen, Tuan-Quang
///
//
#ifndef _ID_FACTORY_ERROR_STRING_H_
#define _ID_FACTORY_ERROR_STRING_H_

#include <types.h>

NS_IZENELIB_IR_BEGIN

namespace idmanager{

    /// @brief list of the error code of the DocumentFactory
    enum _id_factory_error_code
    {
        SF1_ID_FACTORY_UNKNOWN_ERROR,
        SF1_ID_FACTORY_INCONSISTANT_DATA,
        SF1_ID_FACTORY_INVALID_PARAMETER,
        SF1_ID_FACTORY_OUT_OF_BOUND,
        SF1_ID_FACTORY_CANNOT_ADD_DATA,
        SF1_ID_FACTORY_INCORRECT_VALUE,
        SF1_ID_FACTORY_ERROR_NO,
    };


    /// @brief list of the error string corresponding to each error code of the DocumentFactory
    static const char IDFactoryErrorString[SF1_ID_FACTORY_ERROR_NO][100] =
    {
        "Unknown error",
        "Inconsistant data",
        "Input parameters are invalid",
        "Data is out of bound",
        "Value is incorrect"
    };

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif

