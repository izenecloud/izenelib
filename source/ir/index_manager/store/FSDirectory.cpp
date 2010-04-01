#include <ir/index_manager/store/FSDirectory.h>
#include <ir/index_manager/store/FSIndexInput.h>
#include <ir/index_manager/store/MMapIndexInput.h>
#include <ir/index_manager/store/FSIndexOutput.h>
#include <ir/index_manager/utility/Utilities.h>

#include <dirent.h>
#include <map>

using namespace std;

using namespace izenelib::ir::indexmanager;

FSDirectory::FSDirectory(const string& path,bool bCreate)
        : nRefCount(0)
        , mmap_(false)
        , rwLock_(NULL)
{
    directory = path;
    if (bCreate)
    {
        create();
    }

    if (!Utilities::dirExists(directory.c_str()))
    {
        string s = directory;
        s += " is not a directory.";
        SF1V5_THROW(ERROR_FILEIO,s);
    }
    
    rwLock_ = new izenelib::util::ReadWriteLock;
}

FSDirectory::~FSDirectory(void)
{
    if(rwLock_)
        delete rwLock_;
}


void FSDirectory::create()
{
    if ( !Utilities::dirExists(directory.c_str()) )
    {
        if ( mkdir(directory.c_str(),0777) == -1 )
        {
            string s = "Couldn't create directory: ";
            s += directory;
            SF1V5_THROW(ERROR_FILEIO,s);
        }
        return;
    }
}

void FSDirectory::deleteFile(const string& filename,bool throwError)
{
    string fullpath = directory + "/" + filename;
    if ( unlink(fullpath.c_str()) == -1 && throwError)
    {
        string str = "deleteFile error:";
        str += fullpath;
        throw FileIOException(str);
    }
}
void FSDirectory::renameFile(const string& from, const string& to)
{
    string tofullpath = directory + "/" + to;
    string fromfullpath = directory + "/" + from;
    if ( fileExists(to.c_str()) )
    {
        string s = to;
        s += " already exist";
        throw FileIOException(s);

    }
    else
    {
        if ( rename(fromfullpath.c_str(),tofullpath.c_str()) != 0 )
        {
            string s;
            s = "couldn't rename ";
            s += fromfullpath;
            s += " to ";
            s += tofullpath;
            throw FileIOException(s);
        }
    }
}
void FSDirectory::deleteFiles(const string& filename,bool throwError)
{
    DIR* dir = opendir(directory.c_str());
    struct dirent* fl = readdir(dir);
    struct stat64 buf;

    string path;
    string fname;
    string::size_type npos = (size_t)-1;
    while ( fl != NULL )
    {
        path = directory;
        path += "/";
        path += fl->d_name;
        int32_t ret = stat64(path.c_str(),&buf);
        if ( ret==0 && !(buf.st_mode & S_IFDIR) )
        {
            if ( (strcmp(fl->d_name, ".")) && (strcmp(fl->d_name, "..")) )
            {
                fname = fl->d_name;
                string::size_type pos = fname.rfind('.');
                if (pos != npos)
                    fname = fname.substr(0,pos);
                if (fname == filename)
                {
                    if ( ( unlink( path.c_str() ) == -1 ) && (throwError) )
                    {
                        closedir(dir);
                        string s;
                        s = "Couldn't delete file:";
                        s += path;
                        SF1V5_THROW(ERROR_FILEIO,s);
                    }
                    fname.clear();
                }
            }
        }
        fl = readdir(dir);
    }
    closedir(dir);
}
void FSDirectory::renameFiles(const string& from, const string& to)
{
    DIR* dir = opendir(directory.c_str());
    struct dirent* fl = readdir(dir);
    struct stat64 buf;

    string path,fname,fext,s;
    string::size_type npos = (size_t)-1;
    while ( fl != NULL )
    {
        path = directory;
        path += "/";
        path += fl->d_name;
        int32_t ret = stat64(path.c_str(),&buf);
        if ( ret==0 && !(buf.st_mode & S_IFDIR) )
        {
            if ( (strcmp(fl->d_name, ".")) && (strcmp(fl->d_name, "..")) )
            {
                s = fl->d_name;
                string::size_type pos = s.rfind('.');
                if (pos != npos)
                {
                    fname = s.substr(0,pos);
                    fext = s.substr(pos,s.length()-pos);
                }
                if (fname == from)
                {
                    renameFile(fl->d_name,to+fext);
                    fname.clear();
                }
            }
        }
        fl = readdir(dir);
    }
    closedir(dir);
}
bool FSDirectory::fileExists(const string& name) const
{
    string s = directory + "/" + name;
    return Utilities::dirExists(s.c_str());
}
IndexInput* FSDirectory::openInput(const string& name)
{
    string fullpath = directory + "/" + name;
    return new FSIndexInput(fullpath.c_str());
}
IndexInput* FSDirectory::openInput(const string& name, size_t bufsize)
{
    string fullpath = directory + "/" + name;
    return new FSIndexInput(fullpath.c_str(),bufsize);
}

IndexInput* FSDirectory::openMMapInput(const string& name)
{
    string fullpath = directory + "/" + name;
    return new MMapIndexInput(fullpath.c_str());
}

IndexOutput* FSDirectory::createOutput(const string& name, const string& mode)
{
    string fullpath = directory + "/" + name;
    return new FSIndexOutput(fullpath.c_str(), mode);
}

IndexOutput* FSDirectory::createOutput(const string& name, size_t buffersize, const string& mode)
{
    string fullpath = directory + "/" + name;
    return new FSIndexOutput(fullpath.c_str(), buffersize, mode);
}

void FSDirectory::close()
{
    nRefCount--;
}

