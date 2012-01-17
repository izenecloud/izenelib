#include <ir/index_manager/store/FSDirectory.h>
#include <ir/index_manager/store/FSIndexInput.h>
#include <ir/index_manager/store/MMapIndexInput.h>
#include <ir/index_manager/store/FSIndexOutput.h>
#include <ir/index_manager/utility/Utilities.h>
#include <util/izene_log.h>

#include <boost/filesystem.hpp>

#include <dirent.h>
#include <map>

using namespace std;

using namespace izenelib::ir::indexmanager;

FSDirectory::FSDirectory(const string& path,bool bCreate)
        : mmap_(false)
        , rwLock_(NULL)
{
    directoryName_ = path;
    if (bCreate)
    {
        create();
    }

    if (!Utilities::dirExists(directoryName_.c_str()))
    {
        string s = directoryName_;
        s += " is not a directoryName_.";
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
    boost::filesystem::create_directories(directoryName_);
}

void FSDirectory::deleteFile(const string& filename,bool throwError)
{
    DVLOG(5) << "FSDirectory::deleteFile(" << filename << ")";
    string fullpath = directoryName_ + "/" + filename;
    if ( unlink(fullpath.c_str()) == -1 && throwError)
    {
        string str = "deleteFile error:";
        str += fullpath;
        throw FileIOException(str);
    }
}
void FSDirectory::renameFile(const string& from, const string& to)
{
    DVLOG(5) << "FSDirectory::renameFile(" << from << ", " << to << ")";
    string tofullpath = directoryName_ + "/" + to;
    string fromfullpath = directoryName_ + "/" + from;
    if ( fileExists(to.c_str()) )
    {
        string s = to;
        s += " already exist";
        throw FileIOException(s);

    }
    else
    {
        DVLOG(5) << "=> rename(), fromfullpath: " << fromfullpath << ", tofullpath: " << tofullpath;
        if ( rename(fromfullpath.c_str(),tofullpath.c_str()) != 0 )
        {
            string s;
            s = "couldn't rename ";
            s += fromfullpath;
            s += " to ";
            s += tofullpath;
            throw FileIOException(s);
        }
        DVLOG(5) << "<= rename()";
    }
}
void FSDirectory::deleteFiles(const string& filename,bool throwError)
{
    DVLOG(5) << "FSDirectory::deleteFiles(" << filename << ")";
    DIR* dir = opendir(directoryName_.c_str());
    struct dirent* fl = readdir(dir);
    struct stat64 buf;

    string path;
    string fname;
    string::size_type npos = (size_t)-1;
    while ( fl != NULL )
    {
        path = directoryName_;
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
                    DVLOG(5) << "=> unlink(), path: " << path;
                    if ( ( unlink( path.c_str() ) == -1 ) && (throwError) )
                    {
                        closedir(dir);
                        string s;
                        s = "Couldn't delete file:";
                        s += path;
                        SF1V5_THROW(ERROR_FILEIO,s);
                    }
                    DVLOG(5) << "<= unlink()";
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
    DVLOG(5) << "FSDirectory::renameFiles(" << from << ", " << to << ")";
    DIR* dir = opendir(directoryName_.c_str());
    struct dirent* fl = readdir(dir);
    struct stat64 buf;

    string path,fname,fext,s;
    string::size_type npos = (size_t)-1;
    while ( fl != NULL )
    {
        path = directoryName_;
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
    string s = directoryName_ + "/" + name;
    return Utilities::dirExists(s.c_str());
}
IndexInput* FSDirectory::openInput(const string& name)
{
    DVLOG(5) << "FSDirectory::openInput(" << name << ")";
    string fullpath = directoryName_ + "/" + name;
    return new FSIndexInput(fullpath.c_str());
}
IndexInput* FSDirectory::openInput(const string& name, size_t bufsize)
{
    DVLOG(5) << "FSDirectory::openInput(" << name << ", " << bufsize << ")";
    string fullpath = directoryName_ + "/" + name;
    return new FSIndexInput(fullpath.c_str(),bufsize);
}

IndexInput* FSDirectory::openMMapInput(const string& name)
{
    DVLOG(5) << "FSDirectory::openMMapInput(" << name << ")";
    string fullpath = directoryName_ + "/" + name;
    return new MMapIndexInput(fullpath.c_str());
}

IndexOutput* FSDirectory::createOutput(const string& name, const string& mode)
{
    DVLOG(5) << "FSDirectory::createOutput(" << name << ")";
    string fullpath = directoryName_ + "/" + name;
    return new FSIndexOutput(fullpath.c_str(), mode);
}

IndexOutput* FSDirectory::createOutput(const string& name, size_t buffersize, const string& mode)
{
    DVLOG(5) << "FSDirectory::createOutput(" << name << ", " << buffersize << ")";
    string fullpath = directoryName_ + "/" + name;
    return new FSIndexOutput(fullpath.c_str(), mode, buffersize);
}

void FSDirectory::close()
{
}

