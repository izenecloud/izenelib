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
This file contains the implementation of UDT API.

reference: UDT programming manual and socket programming reference
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 05/15/2007
*****************************************************************************/

#ifndef WIN32
   #include <unistd.h>
#else
   #include <winsock2.h>
   #include <ws2tcpip.h>
#endif
#include "api.h"
#include "core.h"

using namespace std;


CUDTSocket::CUDTSocket():
m_pSelfAddr(NULL),
m_pPeerAddr(NULL),
m_pUDT(NULL),
m_pQueuedSockets(NULL),
m_pAcceptSockets(NULL)
{
   #ifndef WIN32
      pthread_mutex_init(&m_AcceptLock, NULL);
      pthread_cond_init(&m_AcceptCond, NULL);
   #else
      m_AcceptLock = CreateMutex(NULL, false, NULL);
      m_AcceptCond = CreateEvent(NULL, false, false, NULL);
   #endif
}

CUDTSocket::~CUDTSocket()
{
   if (AF_INET == m_iIPversion)
   {
      if (m_pSelfAddr)
         delete (sockaddr_in*)m_pSelfAddr;
      if (m_pPeerAddr)
         delete (sockaddr_in*)m_pPeerAddr;
   }
   else
   {
      if (m_pSelfAddr)
         delete (sockaddr_in6*)m_pSelfAddr;
      if (m_pPeerAddr)
         delete (sockaddr_in6*)m_pPeerAddr;
   }

   if (m_pUDT)
      delete m_pUDT;

   if (m_pQueuedSockets)
      delete m_pQueuedSockets;
   if (m_pAcceptSockets)
      delete m_pAcceptSockets;

   #ifndef WIN32
      pthread_mutex_destroy(&m_AcceptLock);
      pthread_cond_destroy(&m_AcceptCond);
   #else
      CloseHandle(m_AcceptLock);
      CloseHandle(m_AcceptCond);
   #endif
}

////////////////////////////////////////////////////////////////////////////////

CUDTUnited::CUDTUnited():
m_SocketID(1 << 30)
{
   #ifndef WIN32
      pthread_mutex_init(&m_ControlLock, NULL);
      pthread_mutex_init(&m_IDLock, NULL);
   #else
      m_ControlLock = CreateMutex(NULL, false, NULL);
      m_IDLock = CreateMutex(NULL, false, NULL);
   #endif

   #ifndef WIN32
      pthread_key_create(&m_TLSError, TLSDestroy);
   #else
      m_TLSError = TlsAlloc();
   #endif

   // Global initialization code
   #ifdef WIN32
      WORD wVersionRequested;
      WSADATA wsaData;
      wVersionRequested = MAKEWORD(2, 2);

      if (0 != WSAStartup(wVersionRequested, &wsaData))
         throw CUDTException(1, 0,  WSAGetLastError());
   #endif
}

CUDTUnited::~CUDTUnited()
{
   #ifndef WIN32
      pthread_mutex_destroy(&m_ControlLock);
      pthread_mutex_destroy(&m_IDLock);
   #else
      CloseHandle(m_ControlLock);
      CloseHandle(m_IDLock);
   #endif

   #ifndef WIN32
      pthread_key_delete(m_TLSError);
   #else
      TlsFree(m_TLSError);
   #endif

   // Global destruction code
   #ifdef WIN32
      WSACleanup();
   #endif
}

UDTSOCKET CUDTUnited::newSocket(const int& af, const int& type)
{
   // garbage collection before a new socket is created
   checkBrokenSockets();

   if ((type != SOCK_STREAM) && (type != SOCK_DGRAM))
      throw CUDTException(5, 3, 0);

   CUDTSocket* ns = NULL;

   try
   {
      ns = new CUDTSocket;
      ns->m_pUDT = new CUDT;
      if (AF_INET == af)
         ns->m_pSelfAddr = (sockaddr*)(new sockaddr_in);
      else
         ns->m_pSelfAddr = (sockaddr*)(new sockaddr_in6);
   }
   catch (...)
   {
      delete ns;
      throw CUDTException(3, 2, 0);
   }

   #ifndef WIN32
      pthread_mutex_lock(&m_IDLock);
   #else
      WaitForSingleObject(m_IDLock, INFINITE);
   #endif
   ns->m_Socket = -- m_SocketID;
   #ifndef WIN32
      pthread_mutex_unlock(&m_IDLock);
   #else
      ReleaseMutex(m_IDLock);
   #endif

   ns->m_Status = CUDTSocket::INIT;
   ns->m_ListenSocket = 0;
   ns->m_pUDT->m_SocketID = ns->m_Socket;
   ns->m_pUDT->m_iSockType = type;
   ns->m_pUDT->m_iIPversion = ns->m_iIPversion = af;

   // protect the m_Sockets structure.
   #ifndef WIN32
      pthread_mutex_lock(&m_ControlLock);
   #else
      WaitForSingleObject(m_ControlLock, INFINITE);
   #endif
   try
   {
      m_Sockets[ns->m_Socket] = ns;
   }
   catch (...)
   {
      //failure and rollback
      delete ns;
      ns = NULL;
   }
   #ifndef WIN32
      pthread_mutex_unlock(&m_ControlLock);
   #else
      ReleaseMutex(m_ControlLock);
   #endif

   if (NULL == ns)
      throw CUDTException(3, 2, 0);

   return ns->m_Socket;
}

