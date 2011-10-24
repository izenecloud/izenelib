#include <ir/index_manager/utility/Exception.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

string IndexManagerException::s_errorStrings[NUM_ERRORS] =
{
    "Unknown error",				//UNKNOWN_ERROR
    "Generic error",				//GENERIC_ERROR
    "Missing parameter",			//MISSING_PARAMETER_ERROR
    "Bad parameter",				//BAD_PARAMETER_ERROR
    "File I/O error",				//FILEIO_ERROR
    "Rumtime error",				//RUNTIME_ERROR
    "Out of memory",				//OUTOFMEM_ERROR
    "Illegal argument",				//ILLEGALARGUMENT_ERROR
    "Unsopported operation",		//UNSUPPORTED_ERROR
    "Out of range",				//OUTOFRANG_ERROR
    "Index file collpased",			//INDEX_COLLAPSE_ERROR
    "Version error",				//VERSION_ERROR
    "Assert error",				//ASSERT_ERROR
    "Empty index barrel"			//EMPTY_BARREL_ERROR
};

