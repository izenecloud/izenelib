#include <ir/index_manager/store/FSIndexInput.h>
#include <ir/index_manager/utility/Utilities.h>
#include <util/izene_log.h>

using namespace izenelib::ir::indexmanager;

FSIndexInput::FSIndexInput(const char* filename)
{
    DVLOG(5) << "=> FSIndexInput::FSIndexInput(), filename: " << filename;
    fileHandle_ = fopen(filename, "rb");

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        perror(filename);
        string sFileName = filename;
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + sFileName);
    }
    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);

    filename_ = filename;
    DVLOG(5) << "<= FSIndexInput::FSIndexInput(), length_: " << length_;
}

FSIndexInput::FSIndexInput(const char* filename,size_t buffsize)
        :IndexInput(buffsize)
{
    DVLOG(5) << "=> FSIndexInput::FSIndexInput(), filename: " << filename << ", buffsize: " << buffsize;
    fileHandle_ = fopen(filename, "rb");

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        string sFileName = filename;
        cerr<<"open file error "<<sFileName<<endl;
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + sFileName);
    }

    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);

    filename_ = filename;
    DVLOG(5) << "<= FSIndexInput::FSIndexInput(), length_: " << length_;
}

FSIndexInput::~FSIndexInput()
{
    close();
}

void FSIndexInput::readInternal(char* b,size_t length,bool bCheck/* = true*/)
{
    DVLOG(5) << "=> FSIndexInput::readInternal(), filename_: " << filename_ << ", length: " << length << ", dirty_: " << dirty_;
    int ret = fread(b, 1, length, fileHandle_);
    if (ret != (int)length)
        if (!feof(fileHandle_))
        {
            close();
            SF1V5_THROW(ERROR_FILEIO,"FSIndexInput::readInternal():file IO read error:" + filename_);
        }
    DVLOG(5) << "<= FSIndexInput::readInternal()";
}

void FSIndexInput::close()
{
    DVLOG(5) << "=> FSIndexInput::close(), filename_: " << filename_ << ", dirty_: " << dirty_;
    if (fileHandle_ )
    {
        fclose(fileHandle_);
        fileHandle_ = NULL;
    }
    DVLOG(5) << "<= FSIndexInput::close()";
}

IndexInput* FSIndexInput::clone()
{
    DVLOG(5) << "=> FSIndexInput::clone(), filename_: " << filename_ << ", dirty_: " << dirty_;
    FSIndexInput* pClone = new FSIndexInput(filename_.c_str(),bufferSize_);
    pClone->seek(getFilePointer());
    DVLOG(5) << "<= FSIndexInput::clone()";
    return pClone;
}
void FSIndexInput::seekInternal(int64_t position)
{
    DVLOG(5) << "=> FSIndexInput::seekInternal(), filename_: " << filename_ << ", position: " << position << ", dirty_: " << dirty_;
    if (0 != fseek(fileHandle_, position, SEEK_SET))
    {
        close();
        SF1V5_THROW(ERROR_FILEIO,"FSIndexInput::seekInternal():file IO seek error: " + filename_);
    }
    DVLOG(5) << "<= FSIndexInput::seekInternal()";
}

void FSIndexInput::reopen()
{
    DVLOG(5) << "=> FSIndexInput::reopen(), filename_: " << filename_ << ", dirty_: " << dirty_;
    IndexInput::reset();
    fclose(fileHandle_);
    fileHandle_ = fopen(filename_.c_str(), "rb");
    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);
    DVLOG(5) << "<= FSIndexInput::reopen()";
}