int CUDTUnited::newConnection(const UDTSOCKET listen, const sockaddr* peer, CHandShake* hs)
{
   // garbage collection before a new socket is created
   checkBrokenSockets();

   CUDTSocket* ns;
   CUDTSocket* ls = locate(listen);

   // if this connection has already been processed
   if (NULL != (ns = locate(listen, peer)))
   {
      if (ns->m_pUDT->m_bBroken)
      {
         // last connection from the "peer" address has been broken
         ns->m_Status = CUDTSocket::CLOSED;
         gettimeofday(&ns->m_TimeStamp, 0);

         #ifndef WIN32
            pthread_mutex_lock(&(ls->m_AcceptLock));
         #else
            WaitForSingleObject(ls->m_AcceptLock, INFINITE);
         #endif
         ls->m_pQueuedSockets->erase(ns->m_Socket);
         ls->m_pAcceptSockets->erase(ns->m_Socket);
         #ifndef WIN32
            pthread_mutex_unlock(&(ls->m_AcceptLock));
         #else
            ReleaseMutex(ls->m_AcceptLock);
         #endif
      }
      else if (hs->m_iISN == ns->m_pUDT->m_iPeerISN)
      {
         // connection already exist, this is a repeated connection request
         // respond with existing HS information

         hs->m_iISN = ns->m_pUDT->m_iISN;
         hs->m_iMSS = ns->m_pUDT->m_iMSS;
         hs->m_iFlightFlagSize = ns->m_pUDT->m_iFlightFlagSize;
         hs->m_iReqType = -1;
         if (AF_INET == ls->m_iIPversion)
            hs->m_iPort = ntohs(((sockaddr_in*)ns->m_pSelfAddr)->sin_port);
         else
            hs->m_iPort = ntohs(((sockaddr_in6*)ns->m_pSelfAddr)->sin6_port);

         return 0;

         //except for this situation a new connection should be started
      }
   }

   // exceeding backlog, refuse the connection request
   if (ls->m_pQueuedSockets->size() >= ls->m_uiBackLog)
      return -1;

   try
   {
      ns = new CUDTSocket;
      ns->m_pUDT = new CUDT(*(ls->m_pUDT));
      if (AF_INET == ls->m_iIPversion)
      {
         ns->m_pSelfAddr = (sockaddr*)(new sockaddr_in);
         ns->m_pPeerAddr = (sockaddr*)(new sockaddr_in);
         memcpy(ns->m_pPeerAddr, peer, sizeof(sockaddr_in));
      }
      else
      {
         ns->m_pSelfAddr = (sockaddr*)(new sockaddr_in6);
         ns->m_pPeerAddr = (sockaddr*)(new sockaddr_in6);
         memcpy(ns->m_pPeerAddr, peer, sizeof(sockaddr_in6));
      }
   }
   catch (...)
   {
      delete ns;
      return -1;
   }

   #ifndef WIN32
      pthread_mutex_lock(&m_IDLock);
   #else
      WaitForSingleObject(m_IDLock, INFINITE);
   #endif
   ns->m_Socket = -- m_SocketID;
   #ifndef WIN32
      pthread_mutex_unlock(&m_IDLock);
   #else
      ReleaseMutex(m_IDLock);
   #endif

   ns->m_Status = CUDTSocket::INIT;
   ns->m_ListenSocket = listen;
   ns->m_iIPversion = ls->m_iIPversion;
   ns->m_pUDT->m_SocketID = ns->m_Socket;

   int error = 0;

   try
   {
      ns->m_pUDT->open();
      ns->m_pUDT->connect(peer, hs);
   }
   catch (...)
   {
      error = 1;
      goto ERR_ROLLBACK;
   }

   // copy address information of local node
   ns->m_pUDT->m_pChannel->getSockAddr(ns->m_pSelfAddr);

   // protect the m_Sockets structure.
   #ifndef WIN32
      pthread_mutex_lock(&m_ControlLock);
   #else
      WaitForSingleObject(m_ControlLock, INFINITE);
   #endif
   try
   {
      m_Sockets[ns->m_Socket] = ns;
   }
   catch (...)
   {
      error = 2;
   }
   #ifndef WIN32
      pthread_mutex_unlock(&m_ControlLock);
   #else
      ReleaseMutex(m_ControlLock);
   #endif

   #ifndef WIN32
      pthread_mutex_lock(&(ls->m_AcceptLock));
   #else
      WaitForSingleObject(ls->m_AcceptLock, INFINITE);
   #endif
   try
   {
      ls->m_pQueuedSockets->insert(ns->m_Socket);
   }
   catch (...)
   {
      error = 3;
   }
   #ifndef WIN32
      pthread_mutex_unlock(&(ls->m_AcceptLock));
   #else
      ReleaseMutex(ls->m_AcceptLock);
   #endif

   ERR_ROLLBACK:
   if (error > 0)
   {
      ns->m_pUDT->close();
      if (error > 1)
          m_Sockets.erase(ns->m_Socket);
      delete ns;

      return -1;
   }

   // complete the response HS information with port number
   if (AF_INET == ls->m_iIPversion)
      hs->m_iPort = ntohs(((sockaddr_in*)ns->m_pSelfAddr)->sin_port);
   else
      hs->m_iPort = ntohs(((sockaddr_in6*)ns->m_pSelfAddr)->sin6_port);

   // wake up a waiting accept() call
   #ifndef WIN32
      pthread_cond_signal(&(ls->m_AcceptCond));
   #else
      SetEvent(ls->m_AcceptCond);
   #endif

   return 1;
}

