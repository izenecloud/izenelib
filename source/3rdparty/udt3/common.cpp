/*****************************************************************************
Copyright © 2001 - 2007, The Board of Trustees of the University of Illinois.
All Rights Reserved.

UDP-based Data Transfer Library (UDT) version 3

Laboratory for Advanced Computing (LAC)
National Center for Data Mining (NCDM)
University of Illinois at Chicago
http://www.lac.uic.edu/

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*****************************************************************************/

/*****************************************************************************
This file contains implementation of UDT common routines of timer,
mutex facility, and exception processing.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [ygu@cs.uic.edu], last updated 02/07/2007
*****************************************************************************/


#ifndef WIN32
   #include <unistd.h>
   #include <cstring>
   #include <cstdlib>
   #include <cerrno>
#else
   #include <winsock2.h>
   #include <ws2tcpip.h>
#endif
#include <cmath>
#include "common.h"


#ifdef WIN32
   int gettimeofday(timeval *tv, void*)
   {
      LARGE_INTEGER ccf;
      if (QueryPerformanceFrequency(&ccf))
      {
         LARGE_INTEGER cc;
         QueryPerformanceCounter(&cc);
         tv->tv_sec = (long)(cc.QuadPart / ccf.QuadPart);
         tv->tv_usec = (long)((cc.QuadPart % ccf.QuadPart) / (ccf.QuadPart / 1000000));
      }
      else
      {
         uint64_t ft;
         GetSystemTimeAsFileTime((FILETIME *)&ft);
         tv->tv_sec = (long)(ft / 10000000);
         tv->tv_usec = (long)((ft % 10000000) / 10);
      }

      return 0;
   }
#endif


uint64_t CTimer::s_ullCPUFrequency = CTimer::readCPUFrequency();

void CTimer::rdtsc(uint64_t &x)
{
   #ifdef WIN32
      if (!QueryPerformanceCounter((LARGE_INTEGER *)&x))
      {
         timeval t;
         gettimeofday(&t, 0);
         x = t.tv_sec * 1000000 + t.tv_usec;
      }
   #elif IA32
      // read CPU clock with RDTSC instruction on IA32 acrh
      unsigned int lval, hval;
      __asm__ volatile ("rdtsc" : "=a" (lval), "=d" (hval));
      x = hval;
      x = (x << 32) | lval;

      // on Windows
      /*
         unsigned int a, b;
         __asm 
         {
            rdtsc
            mov a, eax
            mov b, ebx
         }
         x = b;
         x = (x << 32) + a;
      */

   #elif IA64
      __asm__ volatile ("mov %0=ar.itc" : "=r"(x) :: "memory");
   #elif AMD64
      unsigned int lval, hval;
      __asm__ volatile ("rdtsc" : "=a" (lval), "=d" (hval));
      x = hval;
      x = (x << 32) | lval;
   #else
      // use system call to read time clock for other archs
      timeval t;
      gettimeofday(&t, 0);
      x = (uint64_t)t.tv_sec * (uint64_t)1000000 + (uint64_t)t.tv_usec;
   #endif
}

uint64_t CTimer::readCPUFrequency()
{
   #ifdef WIN32
      int64_t ccf;
      if (QueryPerformanceFrequency((LARGE_INTEGER *)&ccf))
         return ccf / 1000000;
      else
         return 1;
   #elif IA32 || IA64 || AMD64
      // alternative: read /proc/cpuinfo

      uint64_t t1, t2;

      rdtsc(t1);
      usleep(100000);
      rdtsc(t2);

      // CPU clocks per microsecond
      return (t2 - t1) / 100000;
   #else
      return 1;
   #endif
}

uint64_t CTimer::getCPUFrequency()
{
   return s_ullCPUFrequency;
}

void CTimer::sleep(const uint64_t& interval)
{
   uint64_t t;
   rdtsc(t);

   // sleep next "interval" time
   sleepto(t + interval);
}

void CTimer::sleepto(const uint64_t& nexttime)
{
   // Use class member such that the method can be interrupted by others
   m_ullSchedTime = nexttime;

   uint64_t t;
   rdtsc(t);

   while (t < m_ullSchedTime)
   {
      #ifdef IA32
         //__asm__ volatile ("nop; nop; nop; nop; nop;");
         __asm__ volatile ("pause; rep; nop; nop; nop; nop; nop;");
      #elif IA64
         __asm__ volatile ("nop 0; nop 0; nop 0; nop 0; nop 0;");
      #elif AMD64
         __asm__ volatile ("nop; nop; nop; nop; nop;");
      #endif

      // TODO: use high precision timer if it is available

      rdtsc(t);
   }
}

void CTimer::interrupt()
{
   // schedule the sleepto time to the current CCs, so that it will stop
   rdtsc(m_ullSchedTime);
}

