/// @file MessageFrameworkErrorString.h
///	@date 6/5/2008
/// @author Nguyen, Tuan-Quang
/// @brief list of the error code of the MessageFramework

#ifndef _MSGFRK_ERROR_STRING_H_
#define _MSGFRK_ERROR_STRING_H_

namespace messageframework
{
		/// @brief list of the error code of the MessageFramework
		enum _error_code
		{
				SF1_MSGFRK_UNKNOWN_ERROR =0,
				SF1_MSGFRK_UNKNOWN_DATATYPE,
				SF1_MSGFRK_INVALID_DATA,
				SF1_MSGFRK_CONNECTION_TIMEOUT,
				SF1_MSGFRK_DATA_NOT_FOUND,
				SF1_MSGFRK_DATA_OUT_OF_RANGE,
				SF1_MSGFRK_MEM_ALLOC_FAILED,
				SF1_MSGFRK_LOGIC_ERROR,
				SF1_MSGFRK_UNSUPPORTED_OPERATION,
				SF1_MSGFRK_ERROR_NO
		};


		/// @brief list of the error string corresponding to each error code of the
		/// MessageFramework
		static const char MessageFrameworkErrorString[SF1_MSGFRK_ERROR_NO][100] =
		{
				"Unknown error",
				"Unknown datatype",
				"Data is invalid",
				"Cannot connect to server, connection is timed out.",
				"Cannot find data",
				"Data is out of range",
				"Cannot allocate memory",
				"Unsupported operation",
				"Logic error"
		};
}// end of namespace messageframework

#define SF1_MSGFRK_NO_ERROR SF1_MSGFRK_ERROR_NO

#endif