CUDT* CUDTUnited::lookup(const UDTSOCKET u)
{
   // protects the m_Sockets structure
   CGuard cg(m_ControlLock);

   map<UDTSOCKET, CUDTSocket*>::iterator i = m_Sockets.find(u);

   if (i == m_Sockets.end())
      throw CUDTException(5, 4, 0);

   return i->second->m_pUDT;
}

int CUDTUnited::bind(const UDTSOCKET u, const sockaddr* name, const int& namelen)
{
   CUDTSocket* s = locate(u);

   if (NULL == s)
      throw CUDTException(5, 4, 0);

   // check the size of SOCKADDR structure
   if (AF_INET == s->m_iIPversion)
   {
      if (namelen != sizeof(sockaddr_in))
         throw CUDTException(5, 3, 0);
   }
   else
   {
      if (namelen != sizeof(sockaddr_in6))
         throw CUDTException(5, 3, 0);
   }

   // cannot bind a socket more than once
   if (CUDTSocket::INIT != s->m_Status)
      throw CUDTException(5, 0, 0);

   s->m_pUDT->open(name);
   s->m_Status = CUDTSocket::OPENED;

   // copy address information of local node
   s->m_pUDT->m_pChannel->getSockAddr(s->m_pSelfAddr);

   return 0;
}

int CUDTUnited::listen(const UDTSOCKET u, const int& backlog)
{
   CUDTSocket* s = locate(u);

   if (NULL == s)
      throw CUDTException(5, 4, 0);

   // listen is not supported in rendezvous connection setup
   if (s->m_pUDT->m_bRendezvous)
      throw CUDTException(5, 7, 0);

   if (backlog <= 0)
      throw CUDTException(5, 3, 0);

   s->m_uiBackLog = backlog;

   // do nothing if the socket is already listening
   if (CUDTSocket::LISTENING == s->m_Status)
      return 0;

   // a socket can listen only if is in OPENED status
   if (CUDTSocket::OPENED != s->m_Status)
      throw CUDTException(5, 5, 0);

   try
   {
      s->m_pQueuedSockets = new set<UDTSOCKET>;
      s->m_pAcceptSockets = new set<UDTSOCKET>;
   }
   catch (...)
   {
      delete s->m_pQueuedSockets;
      throw CUDTException(3, 2, 0);
   }

   s->m_pUDT->listen();

   s->m_Status = CUDTSocket::LISTENING;

   return 0;
}

