#include "ProcMemInfo.h"


namespace izenelib{
namespace cache{

void ProcMemInfo::getProcMemInfo(unsigned long & virtualMem, unsigned long & realMem, unsigned long & procMaxMem)
    throw (ios_base::failure)
{
    char temp[50] = {0,};
    string buffer;
    sprintf( temp, "/proc/%d/stat", getpid() );

    try{
        getStatFile(temp, buffer);
        readProcStatus(buffer, virtualMem, realMem, procMaxMem);

        realMem = realMem << 2;    //each page is 4bytes, so multiply by 4
    }
    catch (ios_base::failure e)
    {
        throw e;
    }

}

void ProcMemInfo::getProcMemInfo(pid_t pid, unsigned long & virtualMem, unsigned long & realMem, unsigned long & procMaxMem)
    throw (ios_base::failure)
{
    char temp[50] = {0,};
    string buffer;
    sprintf( temp, "/proc/%d/stat", pid );

    try{
        getStatFile(temp, buffer);
        readProcStatus(buffer, virtualMem, realMem, procMaxMem);

        realMem = realMem << 2;    //each page is 4bytes, so multiply by 4
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
        unsigned long & realMem, 
        unsigned long & procMaxMem
        )
{

    const char *pBuf = buffer.c_str();
    int count = 1;
    int i = 0;

    while(count != 23)
    {
        if(pBuf[i] == ' ')
        {
            count++;
        }
        i++;
    }

    pBuf = pBuf + i;


    sscanf(pBuf, "%lu %lu %lu", &virtualMem, &realMem, &procMaxMem);
}

}
}
