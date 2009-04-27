/* vim: set tabstop=4 : */
#ifndef __febird_io_load_save_file_h__
#define __febird_io_load_save_file_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "mem_map_stream.h"
#include "DataIO.h"

//#include "../statistic_time.h"

namespace febird {

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// for convenient using...

#define FEBIRD_LOAD_FUNCTION_NAME    native_load_from_file
#define FEBIRD_SAVE_FUNCTION_NAME    native_save_to_file
#define FEBIRD_DATA_INPUT_CLASS      LittleEndianDataInput
#define FEBIRD_DATA_OUTPUT_CLASS     LittleEndianDataOutput
#define FEBIRD_DATA_INPUT_LOAD_FROM(input, x)  input >> x
#define FEBIRD_DATA_OUTPUT_SAVE_TO(output, x)  output << x
#include "load_save_convenient.h"


#define FEBIRD_LOAD_FUNCTION_NAME    portable_load_from_file
#define FEBIRD_SAVE_FUNCTION_NAME    portable_save_to_file
#define FEBIRD_DATA_INPUT_CLASS      PortableDataInput
#define FEBIRD_DATA_OUTPUT_CLASS     PortableDataOutput
#define FEBIRD_DATA_INPUT_LOAD_FROM(input, x)  input >> x
#define FEBIRD_DATA_OUTPUT_SAVE_TO(output, x)  output << x
#include "load_save_convenient.h"


#define FEBIRD_LOAD_FUNCTION_NAME    dump_load_from_file
#define FEBIRD_SAVE_FUNCTION_NAME    dump_save_to_file
#define FEBIRD_DATA_INPUT_CLASS      LittleEndianDataInput
#define FEBIRD_DATA_OUTPUT_CLASS     LittleEndianDataOutput
#define FEBIRD_DATA_INPUT_LOAD_FROM(input, x)  DataIO_dump_load_object(input, x)
#define FEBIRD_DATA_OUTPUT_SAVE_TO(output, x)  DataIO_dump_save_object(output, x)
#include "load_save_convenient.h"

/**
 @brief 更新文件
 */
#define FEBIRD_SAVE_FUNCTION_NAME    dump_update_file_only
#define FEBIRD_DATA_OUTPUT_CLASS     LittleEndianDataOutput
#define FEBIRD_DATA_OUTPUT_SAVE_TO(output, x)  x.dump_save(output)
#define FEBIRD_SAVE_FILE_OPEN_MODE "rb+" //!< 可读可写
#include "load_save_convenient.h"

template<class Object>
void dump_update_file(const Object& x, const std::string& szFile, bool printLog=true)
{
	try {
		dump_update_file_only(x, szFile, printLog);
	}
	catch (const OpenFileException&) {
		dump_save_to_file(x, szFile, printLog);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


} // namespace febird

#endif // __febird_io_load_save_file_h__
