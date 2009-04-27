#ifndef __febird_config_h__
#define __febird_config_h__

#if defined(_MSC_VER)

# pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

#  if defined(FEBIRD_CREATE_DLL)
#    pragma warning(disable: 4251)
#    define FEBIRD_DLL_EXPORT __declspec(dllexport)      // creator of dll
#  elif defined(FEBIRD_USE_DLL)
#    pragma warning(disable: 4251)
#    define FEBIRD_DLL_EXPORT __declspec(dllimport)      // user of dll
#    ifdef _DEBUG
#	   pragma comment(lib, "febird-d.lib")
#    else
#	   pragma comment(lib, "febird.lib")
#    endif
#  else
#    define FEBIRD_DLL_EXPORT                            // static lib creator or user
#  endif

#else /* _MSC_VER */

#  define FEBIRD_DLL_EXPORT

#endif /* _MSC_VER */

//////////////////////////////////////////////////////////////////////////
// for FEBIRD_RESTRICT keyword

#if defined(__cplusplus)

#  if defined(_MSC_VER)
#    define FEBIRD_RESTRICT __restrict
#  elif defined(__GNUC__)
#    if defined(__GNUC_MINOR__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#      define FEBIRD_RESTRICT __restrict__
#    else
#      define FEBIRD_RESTRICT
#    endif
#  else
#    define FEBIRD_RESTRICT
#  endif

#endif // __cplusplus

#endif // __febird_config_h__
