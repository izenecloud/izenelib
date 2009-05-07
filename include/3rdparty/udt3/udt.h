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
This is the (only) header file of the UDT API, needed for programming with UDT.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 05/15/2007
*****************************************************************************/

#ifndef _UDT_H_
#define _UDT_H_


#ifndef WIN32
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
#else
   #include <windows.h>
#endif
#include <fstream>
#include <set>


////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
   // Explicitly define 32-bit and 64-bit numbers
   typedef __int32 int32_t;
   typedef __int64 int64_t;
   typedef unsigned __int32 uint32_t;
   #if _MSC_VER >= 1300
      typedef unsigned __int64 uint64_t;
   #else
      // VC 6.0 does not support unsigned __int64: may bring potential problems.
      typedef __int64 uint64_t;
   #endif

   #ifdef UDT_EXPORTS
      #define UDT_API __declspec(dllexport)
   #else
      #define UDT_API __declspec(dllimport)
   #endif
#else
   #define UDT_API
#endif

typedef void (*UDT_MEM_ROUTINE)(char*, int, void*);

typedef int UDTSOCKET;

typedef std::set<UDTSOCKET> ud_set;
#define UD_CLR(u, uset) ((uset)->erase(u))
#define UD_ISSET(u, uset) ((uset)->find(u) != (uset)->end())
#define UD_SET(u, uset) ((uset)->insert(u))
#define UD_ZERO(uset) ((uset)->clear())

////////////////////////////////////////////////////////////////////////////////

enum UDTOpt
{
   UDT_MSS,             // the Maximum Transfer Unit
   UDT_SNDSYN,          // if sending is blocking
   UDT_RCVSYN,          // if receiving is blocking
   UDT_CC,              // custom congestion control algorithm
   UDT_FC,              // deprecated, for compatibility only
   UDT_SNDBUF,          // maximum buffer in sending queue
   UDT_RCVBUF,          // UDT receiving buffer size
   UDT_LINGER,          // waiting for unsent data when closing
   UDP_SNDBUF,          // UDP sending buffer size
   UDP_RCVBUF,          // UDP receiving buffer size
   UDT_MAXMSG,          // maximum datagram message size
   UDT_MSGTTL,          // time-to-live of a datagram message
   UDT_RENDEZVOUS,      // rendezvous connection mode
   UDT_SNDTIMEO,        // send() timeout
   UDT_RCVTIMEO	        // recv() timeout
};

////////////////////////////////////////////////////////////////////////////////

struct UDT_API CPerfMon
{
   // global measurements
   int64_t msTimeStamp;                 // time since the UDT entity is started, in milliseconds
   int64_t pktSentTotal;                // total number of sent data packets, including retransmissions
   int64_t pktRecvTotal;                // total number of received packets
   int pktSndLossTotal;                 // total number of lost packets (sender side)
   int pktRcvLossTotal;                 // total number of lost packets (receiver side)
   int pktRetransTotal;                 // total number of retransmitted packets
   int pktSentACKTotal;                 // total number of sent ACK packets
   int pktRecvACKTotal;                 // total number of received ACK packets
   int pktSentNAKTotal;                 // total number of sent NAK packets
   int pktRecvNAKTotal;                 // total number of received NAK packets

   // local measurements
   int64_t pktSent;                     // number of sent data packets, including retransmissions
   int64_t pktRecv;                     // number of received packets
   int pktSndLoss;                      // number of lost packets (sender side)
   int pktRcvLoss;                      // number of lost packets (receiverer side)
   int pktRetrans;                      // number of retransmitted packets
   int pktSentACK;                      // number of sent ACK packets
   int pktRecvACK;                      // number of received ACK packets
   int pktSentNAK;                      // number of sent NAK packets
   int pktRecvNAK;                      // number of received NAK packets
   double mbpsSendRate;                 // sending rate in Mb/s
   double mbpsRecvRate;                 // receiving rate in Mb/s

