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
This header file contains the definitions of common types and utility classes.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 02/07/2007
*****************************************************************************/

#ifndef __UDT_COMMON_H__
#define __UDT_COMMON_H__


#ifndef WIN32
   #include <sys/time.h>
   #include <sys/uio.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <pthread.h>
#else
   #include <windows.h>
#endif
#include "udt.h"


#ifdef WIN32
   // Windows compability
   typedef HANDLE pthread_t;
   typedef HANDLE pthread_mutex_t;
   typedef HANDLE pthread_cond_t;
   typedef DWORD pthread_key_t;

   struct iovec
   {
      int iov_len;
      char* iov_base;
   };

   int gettimeofday(timeval *tv, void*);
#endif

#ifdef UNIX
   #define usleep(usec) \
   { \
      struct timeval _timeout; \
      _timeout.tv_sec  = 0; \
      _timeout.tv_usec = usec; \
      ::select (0, NULL, NULL, NULL, &_timeout); \
   }
#endif

////////////////////////////////////////////////////////////////////////////////

class CTimer
{
public:

      // Functionality:
      //    Sleep for "interval" CCs.
      // Parameters:
      //    0) [in] interval: CCs to sleep.
      // Returned value:
      //    None.

   void sleep(const uint64_t& interval);

      // Functionality:
      //    Seelp until CC "nexttime".
      // Parameters:
      //    0) [in] nexttime: next time the caller is waken up.
      // Returned value:
      //    None.

   void sleepto(const uint64_t& nexttime);

      // Functionality:
      //    Stop the sleep() or sleepto() methods.
      // Parameters:
      //    None.
      // Returned value:
      //    None.

   void interrupt();

public:

      // Functionality:
      //    Read the CPU clock cycle into x.
      // Parameters:
      //    0) [out] x: to record cpu clock cycles.
      // Returned value:
      //    None.

   static void rdtsc(uint64_t &x);

      // Functionality:
      //    return the CPU frequency.
      // Parameters:
      //    None.
      // Returned value:
      //    CPU frequency.

   static uint64_t getCPUFrequency();

private:
   uint64_t m_ullSchedTime;             // next schedulled time

private:
   static uint64_t s_ullCPUFrequency;	// CPU frequency : clock cycles per microsecond
   static uint64_t readCPUFrequency();
};

////////////////////////////////////////////////////////////////////////////////

class CGuard
{
public:
   CGuard(pthread_mutex_t& lock);
   ~CGuard();

private:
   pthread_mutex_t& m_Mutex;            // Alias name of the mutex to be protected
   int m_iLocked;                       // Locking status

   void operator = (const CGuard&) {}
};

////////////////////////////////////////////////////////////////////////////////

// UDT Sequence Number 0 - (2^31 - 1)

// seqcmp: compare two seq#, considering the wraping
// seqlen: length from the 1st to the 2nd seq#, including both
// seqoff: offset from the 2nd to the 1st seq#
// incseq: increase the seq# by 1
// decseq: decrease the seq# by 1
// incseq: increase the seq# by a given offset

class CSeqNo
{
public:
   inline static const int seqcmp(const int32_t& seq1, const int32_t& seq2)
   {return (abs(seq1 - seq2) < m_iSeqNoTH) ? (seq1 - seq2) : (seq2 - seq1);}

   inline static const int seqlen(const int32_t& seq1, const int32_t& seq2)
   {return (seq1 <= seq2) ? (seq2 - seq1 + 1) : (seq2 - seq1 + 1 + m_iMaxSeqNo);}

   inline static const int seqoff(const int32_t& seq1, const int32_t& seq2)
   {
      if (abs(seq1 - seq2) < m_iSeqNoTH)
         return seq2 - seq1;

      if (seq1 < seq2)
         return seq2 - seq1 - m_iMaxSeqNo;

      return seq2 - seq1 + m_iMaxSeqNo;
   }

   inline static const int32_t incseq(const int32_t seq)
   {return (seq == m_iMaxSeqNo - 1) ? 0 : seq + 1;}

   inline static const int32_t decseq(const int32_t& seq)
   {return (seq == 0) ? m_iMaxSeqNo - 1 : seq - 1;}

   inline static const int32_t incseq(const int32_t& seq, const int32_t& inc)
   {return (m_iMaxSeqNo - seq > inc) ? seq + inc : seq - m_iMaxSeqNo + inc;}

public:
   static const int32_t m_iSeqNoTH;             // threshold for comparing seq. no.
   static const int32_t m_iMaxSeqNo;            // maximum sequence number used in UDT
};

////////////////////////////////////////////////////////////////////////////////

// UDT ACK Sub-sequence Number: 0 - (2^31 - 1)

class CAckNo
{
public:
   inline static const int32_t incack(const int32_t& ackno)
   {return (ackno == m_iMaxAckSeqNo - 1) ? 0 : ackno + 1;}

public:
   static const int32_t m_iMaxAckSeqNo;         // maximum ACK sub-sequence number used in UDT
};

////////////////////////////////////////////////////////////////////////////////

// UDT Message Number: 0 - (2^29 - 1)

class CMsgNo
{
public:
   inline static const int msgcmp(const int32_t& msgno1, const int32_t& msgno2)
   {return (abs(msgno1 - msgno2) < m_iMsgNoTH) ? (msgno1 - msgno2) : (msgno2 - msgno1);}

   inline static const int msglen(const int32_t& msgno1, const int32_t& msgno2)
   {return (msgno1 <= msgno2) ? (msgno2 - msgno1 + 1) : (msgno2 - msgno1 + m_iMaxMsgNo);}

   inline static const int msgoff(const int32_t& msgno1, const int32_t& msgno2)
   {
      if (abs(msgno1 - msgno2) < m_iMsgNoTH)
         return msgno2 - msgno1;

      if (msgno1 < msgno2)
         return msgno2 - msgno1 - m_iMaxMsgNo;

      return msgno2 - msgno1 + m_iMaxMsgNo;
   }

   inline static const int32_t incmsg(const int32_t& msgno)
   {return (msgno == m_iMaxMsgNo - 1) ? 0 : msgno + 1;}

public:
   static const int32_t m_iMsgNoTH;             // threshold for comparing msg. no.
   static const int32_t m_iMaxMsgNo;            // maximum message number used in UDT
};


#endif