UDTSOCKET CUDTUnited::accept(const UDTSOCKET listen, sockaddr* addr, int* addrlen)
{
   CUDTSocket* ls = locate(listen);

   if (ls == NULL)
      throw CUDTException(5, 4, 0);

   // the "listen" socket must be in LISTENING status
   if (CUDTSocket::LISTENING != ls->m_Status)
      throw CUDTException(5, 6, 0);

   // no "accept" in rendezvous connection setup
   if (ls->m_pUDT->m_bRendezvous)
      throw CUDTException(5, 7, 0);


   UDTSOCKET u = CUDT::INVALID_SOCK;
   bool accepted = false;

   // !!only one conection can be set up each time!!
   #ifndef WIN32
      while (!accepted)
      {
         pthread_mutex_lock(&(ls->m_AcceptLock));

         if (ls->m_pQueuedSockets->size() > 0)
         {
            u = *(ls->m_pQueuedSockets->begin());
            ls->m_pAcceptSockets->insert(ls->m_pAcceptSockets->end(), u);
            ls->m_pQueuedSockets->erase(ls->m_pQueuedSockets->begin());

            accepted = true;
         }
         else if (!ls->m_pUDT->m_bSynRecving)
            accepted = true;
         else if (CUDTSocket::LISTENING == ls->m_Status)
            pthread_cond_wait(&(ls->m_AcceptCond), &(ls->m_AcceptLock));

         if (CUDTSocket::LISTENING != ls->m_Status)
            accepted = true;

         pthread_mutex_unlock(&(ls->m_AcceptLock));
      }
   #else
      while (!accepted)
      {
         WaitForSingleObject(ls->m_AcceptLock, INFINITE);

         if (ls->m_pQueuedSockets->size() > 0)
         {
            u = *(ls->m_pQueuedSockets->begin());
            ls->m_pAcceptSockets->insert(ls->m_pAcceptSockets->end(), u);
            ls->m_pQueuedSockets->erase(ls->m_pQueuedSockets->begin());

            accepted = true;
         }
         else if (!ls->m_pUDT->m_bSynRecving)
            accepted = true;

         ReleaseMutex(ls->m_AcceptLock);

         if  (!accepted & (CUDTSocket::LISTENING == ls->m_Status))
            WaitForSingleObject(ls->m_AcceptCond, INFINITE);

         if (CUDTSocket::LISTENING != ls->m_Status)
         {
            SetEvent(ls->m_AcceptCond);
            accepted = true;
         }
      }
   #endif


   if (u == CUDT::INVALID_SOCK)
   {
      // non-blocking receiving, no connection available
      if (!ls->m_pUDT->m_bSynRecving)
         throw CUDTException(6, 2, 0);

      // listening socket is closed
      throw CUDTException(5, 6, 0);
   }

   if (NULL != addr)
   {
      if (NULL == addrlen)
         throw CUDTException(5, 3, 0);

      if (AF_INET == locate(u)->m_iIPversion)
         *addrlen = sizeof(sockaddr_in);
      else
         *addrlen = sizeof(sockaddr_in6);

      // copy address information of peer node
      memcpy(addr, locate(u)->m_pPeerAddr, *addrlen);
   }

   return u;
}

int CUDTUnited::connect(const UDTSOCKET u, const sockaddr* name, const int& namelen)
{
   CUDTSocket* s = locate(u);

   if (NULL == s)
      throw CUDTException(5, 4, 0);

   // check the size of SOCKADDR structure
   if (AF_INET == s->m_iIPversion)
   {
      if (namelen != sizeof(sockaddr_in))
         throw CUDTException(5, 3, 0);
   }
   else
   {
      if (namelen != sizeof(sockaddr_in6))
         throw CUDTException(5, 3, 0);
   }

   // a socket can "connect" only if it is in INIT or OPENED status
   if (CUDTSocket::INIT == s->m_Status)
   {
      if (!s->m_pUDT->m_bRendezvous)
         s->m_pUDT->open();
      else
         throw CUDTException(5, 8, 0);
   }
   else if (CUDTSocket::OPENED != s->m_Status)
      throw CUDTException(5, 2, 0);

   s->m_pUDT->connect(name);
   s->m_Status = CUDTSocket::CONNECTED;

   // copy address information of local node
   s->m_pUDT->m_pChannel->getSockAddr(s->m_pSelfAddr);

   // record peer address
   if (AF_INET == s->m_iIPversion)
      s->m_pPeerAddr = (sockaddr*)(new sockaddr_in);
   else
      s->m_pPeerAddr = (sockaddr*)(new sockaddr_in6);
   s->m_pUDT->m_pChannel->getPeerAddr(s->m_pPeerAddr);

   return 0;
}

int CUDTUnited::close(const UDTSOCKET u)
{
   CUDTSocket* s = locate(u);
   
   if (NULL == s)
      throw CUDTException(5, 4, 0);

   CUDTSocket::UDTSTATUS os = s->m_Status;

   // synchronize with garbage collection.
   #ifndef WIN32
      pthread_mutex_lock(&m_ControlLock);
   #else
      WaitForSingleObject(m_ControlLock, INFINITE);
   #endif
   s->m_Status = CUDTSocket::CLOSED;
   #ifndef WIN32
      pthread_mutex_unlock(&m_ControlLock);
   #else
      ReleaseMutex(m_ControlLock);
   #endif

   // broadcast all "accept" waiting
   if (CUDTSocket::LISTENING == os)
   {
      #ifndef WIN32
         pthread_mutex_lock(&(s->m_AcceptLock));
         pthread_mutex_unlock(&(s->m_AcceptLock));
         pthread_cond_broadcast(&(s->m_AcceptCond));
      #else
         SetEvent(s->m_AcceptCond);
      #endif
   }

   // garbage collection should not try to close this instance since it may block other process due to lingering sending
   s->m_TimeStamp.tv_sec = -1;

   s->m_pUDT->close();

   // a socket will not be immediated removed when it is closed
   // in order to prevent other methods from accessing invalid address
   // a timer is started and the socket will be removed after approximately 1 second
   gettimeofday(&s->m_TimeStamp, 0);

   return 0;
}