   // instant measurements
   double usPktSndPeriod;               // packet sending period, in microseconds
   int pktFlowWindow;                   // flow window size, in number of packets
   int pktCongestionWindow;             // congestion window size, in number of packets
   int pktFlightSize;                   // number of packets on flight
   double msRTT;                        // RTT, in milliseconds
   double mbpsBandwidth;                // estimated bandwidth, in Mb/s
   int byteAvailSndBuf;                 // available UDT sender buffer size
   int byteAvailRcvBuf;                 // available UDT receiver buffer size
};

////////////////////////////////////////////////////////////////////////////////

class UDT_API CUDTException
{
public:
   CUDTException(int major = 0, int minor = 0, int err = -1);
   CUDTException(const CUDTException& e);
   virtual ~CUDTException();

      // Functionality:
      //    Get the description of the exception.
      // Parameters:
      //    None.
      // Returned value:
      //    Text message for the exception description.

   virtual const char* getErrorMessage();

      // Functionality:
      //    Get the system errno for the exception.
      // Parameters:
      //    None.
      // Returned value:
      //    errno.

   virtual const int getErrorCode() const;

private:
   int m_iMajor;        // major exception categories

// 0: correct condition
// 1: network setup exception
// 2: network connection broken
// 3: memory exception
// 4: file exception
// 5: method not supported
// 6+: undefined error

   int m_iMinor;        // for specific error reasons

   int m_iErrno;        // errno returned by the system if there is any

   char m_pcMsg[1024];  // text error message
};

////////////////////////////////////////////////////////////////////////////////

namespace UDT
{
typedef CUDTException ERRORINFO;
typedef UDTOpt SOCKOPT;
typedef CPerfMon TRACEINFO;
typedef ud_set UDSET;

UDT_API extern const UDTSOCKET INVALID_SOCK;
#undef ERROR
UDT_API extern const int ERROR;

UDT_API UDTSOCKET socket(int af, int type, int protocol);

UDT_API int bind(UDTSOCKET u, const struct sockaddr* name, int namelen);

UDT_API int listen(UDTSOCKET u, int backlog);

UDT_API UDTSOCKET accept(UDTSOCKET u, struct sockaddr* addr, int* addrlen);

UDT_API int connect(UDTSOCKET u, const struct sockaddr* name, int namelen);

UDT_API int close(UDTSOCKET u);

UDT_API int getpeername(UDTSOCKET u, struct sockaddr* name, int* namelen);

UDT_API int getsockname(UDTSOCKET u, struct sockaddr* name, int* namelen);

UDT_API int getsockopt(UDTSOCKET u, int level, SOCKOPT optname, void* optval, int* optlen);

UDT_API int setsockopt(UDTSOCKET u, int level, SOCKOPT optname, const void* optval, int optlen);

UDT_API int shutdown(UDTSOCKET u, int how);

UDT_API int send(UDTSOCKET u, const char* buf, int len, int flags = 0, int* handle = NULL, UDT_MEM_ROUTINE routine = NULL, void* context = NULL);

UDT_API int recv(UDTSOCKET u, char* buf, int len, int flags = 0, int* handle = NULL, UDT_MEM_ROUTINE routine = NULL, void* context = NULL);

UDT_API int sendmsg(UDTSOCKET u, const char* buf, int len, int ttl = -1, bool inorder = false);

UDT_API int recvmsg(UDTSOCKET u, char* buf, int len);

UDT_API int64_t sendfile(UDTSOCKET u, std::ifstream& ifs, const int64_t& offset, const int64_t& size, const int& block = 366000);

UDT_API int64_t recvfile(UDTSOCKET u, std::ofstream& ofs, const int64_t& offset, const int64_t& size, const int& block = 7320000);

UDT_API bool getoverlappedresult(UDTSOCKET u, int handle, int& progress, bool wait = false);

UDT_API int select(int nfds, UDSET* readfds, UDSET* writefds, UDSET* exceptfds, const struct timeval* timeout);

UDT_API ERRORINFO getlasterror();

UDT_API int perfmon(UDTSOCKET u, TRACEINFO* perf, bool clear = true);
}


#endif