//
// Automatically lock in constructor
CGuard::CGuard(pthread_mutex_t& lock):
m_Mutex(lock)
{
   #ifndef WIN32
      m_iLocked = pthread_mutex_lock(&m_Mutex);
   #else
      m_iLocked = WaitForSingleObject(m_Mutex, INFINITE);
   #endif
}

// Automatically unlock in destructor
CGuard::~CGuard()
{
   #ifndef WIN32
      if (0 == m_iLocked)
         pthread_mutex_unlock(&m_Mutex);
   #else
      if (WAIT_FAILED != m_iLocked)
         ReleaseMutex(m_Mutex);
   #endif
}

//
CUDTException::CUDTException(int major, int minor, int err):
m_iMajor(major),
m_iMinor(minor)
{
   if (-1 == err)
      #ifndef WIN32
         m_iErrno = errno;
      #else
         m_iErrno = GetLastError();
      #endif
   else
      m_iErrno = err;
}

CUDTException::CUDTException(const CUDTException& e):
m_iMajor(e.m_iMajor),
m_iMinor(e.m_iMinor),
m_iErrno(e.m_iErrno)
{
}

CUDTException::~CUDTException()
{
}

const char* CUDTException::getErrorMessage()
{
   // translate "Major:Minor" code into text message.

   switch (m_iMajor)
   {
      case 0:
        strcpy(m_pcMsg, "Success");
        break;

      case 1:
        strcpy(m_pcMsg, "Connection setup failure");

        switch (m_iMinor)
        {
        case 1:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "connection time out");

           break;

        case 2:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "connection rejected");

           break;

        case 3:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "unable to create/configure UDP socket");

           break;

        case 4:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "abort for security reasons");

           break;

        default:
           break;
        }

        break;

      case 2:
        switch (m_iMinor)
        {
        case 1:
           strcpy(m_pcMsg, "Connection was broken");

           break;

        case 2:
           strcpy(m_pcMsg, "Connection does not exist");

           break;

        default:
           break;
        }

        break;

      case 3:
        strcpy(m_pcMsg, "System resource failure");

        switch (m_iMinor)
        {
        case 1:
           strcpy(m_pcMsg, "unable to create new threads");

           break;

        case 2:
           strcpy(m_pcMsg, "unable to allocate buffers");

           break;

        default:
           break;
        }

        break;

      case 4:
        strcpy(m_pcMsg, "File system failure");

        switch (m_iMinor)
        {
        case 1:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "cannot seek read position");

           break;

        case 2:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "failure in read");

           break;

        case 3:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "cannot seek write position");

           break;

        case 4:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "failure in write");

           break;

        default:
           break;
        }

        break;

      case 5:
        strcpy(m_pcMsg, "Operation not supported");
 
        switch (m_iMinor)
        {
        case 1:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Cannot do this operation on a BOUND socket");

           break;

        case 2:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Cannot do this operation on a CONNECTED socket");

           break;

        case 3:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Bad parameters");

           break;

        case 4:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Invalid socket ID");

           break;

        case 5:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Cannot do this operation on an UNBOUND socket");

           break;

        case 6:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Socket is not in listening state");

           break;

        case 7:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Listen/accept is not supported in rendezous connection setup");

           break;

        case 8:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "Cannot call connect on UNBOUND socket in rendezvous connection setup");

           break;

        case 9:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "This operation is not supported in SOCK_STREAM mode");

           break;

        case 10:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "This operation is not supported in SOCK_DGRAM mode");

           break;

        default:
           break;
        }

        break;

     case 6:
        strcpy(m_pcMsg, "Non-blocking call failure");

        switch (m_iMinor)
        {
        case 1:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "no buffer available for sending");

           break;

        case 2:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "no data available for reading");

           break;

        case 3:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "no buffer available for overlapped reading");

           break;

        case 4:
           strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
           strcpy(m_pcMsg + strlen(m_pcMsg), "non-blocking overlapped recv is on going");

           break;

        default:
           break;
        }

        break;

      default:
        strcpy(m_pcMsg, "Unknown error");
   }

   // Adding "errno" information
   if (0 < m_iErrno)
   {
      strcpy(m_pcMsg + strlen(m_pcMsg), ": ");
      #ifndef WIN32
         strncpy(m_pcMsg + strlen(m_pcMsg), strerror(m_iErrno), 1024 - strlen(m_pcMsg) - 2);
      #else
         LPVOID lpMsgBuf;
         FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, m_iErrno, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
         strncpy(m_pcMsg + strlen(m_pcMsg), (char*)lpMsgBuf, 1024 - strlen(m_pcMsg) - 2);
         LocalFree(lpMsgBuf);
      #endif
   }

   // period
   #ifndef WIN32
      strcpy(m_pcMsg + strlen(m_pcMsg), ".");
   #endif

   return m_pcMsg;
}

const int CUDTException::getErrorCode() const
{
   return m_iMajor * 1000 + m_iMinor;
}