int CUDTUnited::getpeername(const UDTSOCKET u, sockaddr* name, int* namelen)
{
   CUDTSocket* s = locate(u);

   if (NULL == s)
      throw CUDTException(5, 4, 0);

   if (!s->m_pUDT->m_bConnected)
      throw CUDTException(2, 2, 0);

   if (AF_INET == s->m_iIPversion)
      *namelen = sizeof(sockaddr_in);
   else
      *namelen = sizeof(sockaddr_in6);

   // copy address information of peer node
   memcpy(name, s->m_pPeerAddr, *namelen);

   return 0;
}

int CUDTUnited::getsockname(const UDTSOCKET u, sockaddr* name, int* namelen)
{
   CUDTSocket* s = locate(u);

   if (NULL == s)
      throw CUDTException(5, 4, 0);

   if (AF_INET == s->m_iIPversion)
      *namelen = sizeof(sockaddr_in);
   else
      *namelen = sizeof(sockaddr_in6);

   // copy address information of local node
   memcpy(name, s->m_pSelfAddr, *namelen);

   return 0;
}

int CUDTUnited::select(ud_set* readfds, ud_set* writefds, ud_set* exceptfds, const timeval* timeout)
{
   timeval entertime, currtime;

   gettimeofday(&entertime, 0);

   int64_t to;
   if (NULL == timeout)
      to = (int64_t)1 << 62;
   else
      to = timeout->tv_sec * 1000000 + timeout->tv_usec;

   int count = 0;

   set<UDTSOCKET> rs, ws, es;

   do
   {
      CUDTSocket* s;

      // query read sockets
      if (NULL != readfds)
         for (set<UDTSOCKET>::iterator i = readfds->begin(); i != readfds->end(); ++ i)
         {
            if (NULL == (s = locate(*i)))
               throw CUDTException(5, 4, 0);

            if ((s->m_pUDT->m_bConnected && (s->m_pUDT->m_pRcvBuffer->getRcvDataSize() > 0))
               || (!s->m_pUDT->m_bListening && (s->m_pUDT->m_bBroken || !s->m_pUDT->m_bConnected))
               || (s->m_pUDT->m_bListening && (s->m_pQueuedSockets->size() > 0)))
            {
               rs.insert(*i);
               ++ count;
            }
         }

      // query write sockets
      if (NULL != writefds)
         for (set<UDTSOCKET>::iterator i = writefds->begin(); i != writefds->end(); ++ i)
         {
            if (NULL == (s = locate(*i)))
               throw CUDTException(5, 4, 0);

            if (s->m_pUDT->m_bConnected && (s->m_pUDT->m_pSndBuffer->getCurrBufSize() < s->m_pUDT->m_iSndQueueLimit))
            {
               ws.insert(*i);
               ++ count;
            }
         }

      // query expections on sockets
      /*
      if (NULL != exceptfds)
         for (set<UDTSOCKET>::iterator i = exceptfds->begin(); i != exceptfds->end(); ++ i)
         {
            if (NULL == (s = locate(*i)))
               throw CUDTException(5, 4, 0);

            // check connection request status
               es.insert(*i);
               ++ count;
         }
      */

      if (0 < count)
         break;

      #ifndef WIN32
         usleep(10);
      #else
         Sleep(1);
      #endif

      gettimeofday(&currtime, 0);

   } while (to > ((currtime.tv_sec - entertime.tv_sec) * 1000000 + currtime.tv_usec - entertime.tv_usec));

   if (NULL != readfds)
      *readfds = rs;

   if (NULL != writefds)
      *writefds = ws;

   if (NULL != exceptfds)
      *exceptfds = es;

   return count;
}

CUDTSocket* CUDTUnited::locate(const UDTSOCKET u)
{
   CGuard cg(m_ControlLock);

   map<UDTSOCKET, CUDTSocket*>::iterator i = m_Sockets.find(u);

   if (i == m_Sockets.end())
      return NULL;
   else
      return i->second;
}

