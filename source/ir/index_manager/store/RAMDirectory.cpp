#include <ir/index_manager/store/RAMDirectory.h>
#include <ir/index_manager/store/FSDirectory.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

RAMIndexInput::RAMIndexInput(RAMFile* file_)
        :IndexInput(0)
        ,file(file_)
        ,pointer(0)
{
    length_ = file->length;
}
RAMIndexInput::RAMIndexInput(RAMFile* file_,char* buffer,size_t bufSize)
        :IndexInput(buffer,bufSize)
        ,file(file_)
        ,pointer(0)
{
    length_ = file->length;
}
RAMIndexInput::RAMIndexInput(const RAMIndexInput& clone)
        :file(clone.file)
        ,pointer(clone.pointer)
{
    length_ = clone.length_;
}
RAMIndexInput::~RAMIndexInput()
{
    close();
}
void RAMIndexInput::readInternal(char* b,size_t length,bool bCheck)
{
    const int64_t bytesAvailable = file->length - pointer;
    int64_t remainder = ((int64_t)length <= bytesAvailable) ? length : bytesAvailable;
    int64_t start = pointer;
    size_t destOffset = 0;
    int32_t bufferNumber;
    int32_t bufferOffset;
    int32_t bytesInBuffer;
    int32_t bytesToCopy;
    uint8_t* buffer ;
    while (remainder != 0)
    {
        bufferNumber = (int32_t)(start / file->streamBufSize);
        bufferOffset = (int32_t)(start % file->streamBufSize);
        bytesInBuffer = file->streamBufSize- bufferOffset;

        bytesToCopy = bytesInBuffer >= remainder ? static_cast<int32_t>(remainder) : bytesInBuffer;
        buffer = file->buffers[bufferNumber];

        memcpy(b+destOffset,buffer+bufferOffset,bytesToCopy * sizeof(uint8_t));

        destOffset += bytesToCopy;
        start += bytesToCopy;
        remainder -= bytesToCopy;
        pointer += bytesToCopy;
    }
}
IndexInput* RAMIndexInput::clone()
{
    return new RAMIndexInput(*this);
}
void RAMIndexInput::close()
{
    file = 0;
    length_ = 0;
    pointer = 0;
}
void RAMIndexInput::seekInternal(int64_t position)
{
    if (position < 0 || position > length_)
        SF1V5_THROW(ERROR_FILEIO,"RAMIndexInput::seekInternal():file IO seek error. ");
    pointer = position;
}

//////////////////////////////////////////////////////////////////////////
//RAMIndexOutput
RAMIndexOutput::RAMIndexOutput()
{
    file = new RAMFile();
    bDeleteFile = true;
    pointer = 0;
}
RAMIndexOutput::RAMIndexOutput(RAMFile* ramFile)
        :file(ramFile)
        ,bDeleteFile(false)
        ,pointer(0)
{

}
RAMIndexOutput::~RAMIndexOutput()
{
    if (bDeleteFile)
        delete file;
    file = NULL;
}

void RAMIndexOutput::flushBuffer(char* b, size_t len)
{
    uint8_t * buffer = NULL;
    size_t bufferPos = 0;
    int32_t bufferNumber;
    int32_t bufferOffset;
    int32_t bytesInBuffer;
    int32_t remainInSrcBuffer;
    int32_t bytesToCopy;
    while (bufferPos != len)
    {
        bufferNumber = (int32_t)(pointer/file->streamBufSize);
        bufferOffset = (int32_t)(pointer%file->streamBufSize);
        bytesInBuffer = file->streamBufSize - bufferOffset;
        remainInSrcBuffer = (int32_t)(len - bufferPos);
        bytesToCopy = bytesInBuffer >= remainInSrcBuffer ? remainInSrcBuffer : bytesInBuffer;

        if (bufferNumber ==  (int32_t)file->buffers.size())
        {
            buffer = new uint8_t[file->streamBufSize];
            file->buffers.push_back(buffer);
        }
        else
        {
            buffer = file->buffers[bufferNumber];
        }
        memcpy(buffer+bufferOffset, b + bufferPos, bytesToCopy * sizeof(uint8_t));
        bufferPos += bytesToCopy;
        pointer += bytesToCopy;
    }
    if (pointer > file->length)
        file->length = pointer;

    file->lastModified = Utilities::currentTimeMillis();
}
void RAMIndexOutput::seekInternal(int64_t pos)
{
    pointer = pos;
}

