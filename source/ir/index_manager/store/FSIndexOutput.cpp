#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Utilities.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/FSIndexOutput.h>

#include <stdio.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

FSIndexOutput::FSIndexOutput(const char* filename, const string& mode)
    :IndexOutput(0)
    ,filename_(filename)
{
    if (mode.compare("w+b") == 0)
        fileHandle_ = fopen(filename, "w+b");
    else if(mode.compare("r+") == 0)
    {
        fileHandle_ = fopen(filename, "r+");
        if(fileHandle_ == NULL)
            fileHandle_ = fopen(filename, "a+");
    }
    else if(mode.compare("a+") == 0)
        fileHandle_ = fopen(filename, "a+");
    else
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);
    }

    setbuf(fileHandle_,NULL);
}

FSIndexOutput::FSIndexOutput(const char* filename, size_t buffsize, const string& mode)
    :IndexOutput(buffsize)
    ,filename_(filename)
{
    if (mode.compare("w+b") == 0)
        fileHandle_ = fopen(filename, "w+b");
    else if(mode.compare("r+") == 0)
    {
        fileHandle_ = fopen(filename, "r+");
        if(fileHandle_ == NULL)
            fileHandle_ = fopen(filename, "a+");
    }
    else if(mode.compare("a+") == 0)
        fileHandle_ = fopen(filename, "a+");
    else
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);
    }

    setbuf(fileHandle_,NULL);
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
        SF1V5_THROW(ERROR_FILEIO,"FSIndexOutput::flushBuffer():file IO write error:");
    }
}

void FSIndexOutput::seek(int64_t pos)
{
    IndexOutput::seek(pos);
    if (0 != fseek(fileHandle_, pos, SEEK_SET))
        SF1V5_THROW(ERROR_FILEIO,"FSIndexOutput::seek():file IO seek error.");
}

int64_t FSIndexOutput::length()
{
    return bufferStart + (int64_t)bufferPosition;
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


