#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Utilities.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/FSIndexOutput.h>

#include <stdio.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

FSIndexOutput::FSIndexOutput(const char* filename, const string& mode, size_t buffsize)
    :IndexOutput(buffsize)
    ,fileHandle_(NULL)
    ,filename_(filename)
{
    if (mode.compare("w+b") == 0)
        fileHandle_ = fopen(filename, "w+b");
    else if(mode.compare("r+") == 0)
    {
        fileHandle_ = fopen(filename, "r+");
        if(fileHandle_ == NULL)
            fileHandle_ = fopen(filename, "w+b");
    }
    else
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);
    }

    //setbuf(fileHandle_,NULL);
}

FSIndexOutput::~FSIndexOutput(void)
{
    close();
}

void FSIndexOutput::flushBuffer(char* b, size_t len)
{
    size_t ret = fwrite(b, 1, len, fileHandle_);
    fflush(fileHandle_);
    if (len > 0 && len != ret)
    {
        bufferStart_ = 0;
        bufferPosition_ = 0;
        fclose(fileHandle_);
        fileHandle_ = NULL;
        SF1V5_THROW(ERROR_FILEIO,"FSIndexOutput::flushBuffer():file IO write error:");
    }
}

void FSIndexOutput::seekInternal(int64_t pos)
{
    if (0 != fseek(fileHandle_, pos, SEEK_SET))
    {
        close();
        SF1V5_THROW(ERROR_FILEIO,"FSIndexOutput::seek():file IO seek error.");
    }
}

int64_t FSIndexOutput::length()
{
    return bufferStart_ + (int64_t)bufferPosition_;
}

void FSIndexOutput::close()
{
    if (fileHandle_)
    {
        IndexOutput::close();
        fclose(fileHandle_);
        fileHandle_ = NULL;
    }
}

void FSIndexOutput::trunc()
{
    if(fileHandle_)
    {
        close();
    }
    fileHandle_ = fopen(filename_.c_str(), "w+b");
}

