/// @file IDManagerErrorString.h
///	@date 6/5/2008
/// @author Nguyen, Tuan-Quang
///
//
#ifndef _ID_MANAGER_ERROR_STRING_H_
#define _ID_MANAGER_ERROR_STRING_H_

#include <types.h>

NS_IZENELIB_IR_BEGIN

namespace idmanager{

    /// @brief list of the error code of the DocumentManager
    enum _id_manager_error_code
    {
        SF1_ID_MANAGER_UNKNOWN_ERROR,
        SF1_ID_MANAGER_INCONSISTANT_DATA,
        SF1_ID_MANAGER_INVALID_PARAMETER,
        SF1_ID_MANAGER_OUT_OF_BOUND,
        SF1_ID_MANAGER_CANNOT_ADD_DATA,
        SF1_ID_MANAGER_INCORRECT_VALUE,
        SF1_ID_MANAGER_ERROR_NO,
    };


    /// @brief list of the error string corresponding to each error code of the DocumentManager
    static const char IDManagerErrorString[SF1_ID_MANAGER_ERROR_NO][100] =
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

