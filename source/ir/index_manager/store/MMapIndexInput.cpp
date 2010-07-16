#include <ir/index_manager/store/MMapIndexInput.h>

#include <sys/mman.h>

using namespace izenelib::ir::indexmanager;

MMapIndexInput::MMapIndexInput(const char* path)
{
    handle_ = ::open (path, O_RDONLY);
    filename_ = path;
    if (handle_ < 0){
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + filename_);
    }else{
        // stat it
        struct stat sb;
        if (::fstat (handle_, &sb)){
            SF1V5_THROW(ERROR_FILEIO,"File stat error: " + filename_);
        }else{
            // get length from stat
            length_ = sb.st_size;

            if(length_ == 0)
            {
                data_ = 0;
                pos_ = 0;
                isClone_ = false;
                return;
            }

            // mmap the file
            void* address = ::mmap64(0, length_, PROT_READ, MAP_SHARED, handle_, 0);
            if (address == MAP_FAILED){
                SF1V5_THROW(ERROR_FILEIO,"mmap error: " + filename_);
            }else{
                data_ = (uint8_t*)address;
            }
        }
    }
    pos_ = 0;
    isClone_ = false;
}

MMapIndexInput::MMapIndexInput(const MMapIndexInput& clone){
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
    MMapIndexInput* pClone = new MMapIndexInput(*this);
    pClone->seek(getFilePointer());
    return pClone;
}

void MMapIndexInput::close()  {
    if ( !isClone_ ){
        if ( data_ != NULL )
            ::munmap(data_, length_);
            if ( handle_ > 0 )
                ::close(handle_);
	  	handle_ = 0;
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