void RAMIndexOutput::close()
{
    IndexOutput::close();
}

void RAMIndexOutput::writeTo(IndexOutput* pOutput)
{
    flush();
    int64_t end = file->length;
    int64_t pos = 0;
    int32_t buffer = 0;
    int32_t length ;
    int64_t nextPos ;
    while (pos < end)
    {
        length = file->streamBufSize;
        nextPos = pos + length;
        if (nextPos > end)
        {                        // at the last buffer
            length = (int32_t)(end - pos);
        }
        pOutput->write((const char*)file->buffers[buffer++], length);
        pos = nextPos;
    }
}

//////////////////////////////////////////////////////////////////////////
//RAMDirectory
RAMDirectory::RAMDirectory(void)
{
    rwLock_ = new izenelib::util::ReadWriteLock;
}

RAMDirectory::~RAMDirectory(void)
{
    close();
    delete rwLock_;
}

bool RAMDirectory::fileExists(const string& name) const
{
    return (files.find(name) != files.end());
}

IndexInput* RAMDirectory::openInput(const string& name)
{
    map<string,RAMFile*>::iterator iter = files.find(name);

    if (iter != files.end())
    {
        return new RAMIndexInput(iter->second);
    }
    return NULL;
}

IndexInput* RAMDirectory::openInput(const string& name, size_t bufsize)
{
    map<string,RAMFile*>::iterator iter = files.find(name);
    if (iter != files.end())
    {
        return new RAMIndexInput(iter->second);
    }
    return NULL;
}

void RAMDirectory::deleteFile(const string& filename,bool throwError)
{
    ///TODO:LOCK IT
    map<string,RAMFile*>::iterator iter = files.find(filename);
    if (iter != files.end())
    {
        delete iter->second;
        files.erase(iter);
    }
}
void RAMDirectory::renameFile(const string& from, const string& to)
{
    map<string,RAMFile*>::iterator iter = files.find(from);
    if (iter != files.end())
    {
        files.insert(make_pair(to,iter->second));
        files.erase(iter);
    }
}

void RAMDirectory::deleteFiles(const string& filename,bool throwError)
{
    try
    {
        map<string,RAMFile*>::iterator iter = files.begin();
        string str;
        string::size_type npos = (size_t)-1;
        string::size_type pos ;
        while (iter != files.end())
        {
            str = iter->first;
            pos = str.rfind('.');
            if (pos != npos)
                str = str.substr(0,pos);
            if (str == filename)
            {
                delete iter->second;
                files.erase(iter++);//TODO:
                continue;
                //str.clear();
            }
            iter++;
        }
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_FILEIO,"void RAMDirectory::deleteFiles()");
    }
}


void RAMDirectory::renameFiles(const string& from, const string& to)
{
    string path,fname,fext,s;
    string::size_type npos = (size_t)-1;
    string::size_type pos ;
    map<string,RAMFile*>::iterator iter = files.begin();
    while ( iter != files.end() )
    {
        s = iter->first;
        pos = s.rfind('.');
        if (pos != npos)
        {
            fname = s.substr(0,pos);
            fext = s.substr(pos,s.length()-pos);
        }
        if (fname == from)
        {
            renameFile(s,to+fext);
            fname.clear();
        }
        iter++;
    }
}

IndexOutput* RAMDirectory::createOutput(const string& name, const string& mode)
{
    //TODO:LOCK IT
    map<string,RAMFile*>::iterator iter = files.find(name);

    if (iter != files.end())
    {
        RAMFile* rf = iter->second;
        delete rf;
        files.erase(iter);
    }

    RAMFile* file = new RAMFile();
    files[name] = file;

    RAMIndexOutput* ret = new RAMIndexOutput(file);
    return ret;
}

IndexOutput* RAMDirectory::createOutput(const string& name, size_t buffersize, const string& mode)
{
    //TODO:LOCK IT
    map<string,RAMFile*>::iterator iter = files.find(name);

    if (iter != files.end())
    {
        RAMFile* rf = iter->second;
        delete rf;
        files.erase(iter);
    }

    RAMFile* file = new RAMFile();
    files[name] = file;

    RAMIndexOutput* ret = new RAMIndexOutput(file);
    return ret;
}


void RAMDirectory::close()
{
    //TODO:lock it
    map<string,RAMFile*>::iterator iter = files.begin();
    while ( iter != files.end() )
    {
        delete iter->second;
        iter++;
    }
    files.clear();
}

