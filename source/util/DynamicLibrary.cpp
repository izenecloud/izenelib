#include <util/DynamicLibrary.h>

#include <stdexcept>
#include <iostream>
#include <cstring> // strerror

namespace izenelib{ namespace util{

HDynamicLib osLoadLibrary( const std::string& sLibraryName, bool bRawName )
{
#ifdef WIN32
    if(bRawName)
      return LoadLibrary(sLibraryName.c_str());
    else
      return LoadLibrary((sLibraryName + IZENE_LIBRARY_EXT).c_str());
#else
    if(bRawName)
        return dlopen(sLibraryName.c_str(), RTLD_NOW | RTLD_GLOBAL);
    else
    {
        std::string::size_type nPos = sLibraryName.find_last_of('/');
        if(nPos == std::string::npos)
            return dlopen((IZENE_LIBRARY_PREFIX + sLibraryName + IZENE_LIBRARY_EXT).c_str(), RTLD_NOW | RTLD_GLOBAL);
        else
            //return dlopen((sLibraryName.substr(nPos) + IZENE_LIBRARY_PREFIX + sLibraryName.substr(nPos + 1) + IZENE_LIBRARY_EXT).c_str(), RTLD_LAZY);		
            return dlopen(sLibraryName.c_str(), RTLD_NOW | RTLD_GLOBAL);
    }
#endif
}

PLibSymbol osGetSymbol( HDynamicLib hDynamicLib, const std::string& sSymName )
{
    return 
#ifdef WIN32
    GetProcAddress(hDynamicLib, sSymName.c_str());
#else
    dlsym(hDynamicLib, sSymName.c_str());
#endif
}

bool osFreeLibrary(HDynamicLib hDynamicLib)
{
    return 
#ifdef WIN32
    FreeLibrary(hDynamicLib) == TRUE;
#else
    dlclose(hDynamicLib);
#endif
}

IZENE_EXPORT std::string osGetErrorStr( int nError )
{
#ifdef WIN32
    std::string sMsg;

    sMsg.resize(1024);

    DWORD dwChars = ::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
              0, nError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
              const_cast<TChar*>(sMsg.c_str()), static_cast<DWORD>(sMsg.size()), NULL );

    if (dwChars == 0)
        return "<Unknown error>";

    sMsg.erase(dwChars);

    return sMsg;
#else
    return strerror(nError);
#endif
}


long osGetLastLibraryError()
{
    return 
#ifdef WIN32
    GetLastError();
#else
    dlerror() == NULL ? -1 : 0;
#endif
}

std::string osGetLastLibraryErrorStr()
{
    return 
#ifdef WIN32
    osGetErrorStr(osGetLastLibraryError());
#else
    dlerror();
#endif
}

//////////////////////////////////////////////////////
//
//                      DynamicLibrary
//
/////////////////////////////////////////////////////

DynamicLibrary::DynamicLibrary(bool unload_when_destroy)
    :hDynLib_(NULL)
    ,unload_when_destroy_(unload_when_destroy)
{
}

DynamicLibrary::~DynamicLibrary()
{
    if(unload_when_destroy_)
    {
        try 
        {
            if ( hDynLib_ )
            {
                unloadLibrary();
            }
        } catch (...) {}
    }
}

void DynamicLibrary::load( const std::string& sLibName, bool bRawName /*= false*/ )
{
    if(hDynLib_) throw std::runtime_error("library has already been initialized");	
    hDynLib_ = osLoadLibrary(sLibName, bRawName);
    if(!hDynLib_) throw std::runtime_error("library load exception:"+osGetLastLibraryErrorStr());	
    name_ = sLibName;
}

const std::string& DynamicLibrary::getName() const
{
    if(!hDynLib_) throw std::runtime_error("library not initialized");
    return name_;
}

PLibSymbol DynamicLibrary::getSymbol(const std::string& sSymName) const
{
    PLibSymbol pSym;
    if(!hDynLib_) throw std::runtime_error("library not initialized");
    pSym = osGetSymbol(hDynLib_, sSymName);
    if(!pSym) throw std::runtime_error("no item named:"+sSymName);
    return pSym;
}

void DynamicLibrary::unloadLibrary()
{
    if(!hDynLib_) throw std::runtime_error("library not initialized");
    osFreeLibrary(hDynLib_);
    hDynLib_ = NULL;
}

}}

