#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Utilities.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/UDTFSIndexOutput.h>


using namespace std;

using namespace izenelib::ir::indexmanager;

UDTFSIndexOutput::UDTFSIndexOutput(const char* filename,const string& mode):IndexOutput(0),filename_(filename)
{
    int r = fileHandle_.open(filename,mode);
    if (UDTFSError::OK != r)
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + string(filename) + " " + UDTFSError::getErrorMsg(r));
}

UDTFSIndexOutput::UDTFSIndexOutput(const char* filename,char* buffer,size_t buffsize,const string& mode):IndexOutput(buffer,buffsize),filename_(filename)
{
    int r = fileHandle_.open(filename,mode);
    if (UDTFSError::OK != r)
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + string(filename) + " " + UDTFSError::getErrorMsg(r));
}

UDTFSIndexOutput::UDTFSIndexOutput(const char* filename,size_t buffsize,const string& mode):IndexOutput(buffsize),filename_(filename)
{
    int r = fileHandle_.open(filename,mode);
    if (UDTFSError::OK != r)
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + string(filename) + " " + UDTFSError::getErrorMsg(r));
}

UDTFSIndexOutput::~UDTFSIndexOutput(void)
{
    close();
}

void UDTFSIndexOutput::flushBuffer(char* b, size_t len)
{
    int64_t ret = fileHandle_.write(b, len);
    if (len > 0 && len != (size_t)ret)
    {
        SF1V5_THROW(ERROR_FILEIO,"UDTFSIndexOutput::flushBuffer():file IO write error:");
    }
}

void UDTFSIndexOutput::seekInternal(int64_t pos)
{
    fileHandle_.seek(pos);
}

void UDTFSIndexOutput::close()
{
    IndexOutput::close();
    fileHandle_.close();
}