CUDTSocket* CUDTUnited::locate(const UDTSOCKET u, const sockaddr* peer)
{
   CGuard cg(m_ControlLock);

   map<UDTSOCKET, CUDTSocket*>::iterator i = m_Sockets.find(u);

   // look up the "peer" address in queued sockets set
   for (set<UDTSOCKET>::iterator j1 = i->second->m_pQueuedSockets->begin(); j1 != i->second->m_pQueuedSockets->end(); ++ j1)
   {
      map<UDTSOCKET, CUDTSocket*>::iterator k1 = m_Sockets.find(*j1);

      if (AF_INET == i->second->m_iIPversion)
      {
         // compare IPv4 address
         if ((((sockaddr_in*)peer)->sin_port == ((sockaddr_in*)k1->second->m_pPeerAddr)->sin_port) && (((sockaddr_in*)peer)->sin_addr.s_addr == ((sockaddr_in*)k1->second->m_pPeerAddr)->sin_addr.s_addr))
            return k1->second;
      }
      else
      {
         // compare IPv6 address
         if (((sockaddr_in6*)peer)->sin6_port == ((sockaddr_in6*)k1->second->m_pPeerAddr)->sin6_port)
         {
            int* addr1 = (int*)&(((sockaddr_in6*)peer)->sin6_addr);
            int* addr2 = (int*)&(((sockaddr_in6*)k1->second->m_pPeerAddr)->sin6_addr);

            int m1 = 4;
            for (; m1 > 0; -- m1)
               if (addr1[m1] != addr2[m1])
                  break;

            if (m1 > 0)
               return k1->second;
         }
      }
   }

   // look up the "peer" address in accepted sockets
   for (set<UDTSOCKET>::iterator j2 = i->second->m_pAcceptSockets->begin(); j2 != i->second->m_pAcceptSockets->end(); ++ j2)
   {
      map<UDTSOCKET, CUDTSocket*>::iterator k2 = m_Sockets.find(*j2);

      if (AF_INET == i->second->m_iIPversion)
      {
         // compare IPv4 address
         if ((((sockaddr_in*)peer)->sin_port == ((sockaddr_in*)k2->second->m_pPeerAddr)->sin_port) && (((sockaddr_in*)peer)->sin_addr.s_addr == ((sockaddr_in*)k2->second->m_pPeerAddr)->sin_addr.s_addr))
            return k2->second;
      }
      else
      {
         // compare IPv6 address
         if (((sockaddr_in6*)peer)->sin6_port == ((sockaddr_in6*)k2->second->m_pPeerAddr)->sin6_port)
         {
            int* addr1 = (int*)&(((sockaddr_in6*)peer)->sin6_addr);
            int* addr2 = (int*)&(((sockaddr_in6*)k2->second->m_pPeerAddr)->sin6_addr);

            int m2 = 4;
            for (; m2 > 0; -- m2)
               if (addr1[m2] != addr2[m2])
                  break;

            if (m2 > 0)
               return k2->second;
         }
      }
   }

   return NULL;
}

void CUDTUnited::checkBrokenSockets()
{
   CGuard cg(m_ControlLock);

   // set of sockets To Be Removed
   set<UDTSOCKET> tbr;

   for (map<UDTSOCKET, CUDTSocket*>::iterator i = m_Sockets.begin(); i != m_Sockets.end(); ++ i)
   {
      if (CUDTSocket::CLOSED != i->second->m_Status)
      {
         // garbage collection
         if ((i->second->m_pUDT->m_bBroken) && (0 == i->second->m_pUDT->m_pRcvBuffer->getRcvDataSize()))
         {
            //close broken connections and start removal timer
            i->second->m_Status = CUDTSocket::CLOSED;
            gettimeofday(&i->second->m_TimeStamp, 0);

            // remove from listener's queue
            map<UDTSOCKET, CUDTSocket*>::iterator j = m_Sockets.find(i->second->m_ListenSocket);
            if (j != m_Sockets.end())
               j->second->m_pQueuedSockets->erase(i->second->m_Socket);
         }
      }
      else
      {
         // if timeout, delete the socket; 1-2 sec timeout
         timeval currtime;
         gettimeofday(&currtime, 0);
         if ((i->second->m_TimeStamp.tv_sec >= 0) && (currtime.tv_sec - i->second->m_TimeStamp.tv_sec >= 2))
            tbr.insert(i->second->m_Socket);

         // sockets cannot be removed here because it will invalidate the map iterator
      }
   }

   // remove those timeout sockets
   for (set<UDTSOCKET>::iterator k = tbr.begin(); k != tbr.end(); ++ k)
      removeSocket(*k);
}

void CUDTUnited::removeSocket(const UDTSOCKET u)
{
   map<UDTSOCKET, CUDTSocket*>::iterator i = m_Sockets.find(u);

   // invalid socket ID
   if (i == m_Sockets.end())
      return;

   if (0 != i->second->m_ListenSocket)
   {
      // if it is an accepted socket, remove it from the listener's queue
      map<UDTSOCKET, CUDTSocket*>::iterator j = m_Sockets.find(i->second->m_ListenSocket);

      if (j != m_Sockets.end())
         j->second->m_pAcceptSockets->erase(u);
   }
   else if (NULL != i->second->m_pQueuedSockets)
   {
      // if it is a listener, remove all un-accepted sockets in its queue
      for (set<UDTSOCKET>::iterator j = i->second->m_pQueuedSockets->begin(); j != i->second->m_pQueuedSockets->end(); ++ j)
      {
         m_Sockets[*j]->m_pUDT->close();
         delete m_Sockets[*j];
         m_Sockets.erase(*j);
      }
   }

   // delete this one
   m_Sockets[u]->m_pUDT->close();
   delete m_Sockets[u];
   m_Sockets.erase(u);
}

