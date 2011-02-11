/*
 *  Copyright 2009 Utkin Dmitry
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
#ifndef _DYNAMICLIBRARY_H_
#define _DYNAMICLIBRARY_H_

#ifdef WIN32
#include <windows.h>

#pragma warning (disable : 4251) // needs to have dll-interface to be used by clients of class rise...

#define IZENE_DLL_EXPORT __declspec(dllexport)
#define IZENE_DLL_IMPORT __declspec(dllimport)

#ifdef IZENE_DLL_EXPORTS
#define IZENE_EXPORT IZENE_DLL_EXPORT
#else
#define IZENE_EXPORT IZENE_DLL_IMPORT
#endif

#define IZENE_LIBRARY_PREFIX ""
#define IZENE_LIBRARY_EXT ".dll"

typedef HMODULE HDynamicLib;

#else
#define IZENE_EXPORT 
#define IZENE_DLL_EXPORT
#define IZENE_DLL_IMPORT

#ifdef OS_Darwin
#define IZENE_LIBRARY_PREFIX "lib"
#define IZENE_LIBRARY_EXT ".dylib"
#else
#define IZENE_LIBRARY_PREFIX "lib"
#define IZENE_LIBRARY_EXT ".so"
#endif

#include <dlfcn.h>
//! dynamic library handle
typedef void* HDynamicLib;
#endif

#include <string>


namespace izenelib{ namespace util {


//! library symbol
typedef void* PLibSymbol;

/* load library
*   param  sLibraryName - library name
*   param  bRawName - use system-specific library name
*   return dynamic library handle, NULL if error
*/
HDynamicLib osLoadLibrary(const std::string& sLibraryName, bool bRawName);

/* get library symbol
*  param  hDynamicLib - dynamic lib handle
*  param  sSymName - symbol name
*  return pointer to symbol, NULL if error
*/
PLibSymbol osGetSymbol(HDynamicLib hDynamicLib, const std::string& sSymName);

/* unload and free library
*  param  hDynamicLib - dynamic lib handle
*  return true, if library was successfully unloaded
*/
bool osFreeLibrary(HDynamicLib hDynamicLib);

/* get error description
* param	nError - error code
* return error description
*/
IZENE_EXPORT std::string osGetErrorStr( int nError );

/* get last error code
*  return last error code
*/
long osGetLastLibraryError();

/* get last error description
*   return last error description
*/
std::string osGetLastLibraryErrorStr();



class IZENE_EXPORT DynamicLibrary
{
public:

    DynamicLibrary(bool unload_when_destroy = true);

    virtual ~DynamicLibrary();
      
    /* load dynamic library
    *   param  sLibName - library name
    *   param  bRawName - true, given full library name with extension
    */
    virtual void load(const std::string& sLibName, bool bRawName = false);

    /* get library name
    *   return library name
    */
    const std::string& getName() const;

    /* get library symbol pointer
    *   param  sSymName - symbol name
    *   return pointer to symbol
    */
    PLibSymbol getSymbol(const std::string& sSymName) const;

    /* unload library
    */
    void unloadLibrary();

private:
    HDynamicLib hDynLib_; //!< library handle
    std::string name_;   //!< library name
    bool unload_when_destroy_; //! <unload library when destroy this class 
};

  }}

#endif // _DYNAMICLIBRARY_H_

