#include <util/ProcMemInfo.h>

NS_IZENELIB_UTIL_BEGIN

void ProcMemInfo::getProcMemInfo(unsigned long & virtualMem, unsigned long & realMem)
    throw (ios_base::failure)
{
    getProcMemInfo(getpid(), virtualMem, realMem);
}

void ProcMemInfo::getProcMemInfo(pid_t pid, unsigned long & virtualMem, unsigned long & realMem)
    throw (ios_base::failure)
{
    char temp[50] = {0,};
    string buffer;
    sprintf(temp, "/proc/%d/statm", pid);

    try
    {
        getStatFile(temp, buffer);
        readProcStatus(buffer, virtualMem, realMem);

        // each page is 4K, so multiply by 4K
        realMem = realMem << 12;
        virtualMem = virtualMem << 12;
    }
    catch (ios_base::failure e)
    {
        throw e;
    }
}

void ProcMemInfo::getStatFile(const string path, string & buffer)
    throw (ios_base::failure)
{
    char tBuf[1024] = {0,};

    ifstream fin;
    fin.exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    try
    {
        fin.open( path.c_str() );
        fin.getline(tBuf, 1024);
        tBuf[fin.gcount()] = '\0';
    }
    catch( ifstream::failure e )
    {
        throw e;
    }

    fin.close();

    buffer = "";
    buffer += tBuf;

}


void ProcMemInfo::readProcStatus(
        const string & buffer,
        unsigned long & virtualMem,
        unsigned long & realMem
        )
{
    const char *pBuf = buffer.c_str();

    sscanf(pBuf, "%lu %lu", &virtualMem, &realMem);
}



NS_IZENELIB_UTIL_END