void CUDTUnited::setError(CUDTException* e)
{
   #ifndef WIN32
      delete (CUDTException*)pthread_getspecific(m_TLSError);
      pthread_setspecific(m_TLSError, e);
   #else
      delete (CUDTException*)TlsGetValue(m_TLSError);
      TlsSetValue(m_TLSError, e);
   #endif
}

CUDTException* CUDTUnited::getError()
{
   #ifndef WIN32
      if(NULL == pthread_getspecific(m_TLSError))
         pthread_setspecific(m_TLSError, new CUDTException);
      return (CUDTException*)pthread_getspecific(m_TLSError);
   #else
      if(NULL == TlsGetValue(m_TLSError))
         TlsSetValue(m_TLSError, new CUDTException);
      return (CUDTException*)TlsGetValue(m_TLSError);
   #endif
}

////////////////////////////////////////////////////////////////////////////////

UDTSOCKET CUDT::socket(int af, int type, int)
{
   try
   {
      return s_UDTUnited.newSocket(af, type);
   }
   catch (CUDTException& e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return INVALID_SOCK;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return INVALID_SOCK;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return INVALID_SOCK;
   }
}

int CUDT::bind(UDTSOCKET u, const sockaddr* name, int namelen)
{
   try
   {
      return s_UDTUnited.bind(u, name, namelen);
   }
   catch (CUDTException& e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::listen(UDTSOCKET u, int backlog)
{
   try
   {
      return s_UDTUnited.listen(u, backlog);
   }
   catch (CUDTException& e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

UDTSOCKET CUDT::accept(UDTSOCKET u, sockaddr* addr, int* addrlen)
{
   try
   {
      return s_UDTUnited.accept(u, addr, addrlen);
   }
   catch (CUDTException& e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return INVALID_SOCK;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return INVALID_SOCK;
   }
}

int CUDT::connect(UDTSOCKET u, const sockaddr* name, int namelen)
{
   try
   {
      return s_UDTUnited.connect(u, name, namelen);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::close(UDTSOCKET u)
{
   try
   {
      return s_UDTUnited.close(u);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::getpeername(UDTSOCKET u, sockaddr* name, int* namelen)
{
   try
   {
      return s_UDTUnited.getpeername(u, name, namelen);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::getsockname(UDTSOCKET u, sockaddr* name, int* namelen)
{
   try
   {
      return s_UDTUnited.getsockname(u, name, namelen);;
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::getsockopt(UDTSOCKET u, int, UDTOpt optname, void* optval, int* optlen)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      udt->getOpt(optname, optval, *optlen);

      return 0;
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::setsockopt(UDTSOCKET u, int, UDTOpt optname, const void* optval, int optlen)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      udt->setOpt(optname, optval, optlen);

      return 0;
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::shutdown(UDTSOCKET, int)
{
   try
   {
      //CUDT* udt = s_UDTUnited.lookup(u);
      //udt->shutdown(how);
      return 0;
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::send(UDTSOCKET u, const char* buf, int len, int, int* handle, UDT_MEM_ROUTINE routine, void* context)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      return udt->send((char*)buf, len, handle, routine, context);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::recv(UDTSOCKET u, char* buf, int len, int, int* handle, UDT_MEM_ROUTINE routine, void* context)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      return udt->recv(buf, len, handle, routine, context);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::sendmsg(UDTSOCKET u, const char* buf, int len, int ttl, bool inorder)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      return udt->sendmsg((char*)buf, len, ttl, inorder);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int CUDT::recvmsg(UDTSOCKET u, char* buf, int len)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      return udt->recvmsg(buf, len);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int64_t CUDT::sendfile(UDTSOCKET u, ifstream& ifs, const int64_t& offset, const int64_t& size, const int& block)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      return udt->sendfile(ifs, offset, size, block);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

int64_t CUDT::recvfile(UDTSOCKET u, ofstream& ofs, const int64_t& offset, const int64_t& size, const int& block)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      return udt->recvfile(ofs, offset, size, block);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

bool CUDT::getoverlappedresult(UDTSOCKET u, int handle, int& progress, bool wait)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      return udt->getOverlappedResult(handle, progress, wait);
   }
   catch (CUDTException e)
   {
      // false and -1 means an error; false and positive value means incompleted IO.
      progress = -1;

      s_UDTUnited.setError(new CUDTException(e));
      return false;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return false;
   }
}

int CUDT::select(int, ud_set* readfds, ud_set* writefds, ud_set* exceptfds, const timeval* timeout)
{
   if ((NULL == readfds) && (NULL == writefds) && (NULL == exceptfds))
   {
      s_UDTUnited.setError(new CUDTException(5, 3, 0));
      return ERROR;
   }

   try
   {
      return s_UDTUnited.select(readfds, writefds, exceptfds, timeout);
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (bad_alloc&)
   {
      s_UDTUnited.setError(new CUDTException(3, 2, 0));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

CUDTException& CUDT::getlasterror()
{
   return *s_UDTUnited.getError();
}

int CUDT::perfmon(UDTSOCKET u, CPerfMon* perf, bool clear)
{
   try
   {
      CUDT* udt = s_UDTUnited.lookup(u);

      udt->sample(perf, clear);

      return 0;
   }
   catch (CUDTException e)
   {
      s_UDTUnited.setError(new CUDTException(e));
      return ERROR;
   }
   catch (...)
   {
      s_UDTUnited.setError(new CUDTException(-1, 0, 0));
      return ERROR;
   }
}

bool CUDT::isUSock(UDTSOCKET u)
{
   return (NULL != s_UDTUnited.lookup(u));
}

CUDT* CUDT::getUDTHandle(UDTSOCKET u)
{
   return s_UDTUnited.lookup(u);
}

////////////////////////////////////////////////////////////////////////////////

namespace UDT
{
UDTSOCKET socket(int af, int type, int protocol)
{
   return CUDT::socket(af, type, protocol);
}

int bind(UDTSOCKET u, const struct sockaddr* name, int namelen)
{
   return CUDT::bind(u, name, namelen);
}

int listen(UDTSOCKET u, int backlog)
{
   return CUDT::listen(u, backlog);
}

UDTSOCKET accept(UDTSOCKET u, struct sockaddr* addr, int* addrlen)
{
   return CUDT::accept(u, addr, addrlen);
}

int connect(UDTSOCKET u, const struct sockaddr* name, int namelen)
{
   return CUDT::connect(u, name, namelen);
}

int close(UDTSOCKET u)
{
   return CUDT::close(u);
}

int getpeername(UDTSOCKET u, struct sockaddr* name, int* namelen)
{
   return CUDT::getpeername(u, name, namelen);
}

int getsockname(UDTSOCKET u, struct sockaddr* name, int* namelen)
{
   return CUDT::getsockname(u, name, namelen);
}

int getsockopt(UDTSOCKET u, int level, SOCKOPT optname, void* optval, int* optlen)
{
   return CUDT::getsockopt(u, level, optname, optval, optlen);
}

int setsockopt(UDTSOCKET u, int level, SOCKOPT optname, const void* optval, int optlen)
{
   return CUDT::setsockopt(u, level, optname, optval, optlen);
}

int shutdown(UDTSOCKET u, int how)
{
   return CUDT::shutdown(u, how);
}

int send(UDTSOCKET u, const char* buf, int len, int flags, int* handle, UDT_MEM_ROUTINE routine, void* context)
{
   return CUDT::send(u, buf, len, flags, handle, routine, context);
}

int recv(UDTSOCKET u, char* buf, int len, int flags, int* handle, UDT_MEM_ROUTINE routine, void* context)
{
   return CUDT::recv(u, buf, len, flags, handle, routine, context);
}

int sendmsg(UDTSOCKET u, const char* buf, int len, int ttl, bool inorder)
{
   return CUDT::sendmsg(u, buf, len, ttl, inorder);
}

int recvmsg(UDTSOCKET u, char* buf, int len)
{
   return CUDT::recvmsg(u, buf, len);
}

int64_t sendfile(UDTSOCKET u, ifstream& ifs, const int64_t& offset, const int64_t& size, const int& block)
{
   return CUDT::sendfile(u, ifs, offset, size, block);
}

int64_t recvfile(UDTSOCKET u, ofstream& ofs, const int64_t& offset, const int64_t& size, const int& block)
{
   return CUDT::recvfile(u, ofs, offset, size, block);
}

bool getoverlappedresult(UDTSOCKET u, int handle, int& progress, bool wait)
{
   return CUDT::getoverlappedresult(u, handle, progress, wait);
}

int select(int nfds, UDSET* readfds, UDSET* writefds, UDSET* exceptfds, const struct timeval* timeout)
{
   return CUDT::select(nfds, readfds, writefds, exceptfds, timeout);
}

ERRORINFO getlasterror()
{
   return CUDT::getlasterror();
}

int perfmon(UDTSOCKET u, TRACEINFO* perf, bool clear)
{
   return CUDT::perfmon(u, perf, clear);
}

}
