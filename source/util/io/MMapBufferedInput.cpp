#include <util/io/MMapBufferedInput.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

NS_IZENELIB_UTIL_BEGIN
namespace io{

MMapBufferedInput::MMapBufferedInput(const char* path)
{
    handle_ = ::open (path, O_RDONLY);
    filename_ = path;
    if (handle_ < 0){
        throw std::runtime_error(
            "Open file error: " + filename_);		
    }else{
        // stat it
        struct stat sb;
        if (::fstat (handle_, &sb)){
            throw std::runtime_error(
                 "Open file error: " + filename_);		
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
                throw std::runtime_error(
					"mmap error: " + filename_);		
            }else{
                data_ = (uint8_t*)address;
            }
        }
    }
    pos_ = 0;
    isClone_ = false;
}

MMapBufferedInput::MMapBufferedInput(const MMapBufferedInput& clone){
    data_ = clone.data_;
    pos_ = clone.pos_;
    length_  = clone.length_;
    isClone_ = true;
}

MMapBufferedInput::~MMapBufferedInput(){
    close();
}

BufferedInput* MMapBufferedInput::clone()
{
    MMapBufferedInput* pClone = new MMapBufferedInput(*this);
    pClone->seek(getFilePointer());
    return pClone;
}

void MMapBufferedInput::close()  {
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

void MMapBufferedInput::readInternal(char* b,size_t length,bool bCheck)
{
    memcpy(b, data_+pos_, length);
    pos_+=length;
}

void MMapBufferedInput::seekInternal(int64_t pos)
{
    pos_=pos;
}

}

NS_IZENELIB_UTIL_END

