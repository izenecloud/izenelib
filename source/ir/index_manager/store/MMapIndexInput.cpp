#include <ir/index_manager/store/MMapIndexInput.h>

using namespace izenelib::ir::indexmanager;

MMapIndexInput::MMapIndexInput(const char* path)
{
    FILE* fileHandle = fopen(path, "rb");
    filename_ = path;
    if (fileHandle == NULL)
    {
        perror("error when opening file");
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);
    }
    fseek(fileHandle, 0, SEEK_END);
    length_ = ftell(fileHandle);
    fclose(fileHandle);

    m_file_ = new boost::iostreams::mapped_file(path, ios_base::in , length_, 0);
    data_ = (uint8_t*)m_file_->data();
    pos_ = 0;
    isClone_ = false;
}

MMapIndexInput::MMapIndexInput(const MMapIndexInput& clone){
    m_file_ = 0;
    data_ = clone.data_;
    pos_ = clone.pos_;
    length_  = clone.length_;
    isClone_ = true;
}

MMapIndexInput::~MMapIndexInput(){
    close();
}

IndexInput* MMapIndexInput::clone()
{
    return new MMapIndexInput(*this);
}

void MMapIndexInput::close()  {
    if ( !isClone_ ){
        if(m_file_){
            delete m_file_;
            m_file_ = 0;
        }
    }
    data_ = NULL;
    pos_ = 0;
}

void MMapIndexInput::readInternal(char* b,size_t length,bool bCheck)
{
    memcpy(b, data_+pos_, length);
    pos_+=length;
}

void MMapIndexInput::seekInternal(int64_t pos)
{
    pos_=pos;
}


