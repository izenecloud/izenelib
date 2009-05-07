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
This file contains the implementation of main algorithms of UDT protocol and
the implementation of core UDT interfaces.

Reference:
UDT programming manual
UDT protocol specification (draft-gg-udt-xx.txt)
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 06/28/2007
*****************************************************************************/

#ifndef WIN32
   #include <unistd.h>
   #include <netdb.h>
   #include <arpa/inet.h>
   #include <cerrno>
   #include <cstring>
   #include <cstdlib>
#else
   #include <winsock2.h>
   #include <ws2tcpip.h>
#endif
#include <cmath>
#include "core.h"

using namespace std;


CUDTUnited CUDT::s_UDTUnited;

const UDTSOCKET CUDT::INVALID_SOCK = -1;
const int CUDT::ERROR = -1;

const UDTSOCKET UDT::INVALID_SOCK = CUDT::INVALID_SOCK;
const int UDT::ERROR = CUDT::ERROR;

const int32_t CSeqNo::m_iSeqNoTH = 0x3FFFFFFF;
const int32_t CSeqNo::m_iMaxSeqNo = 0x7FFFFFFF;
const int32_t CAckNo::m_iMaxAckSeqNo = 0x7FFFFFFF;
const int32_t CMsgNo::m_iMsgNoTH = 0xFFFFFFF;
const int32_t CMsgNo::m_iMaxMsgNo = 0x1FFFFFFF;


CUDT::CUDT():
//
// These constants are defined in UDT specification. They MUST NOT be changed!
//
m_iVersion(3),
m_iSYNInterval(10000),
m_iSelfClockInterval(64),
m_iQuickStartPkts(16)
{
   m_pChannel = NULL;
   m_pSndBuffer = NULL;
   m_pRcvBuffer = NULL;
   m_pSndLossList = NULL;
   m_pRcvLossList = NULL;
   m_pTimer = NULL;
   m_pIrrPktList = NULL;
   m_pACKWindow = NULL;
   m_pSndTimeWindow = NULL;
   m_pRcvTimeWindow = NULL;

   // Initilize mutex and condition variables
   initSynch();

   // Default UDT configurations
   m_iMSS = 1500;
   m_bSynSending = true;
   m_bSynRecving = true;
   m_iFlightFlagSize = 25600;
   m_iSndQueueLimit = 20000000;
   m_iUDTBufSize = 20000000;
   m_Linger.l_onoff = 1;
   m_Linger.l_linger = 180;
   m_iUDPSndBufSize = 65536;
   m_iUDPRcvBufSize = 10000000;
   m_iMaxMsg = 9000;
   m_iMsgTTL = -1;
   m_iIPversion = AF_INET;
   m_bRendezvous = false;
   m_iSndTimeOut = -1;
   m_iRcvTimeOut = -1;

   #ifdef CUSTOM_CC
      m_pCCFactory = new CCCFactory<CCC>;
   #else
      m_pCCFactory = NULL;
   #endif
   m_pCC = NULL;

   m_iRTT = 10 * m_iSYNInterval;
   m_iRTTVar = m_iRTT >> 1;
   m_ullCPUFrequency = CTimer::getCPUFrequency();

   // Initial status
   m_bOpened = false;
   m_bConnected = false;
   m_bBroken = false;

   m_pcTmpBuf = NULL;
}

CUDT::CUDT(const CUDT& ancestor):
m_iVersion(ancestor.m_iVersion),
m_iSYNInterval(ancestor.m_iSYNInterval),
m_iSelfClockInterval(ancestor.m_iSelfClockInterval),
m_iQuickStartPkts(ancestor.m_iQuickStartPkts)
{
   m_pChannel = NULL;
   m_pSndBuffer = NULL;
   m_pRcvBuffer = NULL;
   m_pSndLossList = NULL;
   m_pRcvLossList = NULL;
   m_pTimer = NULL;
   m_pIrrPktList = NULL;
   m_pACKWindow = NULL;
   m_pSndTimeWindow = NULL;
   m_pRcvTimeWindow = NULL;

   // Initilize mutex and condition variables
   initSynch();

   // Default UDT configurations
   m_iMSS = ancestor.m_iMSS;
   m_bSynSending = ancestor.m_bSynSending;
   m_bSynRecving = ancestor.m_bSynRecving;
   m_iFlightFlagSize = ancestor.m_iFlightFlagSize;
   m_iSndQueueLimit = ancestor.m_iSndQueueLimit;
   m_iUDTBufSize = ancestor.m_iUDTBufSize;
   m_Linger = ancestor.m_Linger;
   m_iUDPSndBufSize = ancestor.m_iUDPSndBufSize;
   m_iUDPRcvBufSize = ancestor.m_iUDPRcvBufSize;
   m_iMaxMsg = ancestor.m_iMaxMsg;
   m_iMsgTTL = ancestor.m_iMsgTTL;
   m_iSockType = ancestor.m_iSockType;
   m_iIPversion = ancestor.m_iIPversion;
   m_bRendezvous = ancestor.m_bRendezvous;
   m_iSndTimeOut = ancestor.m_iSndTimeOut;
   m_iRcvTimeOut = ancestor.m_iRcvTimeOut;

   #ifdef CUSTOM_CC
      m_pCCFactory = ancestor.m_pCCFactory->clone();
   #else
      m_pCCFactory = NULL;
   #endif
   m_pCC = NULL;

   m_iRTT = ancestor.m_iRTT;
   m_iRTTVar = ancestor.m_iRTTVar;
   m_ullCPUFrequency = ancestor.m_ullCPUFrequency;

   // Initial status
   m_bOpened = false;
   m_bConnected = false;
   m_bBroken = false;

   m_pcTmpBuf = NULL;
}

CUDT::~CUDT()
{
   // release mutex/condtion variables
   destroySynch();

   // destroy the data structures
   if (m_pChannel)
      delete m_pChannel;
   if (m_pSndBuffer)
      delete m_pSndBuffer;
   if (m_pRcvBuffer)
      delete m_pRcvBuffer;
   if (m_pSndLossList)
      delete m_pSndLossList;
   if (m_pRcvLossList)
      delete m_pRcvLossList;
   if (m_pTimer)
      delete m_pTimer;
   if (m_pIrrPktList)
      delete m_pIrrPktList;
   if (m_pACKWindow)
      delete m_pACKWindow;
   if (m_pSndTimeWindow)
      delete m_pSndTimeWindow;
   if (m_pRcvTimeWindow)
      delete m_pRcvTimeWindow;
   if (m_pCCFactory)
      delete m_pCCFactory;
   if (m_pCC)
      delete m_pCC;
   if (m_pcTmpBuf)
      delete [] m_pcTmpBuf;
}

void CUDT::setOpt(UDTOpt optName, const void* optval, const int&)
{
   CGuard cg(m_ConnectionLock);
   CGuard sendguard(m_SendLock);
   CGuard recvguard(m_RecvLock);

   switch (optName)
   {
   case UDT_MSS:
      if (m_bOpened)
         throw CUDTException(5, 1, 0);

      if (*(int*)optval < 28)
         throw CUDTException(5, 3, 0);

      m_iMSS = *(int*)optval;

      break;

   case UDT_SNDSYN:
      m_bSynSending = *(bool *)optval;
      break;

   case UDT_RCVSYN:
      m_bSynRecving = *(bool *)optval;
      break;

   case UDT_CC:
      #ifndef CUSTOM_CC
         throw CUDTException(5, 0, 0);
      #else
         if (m_bOpened)
            throw CUDTException(5, 1, 0);
         if (NULL != m_pCCFactory)
            delete m_pCCFactory;
         m_pCCFactory = ((CCCVirtualFactory *)optval)->clone();
      #endif

      break;

   case UDT_FC:
      if (m_bConnected)
         throw CUDTException(5, 2, 0);

      if (*(int*)optval <= 0)
         throw CUDTException(5, 3);
      m_iFlightFlagSize = *(int*)optval;

      break;

   case UDT_SNDBUF:
      if (m_bOpened)
         throw CUDTException(5, 1, 0);

      if (*(int*)optval <= 0)
         throw CUDTException(5, 3, 0);
      m_iSndQueueLimit = *(int*)optval;

      break;

   case UDT_RCVBUF:
      if (m_bOpened)
         throw CUDTException(5, 1, 0);

      if (*(int*)optval <= 0)
         throw CUDTException(5, 3, 0);

      if (*(int*)optval > (m_iMSS - 28) * 32)
         m_iUDTBufSize = *(int*)optval;
      else
         m_iUDTBufSize = (m_iMSS - 28) * 32;

      break;

   case UDT_LINGER:
      m_Linger = *(linger*)optval;
      break;

   case UDP_SNDBUF:
      if (m_bOpened)
         throw CUDTException(5, 1, 0);

      m_iUDPSndBufSize = *(int*)optval;

      break;

   case UDP_RCVBUF:
      if (m_bOpened)
         throw CUDTException(5, 1, 0);

      m_iUDPRcvBufSize = *(int*)optval;
      break;

   case UDT_MAXMSG:
      if (m_bOpened)
         throw CUDTException(5, 1, 0);

      m_iMaxMsg = *(int*)optval;

      break;

   case UDT_MSGTTL:
      if (m_bOpened)
         throw CUDTException(5, 1, 0);

      m_iMsgTTL = *(int*)optval;

      break;

   case UDT_RENDEZVOUS:
      if (m_bConnected)
         throw CUDTException(5, 1, 0);

      m_bRendezvous = *(bool *)optval;

      break;

   case UDT_SNDTIMEO: 
      m_iSndTimeOut = *(int*)optval; 
      break; 

   case UDT_RCVTIMEO: 
      m_iRcvTimeOut = *(int*)optval; 
      break; 

   default:
      throw CUDTException(5, 0, 0);
   }
}

void CUDT::getOpt(UDTOpt optName, void* optval, int& optlen)
{
   CGuard cg(m_ConnectionLock);

   switch (optName)
   {
   case UDT_MSS:
      *(int*)optval = m_iMSS;
      optlen = sizeof(int);
      break;

   case UDT_SNDSYN:
      *(bool*)optval = m_bSynSending;
      optlen = sizeof(bool);
      break;

   case UDT_RCVSYN:
      *(bool*)optval = m_bSynRecving;
      optlen = sizeof(bool);
      break;

   case UDT_CC:
      #ifndef CUSTOM_CC
         throw CUDTException(5, 0, 0);
      #else
         if (!m_bOpened)
            throw CUDTException(5, 5, 0);
         *(CCC**)optval = m_pCC;
         optlen = sizeof(CCC*);
      #endif

      break;

   case UDT_FC:
      *(int*)optval = m_iFlightFlagSize;
      optlen = sizeof(int);
      break;

   case UDT_SNDBUF:
      *(int*)optval = m_iSndQueueLimit;
      optlen = sizeof(int);
      break;

   case UDT_RCVBUF:
      *(int*)optval = m_iUDTBufSize;
      optlen = sizeof(int);
      break;

   case UDT_LINGER:
      if (optlen < (int)(sizeof(linger)))
         throw CUDTException(5, 3, 0);

      *(linger*)optval = m_Linger;
      optlen = sizeof(linger);
      break;

   case UDP_SNDBUF:
      *(int*)optval = m_iUDPSndBufSize;
      optlen = sizeof(int);
      break;

   case UDP_RCVBUF:
      *(int*)optval = m_iUDPRcvBufSize;
      optlen = sizeof(int);
      break;

   case UDT_MAXMSG:
      *(int*)optval = m_iMaxMsg;
      optlen = sizeof(int);
      break;

   case UDT_MSGTTL:
      *(int*)optval = m_iMsgTTL;
      optlen = sizeof(int);
      break;

   case UDT_RENDEZVOUS:
      *(bool *)optval = m_bRendezvous;
      optlen = sizeof(bool);
      break;

   case UDT_SNDTIMEO: 
      *(int*)optval = m_iSndTimeOut; 
      optlen = sizeof(int); 
      break; 
    
   case UDT_RCVTIMEO: 
      *(int*)optval = m_iRcvTimeOut; 
      optlen = sizeof(int); 
      break; 

   default:
      throw CUDTException(5, 0, 0);
   }
}

void CUDT::open(const sockaddr* addr)
{
   CGuard cg(m_ConnectionLock);

   // Initial status
   m_bClosing = false;
   m_bShutdown = false;
   m_bListening = false;
   m_iEXPCount = 1;

   // Initial sequence number, loss, acknowledgement, etc.
   m_iPktSize = m_iMSS - 28;
   m_iPayloadSize = m_iPktSize - CPacket::m_iPktHdrSize;
   m_iISN = 0;
   m_iPeerISN = 0;
 
   m_bLoss = false;
   gettimeofday(&m_LastSYNTime, 0);

   m_iSndLastAck = 0;
   m_iSndLastDataAck = 0;
   m_iSndCurrSeqNo = -1;

   m_iRcvLastAck = 0;
   m_iRcvLastAckAck = 0;
   m_ullLastAckTime = 0;
   m_iRcvCurrSeqNo = -1;
   m_iNextExpect = 0;
   m_bReadBuf = false;

   m_iLastDecSeq = -1;
   m_iNAKCount = 0;
   m_iDecRandom = 1;
   m_iAvgNAKNum = 1;
   m_iDecCount = 0;

   m_iBandwidth = 1;
   m_bSndSlowStart = true;
   m_bRcvSlowStart = true;
   m_bFreeze = false;

   m_iAckSeqNo = 0;

   m_iSndHandle = (1 << 30);
   m_iRcvHandle = -(1 << 30);

   // Initial sending rate = 1us
   m_ullInterval = m_ullCPUFrequency;
   m_ullTimeDiff = 0;
   m_ullLastDecRate = m_ullCPUFrequency;

   // default congestion window size = infinite
   m_dCongestionWindow = 1 << 30;

   // Initial Window Size = 16 packets
   m_iFlowWindowSize = 16;
   m_iFlowControlWindow = 16;
   m_iMaxFlowWindowSize = m_iFlightFlagSize;

   #ifdef CUSTOM_CC
      m_pCC = m_pCCFactory->create();
      m_pCC->m_UDT = m_SocketID;
      m_ullInterval = (uint64_t)(m_pCC->m_dPktSndPeriod * m_ullCPUFrequency);
      m_dCongestionWindow = m_pCC->m_dCWndSize;
   #endif

   // trace information
   gettimeofday(&m_StartTime, 0);
   m_llSentTotal = m_llRecvTotal = m_iSndLossTotal = m_iRcvLossTotal = m_iRetransTotal = m_iSentACKTotal = m_iRecvACKTotal = m_iSentNAKTotal = m_iRecvNAKTotal = 0;
   gettimeofday(&m_LastSampleTime, 0);
   m_llTraceSent = m_llTraceRecv = m_iTraceSndLoss = m_iTraceRcvLoss = m_iTraceRetrans = m_iSentACK = m_iRecvACK = m_iSentNAK = m_iRecvNAK = 0;

   // Construct and open a channel
   m_pChannel = new CChannel(m_iIPversion);

   m_pChannel->setSndBufSize(m_iUDPSndBufSize);
   m_pChannel->setRcvBufSize(m_iUDPRcvBufSize);

   m_pChannel->open(addr);

   // Create an internal buffer to be used in threads
   m_pcTmpBuf = new char [m_iPayloadSize];

   // Now UDT is opened.
   m_bOpened = true;
}

#ifndef WIN32
void* CUDT::listenHandler(void* listener)
#else
DWORD WINAPI CUDT::listenHandler(LPVOID listener)
#endif
{
   CUDT* self = static_cast<CUDT*>(listener);

   // Type 0 (handshake) control packet
   CPacket initpkt;
   char* initdata = self->m_pcTmpBuf;
   CHandShake* hs = (CHandShake *)initdata;
   initpkt.pack(0, NULL, initdata, sizeof(CHandShake));

   sockaddr* addr;
   sockaddr_in addr4;
   sockaddr_in6 addr6;

   if (AF_INET == self->m_iIPversion)
      addr = (sockaddr*)(&addr4);
   else
      addr = (sockaddr*)(&addr6);

   while (!self->m_bClosing)
   {
      // Listening to the port...
      initpkt.setLength(self->m_iPayloadSize);
      if (self->m_pChannel->recvfrom(initpkt, addr) <= 0)
         continue;

      // When a peer side connects in...
      if ((1 == initpkt.getFlag()) && (0 == initpkt.getType()))
      {
         if ((hs->m_iVersion != self->m_iVersion) || (hs->m_iType != self->m_iSockType) || (-1 == s_UDTUnited.newConnection(self->m_SocketID, addr, hs)))
         {
            // couldn't create a new connection, reject the request
            hs->m_iReqType = 1002;
         }

         self->m_pChannel->sendto(initpkt, addr);
      }
   }

   #ifndef WIN32
      return NULL;
   #else
      return 0;
   #endif
}

void CUDT::listen()
{
   CGuard cg(m_ConnectionLock);

   if (!m_bOpened)
      throw CUDTException(5, 0, 0);

   if (m_bConnected)
      throw CUDTException(5, 2, 0);

   // listen can be called more than once
   if (m_bListening)
      return;

   #ifndef WIN32
      if (0 != pthread_create(&m_ListenThread, NULL, CUDT::listenHandler, this))
         throw CUDTException(3, 1, errno);
   #else
      DWORD threadID;
      if (NULL == (m_ListenThread = CreateThread(NULL, 0, CUDT::listenHandler, this, 0, &threadID)))
         throw CUDTException(3, 1, GetLastError());
   #endif

   m_bListening = true;
}

void CUDT::connect(const sockaddr* serv_addr)
{
   CGuard cg(m_ConnectionLock);

   if (!m_bOpened)
      throw CUDTException(5, 0, 0);

   if (m_bListening)
      throw CUDTException(5, 2, 0);

   if (m_bConnected)
      throw CUDTException(5, 2, 0);

   CPacket request;
   char* reqdata = new char [m_iPayloadSize];
   CHandShake* req = (CHandShake *)reqdata;

   CPacket response;
   char* resdata = new char [m_iPayloadSize];
   CHandShake* res = (CHandShake *)resdata;

   // This is my current configurations.
   req->m_iVersion = m_iVersion;
   req->m_iType = m_iSockType;
   req->m_iMSS = m_iMSS;
   req->m_iFlightFlagSize = m_iFlightFlagSize;
   req->m_iReqType = (!m_bRendezvous) ? 1 : 0;
   req->m_iPort = 0;

   // Random Initial Sequence Number
   timeval currtime;
   gettimeofday(&currtime, 0);
   srand(currtime.tv_usec);
   m_iISN = req->m_iISN = (int32_t)(double(rand()) * CSeqNo::m_iMaxSeqNo / (RAND_MAX + 1.0));

   m_iLastDecSeq = req->m_iISN - 1;
   m_iSndLastAck = req->m_iISN;
   m_iSndLastDataAck = req->m_iISN;
   m_iSndCurrSeqNo = req->m_iISN - 1;

   // Inform the server my configurations.
   request.pack(0, NULL, reqdata, sizeof(CHandShake));
   m_pChannel->sendto(request, serv_addr);

   sockaddr* peer_addr;
   sockaddr_in addr4;
   sockaddr_in6 addr6;
   if (AF_INET == m_iIPversion)
      peer_addr = (sockaddr*)(&addr4);
   else
      peer_addr = (sockaddr*)(&addr6);

   // Wait for the negotiated configurations from the peer side.
   response.pack(0, NULL, resdata, sizeof(CHandShake));
   m_pChannel->recvfrom(response, peer_addr);

   int timeo = 3000000;

   if (m_bRendezvous)
      timeo *= 10;

   timeval entertime;
   gettimeofday(&entertime, 0);

   while (((response.getLength() <= 0) || (1 != response.getFlag()) || (0 != response.getType())) && (!m_bClosing))
   {
      m_pChannel->sendto(request, serv_addr);

      response.setLength(m_iPayloadSize);
      m_pChannel->recvfrom(response, peer_addr);

      gettimeofday(&currtime, 0);
      if ((currtime.tv_sec - entertime.tv_sec) * 1000000 + (currtime.tv_usec - entertime.tv_usec) > timeo)
      {
         delete [] reqdata;
         delete [] resdata;
         throw CUDTException(1, 1, 0);
      }

      #ifdef WIN32
         if (response.getLength() <= 0)
            Sleep(1);
      #endif
   }

   // if the socket is closed before connection...
   if (m_bClosing)
   {
      delete [] reqdata;
      delete [] resdata;
      throw CUDTException(1);
   }

   delete [] reqdata;

   if (1002 == res->m_iReqType)
   {	
      // connection request rejected
      delete [] resdata;
      throw CUDTException(1, 2, 0);
   }

   // secuity check
   bool secure = true;
   if (m_bRendezvous)
   {
      char req_ip[NI_MAXHOST];
      char req_port[NI_MAXSERV];
      char res_ip[NI_MAXHOST];
      char res_port[NI_MAXSERV];
      int addrlen = (AF_INET == m_iIPversion) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
      getnameinfo(serv_addr, addrlen, req_ip, sizeof(req_ip), req_port, sizeof(req_port), NI_NUMERICHOST|NI_NUMERICSERV);
      getnameinfo(peer_addr, addrlen, res_ip, sizeof(res_ip), res_port, sizeof(res_port), NI_NUMERICHOST|NI_NUMERICSERV);

      if ((0 != strcmp(req_ip, res_ip)) || (0 != strcmp(req_port, res_port)))
         secure = false;
   }
   else
   {
      if (m_iISN != res->m_iISN)
         secure = false;
   }

   if (!secure)
   {
      delete [] resdata;
      throw CUDTException(1, 4, 0);
   }

   if (!m_bRendezvous)
   {
      if (AF_INET == m_iIPversion)
         addr4.sin_port = htons(res->m_iPort);
      else
         addr6.sin6_port = htons(res->m_iPort);
   }

   //request accepted, continue connection setup
   m_pChannel->connect(peer_addr);

   // Got it. Re-configure according to the negotiated values.
   m_iMSS = res->m_iMSS;
   m_iMaxFlowWindowSize = res->m_iFlightFlagSize;
   m_iPktSize = m_iMSS - 28;
   m_iPayloadSize = m_iPktSize - CPacket::m_iPktHdrSize;

   m_iPeerISN = res->m_iISN;

   m_iRcvLastAck = res->m_iISN;
   m_iRcvLastAckAck = res->m_iISN;
   m_iRcvCurrSeqNo = res->m_iISN - 1;
   m_iNextExpect = res->m_iISN;

   m_iUserBufBorder = m_iRcvLastAck + (int32_t)ceil(double(m_iUDTBufSize) / m_iPayloadSize);

   delete [] resdata;

   // Prepare all structures
   m_pTimer = new CTimer;
   m_pSndBuffer = new CSndBuffer(m_iPayloadSize);
   m_pRcvBuffer = new CRcvBuffer(m_iPayloadSize, m_iUDTBufSize);
   if (SOCK_DGRAM == m_iSockType)
      m_pRcvBuffer->initMsgList();

   // after introducing lite ACK, the sndlosslist may not be cleared in time, so it requires twice space.
   m_pSndLossList = new CSndLossList(m_iMaxFlowWindowSize * 2);

   m_pRcvLossList = new CRcvLossList(m_iFlightFlagSize);
   m_pIrrPktList = new CIrregularPktList(m_iFlightFlagSize);
   m_pACKWindow = new CACKWindow(4096);
   m_pRcvTimeWindow = new CPktTimeWindow(m_iQuickStartPkts, 16, 64);

   #ifdef CUSTOM_CC
      m_pCC->init();
   #endif

   // Now I am also running, a little while after the server was running.
   #ifndef WIN32
      m_bSndThrStart = false;
      if (0 != pthread_create(&m_RcvThread, NULL, CUDT::rcvHandler, this))
         throw CUDTException(3, 1, errno);
   #else
      m_SndThread = NULL;
      DWORD threadID;
      if (NULL == (m_RcvThread = CreateThread(NULL, 0, CUDT::rcvHandler, this, 0, &threadID)))
         throw CUDTException(3, 1, GetLastError());
   #endif

   // And, I am connected too.
   m_bConnected = true;
}

void CUDT::connect(const sockaddr* peer, CHandShake* hs)
{
   // Type 0 (handshake) control packet
   CPacket initpkt;
   CHandShake ci;
   memcpy(&ci, hs, sizeof(CHandShake));
   initpkt.pack(0, NULL, &ci, sizeof(CHandShake));

   // Uses the smaller MSS between the peers        
   if (ci.m_iMSS > m_iMSS)
      ci.m_iMSS = m_iMSS;
   else
      m_iMSS = ci.m_iMSS;

   // exchange info for maximum flow window size
   m_iMaxFlowWindowSize = ci.m_iFlightFlagSize;
   ci.m_iFlightFlagSize = m_iFlightFlagSize;

   m_iPeerISN = ci.m_iISN;

   m_iRcvLastAck = ci.m_iISN;
   m_iRcvLastAckAck = ci.m_iISN;
   m_iRcvCurrSeqNo = ci.m_iISN - 1;
   m_iNextExpect = ci.m_iISN;

   m_pChannel->connect(peer);

   // use peer's ISN and send it back for security check
   m_iISN = ci.m_iISN;

   m_iLastDecSeq = m_iISN - 1;
   m_iSndLastAck = m_iISN;
   m_iSndLastDataAck = m_iISN;
   m_iSndCurrSeqNo = m_iISN - 1;

   // this is a reponse handshake
   ci.m_iReqType = -1;

   // Save the negotiated configurations.
   memcpy(hs, &ci, sizeof(CHandShake));
  
   m_iPktSize = m_iMSS - 28;
   m_iPayloadSize = m_iPktSize - CPacket::m_iPktHdrSize;

   m_iUserBufBorder = m_iRcvLastAck + (int32_t)ceil(double(m_iUDTBufSize) / m_iPayloadSize);

   // Prepare all structures
   m_pTimer = new CTimer;
   m_pSndBuffer = new CSndBuffer(m_iPayloadSize);
   m_pRcvBuffer = new CRcvBuffer(m_iPayloadSize, m_iUDTBufSize);
   if (SOCK_DGRAM == m_iSockType)
      m_pRcvBuffer->initMsgList();
   m_pSndLossList = new CSndLossList(m_iMaxFlowWindowSize * 2);
   m_pRcvLossList = new CRcvLossList(m_iFlightFlagSize);
   m_pIrrPktList = new CIrregularPktList(m_iFlightFlagSize);
   m_pACKWindow = new CACKWindow(4096);
   m_pRcvTimeWindow = new CPktTimeWindow(m_iQuickStartPkts, 16, 64);

   #ifdef CUSTOM_CC
      m_pCC->init();
   #endif

   // UDT is now running...
   #ifndef WIN32
      m_bSndThrStart = false;
      if (0 != pthread_create(&m_RcvThread, NULL, CUDT::rcvHandler, this))
         throw CUDTException(3, 1, errno);
   #else
      m_SndThread = NULL;
      DWORD threadID;
      if (NULL == (m_RcvThread = CreateThread(NULL, 0, CUDT::rcvHandler, this, 0, &threadID)))
         throw CUDTException(3, 1, GetLastError());
   #endif

   // And of course, it is connected.
   m_bConnected = true;
}

void CUDT::close()
{
   if (!m_bConnected)
      m_bClosing = true;

   CGuard cg(m_ConnectionLock);

   if (!m_bOpened)
      return;

   if (0 != m_Linger.l_onoff)
   {
      timeval t1, t2;
      gettimeofday(&t1, 0);
      t2 = t1;

      while (!m_bBroken && m_bConnected && (m_pSndBuffer->getCurrBufSize() > 0) && ((t2.tv_sec - t1.tv_sec - 1) < m_Linger.l_linger))
      {
         #ifndef WIN32
            usleep(10);
         #else
            Sleep(1);
         #endif

         gettimeofday(&t2, 0);
      }
   }

   #ifdef CUSTOM_CC
      m_pCC->close();
   #endif

   // Inform the threads handler to stop.
   m_bClosing = true;
   m_bBroken = true;

   // Signal the sender and recver if they are waiting for data.
   releaseSynch();

   // Wait for the threads to exit.

   #ifndef WIN32
      if (m_bListening)
      {
         pthread_join(m_ListenThread, NULL);
         m_bListening = false;
      }
      if (m_bConnected)
      {
         m_pTimer->interrupt();
         if (m_bSndThrStart)
         {
            pthread_join(m_SndThread, NULL);
            m_bSndThrStart = false;
         }
         pthread_join(m_RcvThread, NULL);
         m_bConnected = false;
      }
   #else
      if (m_bListening)
      {
         WaitForSingleObject(m_ListenThread, INFINITE);
         CloseHandle(m_ListenThread);
         m_bListening = false;
      }
      if (m_bConnected)
      {
         m_pTimer->interrupt();
         if (NULL != m_SndThread)
         {
            WaitForSingleObject(m_SndThread, INFINITE);
            CloseHandle(m_SndThread);
            m_SndThread = NULL;
         }
         WaitForSingleObject(m_RcvThread, INFINITE);
         CloseHandle(m_RcvThread);
         m_bConnected = false;
      }
   #endif

   // waiting all send and recv calls to stop
   CGuard sendguard(m_SendLock);
   CGuard recvguard(m_RecvLock);

   // Channel is to be destroyed.
   if (m_pChannel)
   {
      // inform the peer side with a "shutdown" packet
      if (!m_bShutdown)
         sendCtrl(5);

      m_pChannel->disconnect();
      delete m_pChannel;
      m_pChannel = NULL;
   }

   // And structures released.
   if (m_pSndBuffer)
      delete m_pSndBuffer;
   if (m_pRcvBuffer)
      delete m_pRcvBuffer;
   if (m_pSndLossList)
      delete m_pSndLossList;
   if (m_pRcvLossList)
      delete m_pRcvLossList;
   if (m_pTimer)
      delete m_pTimer;
   if (m_pIrrPktList)
      delete m_pIrrPktList;
   if (m_pACKWindow)
      delete m_pACKWindow;
   if (m_pSndTimeWindow)
      delete m_pSndTimeWindow;
   if (m_pRcvTimeWindow)
      delete m_pRcvTimeWindow;
   if (m_pCCFactory)
      delete m_pCCFactory;
   if (m_pCC)
      delete m_pCC;
   if (m_pcTmpBuf)
      delete [] m_pcTmpBuf;

   m_pSndBuffer = NULL;
   m_pRcvBuffer = NULL;
   m_pSndLossList = NULL;
   m_pRcvLossList = NULL;
   m_pTimer = NULL;
   m_pIrrPktList = NULL;
   m_pACKWindow = NULL;
   m_pSndTimeWindow = NULL;
   m_pRcvTimeWindow = NULL;
   m_pCCFactory = NULL;
   m_pCC = NULL;
   m_pcTmpBuf = NULL;

   // CLOSED.
   m_bOpened = false;
}

#ifndef WIN32
void* CUDT::sndHandler(void* sender)
#else
DWORD WINAPI CUDT::sndHandler(LPVOID sender)
#endif
{
   CUDT* self = static_cast<CUDT *>(sender);

   CPacket datapkt;
   int payload = 0;
   int offset;

   #ifdef CUSTOM_CC
      int cwnd;
   #endif

   bool probe = false;
   bool newdata;

   uint64_t entertime;
   uint64_t targettime;
   #ifdef NO_BUSY_WAITING
      uint64_t currtime;
   #endif

   timeval now;
   #ifndef WIN32
      timespec timeout;
   #endif

   while (!self->m_bClosing)
   {
      // Remember the time the last packet is sent.
      self->m_pTimer->rdtsc(entertime);

      // Loss retransmission always has higher priority.
      if ((datapkt.m_iSeqNo = self->m_pSndLossList->getLostSeq()) >= 0)
      {
         // protect m_iSndLastDataAck from updating by ACK processing
         CGuard ackguard(self->m_AckLock);

         offset = CSeqNo::seqoff(self->m_iSndLastDataAck, datapkt.m_iSeqNo) * self->m_iPayloadSize;
         if (offset < 0)
            continue;

         int32_t seqpair[2];
         int msglen;

         payload = self->m_pSndBuffer->readData(&(datapkt.m_pcData), offset, self->m_iPayloadSize, datapkt.m_iMsgNo, seqpair[0], msglen);

         if (-1 == payload)
         {
            seqpair[1] = CSeqNo::incseq(seqpair[0], msglen / self->m_iPayloadSize);

            self->sendCtrl(7, &datapkt.m_iMsgNo, seqpair, 8);

            // only one msg drop request is necessary
            self->m_pSndLossList->remove(seqpair[1]);

            continue;
         }
         else if (0 == payload)
            continue;

         ++ self->m_iTraceRetrans;
      }
      else
      {
         // If no loss, pack a new packet.
         newdata = false;

         // check congestion/flow window limit
         #ifndef CUSTOM_CC
            if (self->m_iFlowWindowSize > CSeqNo::seqlen(const_cast<int32_t&>(self->m_iSndLastAck), CSeqNo::incseq(self->m_iSndCurrSeqNo)) - 1)
         #else
            cwnd = (self->m_iFlowWindowSize < (int)self->m_dCongestionWindow) ? self->m_iFlowWindowSize : (int)self->m_dCongestionWindow;
            if (cwnd > CSeqNo::seqlen(const_cast<int32_t&>(self->m_iSndLastAck), CSeqNo::incseq(self->m_iSndCurrSeqNo)) - 1)
         #endif
         {
            if (0 != (payload = self->m_pSndBuffer->readData(&(datapkt.m_pcData), self->m_iPayloadSize, datapkt.m_iMsgNo)))
               newdata = true;
            else
            {
               //check if the sender buffer is empty
               if (0 == self->m_pSndBuffer->getCurrBufSize())
               {
                  // If yes, sleep here until a signal comes.
                  #ifndef WIN32
                     pthread_mutex_lock(&(self->m_SendDataLock));
                     while ((0 == self->m_pSndBuffer->getCurrBufSize()) && (!self->m_bClosing))
                        pthread_cond_wait(&(self->m_SendDataCond), &(self->m_SendDataLock));
                     pthread_mutex_unlock(&(self->m_SendDataLock));
                  #else
                     while ((0 == self->m_pSndBuffer->getCurrBufSize()) && (!self->m_bClosing))
                        WaitForSingleObject(self->m_SendDataCond, INFINITE);
                  #endif

                  #ifdef NO_BUSY_WAITING
                  // the waiting time should not be counted in. clear the time diff to zero.
                     self->m_ullTimeDiff = 0;
                  #endif

                  continue;
               }
            }
         }

         if (newdata)
         {
            self->m_iSndCurrSeqNo = CSeqNo::incseq(self->m_iSndCurrSeqNo);
            datapkt.m_iSeqNo = self->m_iSndCurrSeqNo;

            // every 16 (0xF) packets, a packet pair is sent
            if (0 == (datapkt.m_iSeqNo & 0xF))
               probe = true;
         }
         else
         {
            //wait here for ACK, NAK, or EXP (i.e, some data to sent)
            #ifndef WIN32
               gettimeofday(&now, 0);
               if (now.tv_usec < 990000)
               {
                  timeout.tv_sec = now.tv_sec;
                  timeout.tv_nsec = (now.tv_usec + 10000) * 1000;
               }
               else
               {
                  timeout.tv_sec = now.tv_sec + 1;
                  timeout.tv_nsec = (now.tv_usec + 10000 - 1000000) * 1000;
               }
               pthread_cond_timedwait(&self->m_WindowCond, &self->m_WindowLock, &timeout);
            #else
               WaitForSingleObject(self->m_WindowCond, 1);
            #endif

            #ifdef NO_BUSY_WAITING
               // the waiting time should not be counted in. clear the time diff to zero.
               self->m_ullTimeDiff = 0;
            #endif

            continue;
         }
      }

      gettimeofday(&now, 0);
      datapkt.m_iTimeStamp = (now.tv_sec - self->m_StartTime.tv_sec) * 1000000 + now.tv_usec - self->m_StartTime.tv_usec;
      self->m_pSndTimeWindow->onPktSent(datapkt.m_iTimeStamp);

      // Now sending.
      datapkt.setLength(payload);
      *(self->m_pChannel) << datapkt;

      #ifdef CUSTOM_CC
         self->m_pCC->onPktSent(&datapkt);
      #endif

      ++ self->m_llTraceSent;

      if (probe)
      {
         // sends out probing packet pair
         self->m_pTimer->rdtsc(targettime);
         probe = false;
      }
      else if (self->m_bFreeze)
      {
         // sending is fronzen!
         targettime = entertime + self->m_iSYNInterval * self->m_ullCPUFrequency + self->m_ullInterval;
         self->m_bFreeze = false;
      }
      else
         targettime = entertime + self->m_ullInterval;

      // wait for an inter-packet time.
      #ifndef NO_BUSY_WAITING
         self->m_pTimer->sleepto(targettime);
      #else
         self->m_pTimer->rdtsc(currtime);

         if (currtime >= targettime)
            continue;

         while (currtime + self->m_ullTimeDiff < targettime)
         {
            #ifndef WIN32
               gettimeofday(&now, 0);
               if (now.tv_usec < 990000)
               {
                  timeout.tv_sec = now.tv_sec;
                  timeout.tv_nsec = (now.tv_usec + 10000) * 1000;
               }
               else
               {
                  timeout.tv_sec = now.tv_sec + 1;
                  timeout.tv_nsec = (now.tv_usec + 10000 - 1000000) * 1000;
               }
               if (0 == pthread_cond_timedwait(&self->m_WindowCond, &self->m_WindowLock, &timeout))
                  break;
            #else
               if (WAIT_TIMEOUT != WaitForSingleObject(self->m_WindowCond, 1))
                  break;
            #endif
            self->m_pTimer->rdtsc(currtime);
         }

         self->m_pTimer->rdtsc(currtime);
         if (currtime >= targettime)
            self->m_ullTimeDiff += currtime - targettime;
         else if (self->m_ullTimeDiff > targettime - currtime)
            self->m_ullTimeDiff -= targettime - currtime;
         else
            self->m_ullTimeDiff = 0;
      #endif
   }

   #ifndef WIN32
      return NULL;
   #else
      return 0;
   #endif
}

#ifndef WIN32
void* CUDT::rcvHandler(void* recver)
#else
DWORD WINAPI CUDT::rcvHandler(LPVOID recver)
#endif
{
   CUDT* self = static_cast<CUDT *>(recver);

   CPacket packet;
   char* payload = self->m_pcTmpBuf;
   bool nextslotfound;
   int offset;
   int loss;
   #if defined (CUSTOM_CC) || defined (NO_BUSY_WAITING)
      int pktcount = 0;
   #endif

   // time
   uint64_t currtime;
   uint64_t nextacktime;
   uint64_t nextnaktime;
   uint64_t nextexptime;
   #ifdef CUSTOM_CC
      uint64_t nextccacktime;
      uint64_t nextrto;
   #endif

   // SYN interval, in clock cycles
   const uint64_t ullsynint = self->m_iSYNInterval * self->m_ullCPUFrequency;

   // ACK, NAK, and EXP intervals, in clock cycles
   uint64_t ullackint = ullsynint;
   uint64_t ullnakint = (self->m_iRTT + 4 * self->m_iRTTVar) * self->m_ullCPUFrequency;
   uint64_t ullexpint = (self->m_iRTT + 4 * self->m_iRTTVar) * self->m_ullCPUFrequency + ullsynint;

   // Set up the timers.
   self->m_pTimer->rdtsc(nextacktime);
   nextacktime += ullackint;
   self->m_pTimer->rdtsc(nextnaktime);
   nextnaktime += ullnakint;
   self->m_pTimer->rdtsc(nextexptime);
   nextexptime += ullexpint;
   #ifdef CUSTOM_CC
      self->m_pTimer->rdtsc(nextccacktime);
      nextccacktime += self->m_pCC->m_iACKPeriod * 1000 * self->m_ullCPUFrequency;
      if (!self->m_pCC->m_bUserDefinedRTO)
         self->m_pCC->m_iRTO = self->m_iRTT + 4 * self->m_iRTTVar;
      self->m_pTimer->rdtsc(nextrto);
      nextrto += self->m_pCC->m_iRTO * self->m_ullCPUFrequency;
   #endif

   while (!self->m_bClosing)
   {
      #ifdef NO_BUSY_WAITING
         // signal sleeping sender
         #ifndef WIN32
            pthread_cond_signal(&self->m_WindowCond);
         #else
            SetEvent(self->m_WindowCond);
         #endif
      #endif

      #ifdef CUSTOM_CC
         // update CC parameters
         self->m_ullInterval = (uint64_t)(self->m_pCC->m_dPktSndPeriod * self->m_ullCPUFrequency);
         self->m_dCongestionWindow = self->m_pCC->m_dCWndSize;
      #endif

      // "recv"/"recvfile" is called, overlapped mode is activated, and not enough received data in the protocol buffer
      if (self->m_bReadBuf)
      {
         // Check if there is enough data now.
         #ifndef WIN32
            pthread_mutex_lock(&(self->m_OverlappedRecvLock));
            self->m_bReadBuf = self->m_pRcvBuffer->readBuffer(const_cast<char*>(self->m_pcTempData), const_cast<int&>(self->m_iTempLen));
            pthread_mutex_unlock(&(self->m_OverlappedRecvLock));
         #else
            WaitForSingleObject(self->m_OverlappedRecvLock, INFINITE);
            self->m_bReadBuf = self->m_pRcvBuffer->readBuffer(const_cast<char*>(self->m_pcTempData), const_cast<int&>(self->m_iTempLen));
            ReleaseMutex(self->m_OverlappedRecvLock);
         #endif

         // Still no?! Register the application buffer.
         if (!self->m_bReadBuf)
         {
            offset = self->m_pRcvBuffer->registerUserBuf(const_cast<char*>(self->m_pcTempData), const_cast<int&>(self->m_iTempLen), self->m_iRcvHandle, self->m_pTempRoutine, (void*)(self->m_pTempContext));
            // there is no seq. wrap for user buffer border. If it exceeds the max. seq., we just ignore it.
            self->m_iUserBufBorder = self->m_iRcvLastAck + (int32_t)ceil(double(self->m_iTempLen - offset) / self->m_iPayloadSize);
         }

         // Otherwise, inform the blocked "recv"/"recvfile" call that the expected data has arrived.
         // or returns immediately in non-blocking IO mode.
         if (self->m_bReadBuf || !self->m_bSynRecving)
         {
            self->m_bReadBuf = false;
            #ifndef WIN32
               pthread_mutex_lock(&(self->m_OverlappedRecvLock));
               pthread_cond_signal(&(self->m_OverlappedRecvCond));
               pthread_mutex_unlock(&(self->m_OverlappedRecvLock));
            #else
               SetEvent(self->m_OverlappedRecvCond);
            #endif
         }
      }

      self->m_pTimer->rdtsc(currtime);
      loss = self->m_pRcvLossList->getFirstLostSeq();

      // Query the timers if any of them is expired.
      if ((currtime > nextacktime) || (loss >= self->m_iUserBufBorder) || ((self->m_iRcvCurrSeqNo >= self->m_iUserBufBorder - 1) && (loss < 0)))
      {
         // ACK timer expired, or user buffer is fulfilled.
         self->sendCtrl(2);

         self->m_pTimer->rdtsc(currtime);
         nextacktime = currtime + ullackint;

         #if defined (NO_BUSY_WAITING) && !defined (CUSTOM_CC)
            pktcount = 0;
         #endif
      }

      //send a "light" ACK
      #if defined (CUSTOM_CC)
         if ((self->m_pCC->m_iACKInterval > 0) && (self->m_pCC->m_iACKInterval <= pktcount))
         {
            self->sendCtrl(2, NULL, NULL, 4);
            pktcount = 0;
         }
         if ((self->m_pCC->m_iACKPeriod > 0) && (currtime >= nextccacktime))
         {
            self->sendCtrl(2, NULL, NULL, 4);
            nextccacktime += self->m_pCC->m_iACKPeriod * 1000 * self->m_ullCPUFrequency;
         }
      #elif defined (NO_BUSY_WAITING)
         else if (self->m_iSelfClockInterval <= pktcount)
         {
            self->sendCtrl(2, NULL, NULL, 4);
            pktcount = 0;
         }
      #endif

      if ((loss >= 0) && (currtime > nextnaktime))
      {
         // NAK timer expired, and there is loss to be reported.
         self->sendCtrl(3);

         self->m_pTimer->rdtsc(currtime);
         nextnaktime = currtime + ullnakint;
      }

      if (currtime > nextexptime)
      {
         // Haven't receive any information from the peer, is it dead?!
         // timeout: at least 16 expirations and must be greater than 3 seconds and be less than 30 seconds
         if (((self->m_iEXPCount > 16) && 
             (self->m_iEXPCount * ((self->m_iEXPCount - 1) * (self->m_iRTT + 4 * self->m_iRTTVar) / 2 + self->m_iSYNInterval) > 3000000))
             || (self->m_iEXPCount * ((self->m_iEXPCount - 1) * (self->m_iRTT + 4 * self->m_iRTTVar) / 2 + self->m_iSYNInterval) > 30000000))
         {
            //
            // Connection is broken. 
            // UDT does not signal any information about this instead of to stop quietly.
            // Apllication will detect this when it calls any UDT methods next time.
            //
            self->m_bClosing = true;
            self->m_bBroken = true;

            self->releaseSynch();

            continue;
         }

         // sender: Insert all the packets sent after last received acknowledgement into the sender loss list.
         // recver: Send out a keep-alive packet
         if (CSeqNo::incseq(self->m_iSndCurrSeqNo) != self->m_iSndLastAck)
         {
            int32_t csn = self->m_iSndCurrSeqNo;
            self->m_pSndLossList->insert(const_cast<int32_t&>(self->m_iSndLastAck), csn);
         }
         else
            self->sendCtrl(1);

         if (self->m_pSndBuffer->getCurrBufSize() > 0)
         {
            // Wake up the waiting sender (avoiding deadlock on an infinite sleeping)
            self->m_pTimer->interrupt();

            #ifndef WIN32
               pthread_cond_signal(&self->m_WindowCond);
            #else
               SetEvent(self->m_WindowCond);
            #endif
         }

         ++ self->m_iEXPCount;

         ullexpint = (self->m_iEXPCount * (self->m_iRTT + 4 * self->m_iRTTVar) + self->m_iSYNInterval) * self->m_ullCPUFrequency;

         self->m_pTimer->rdtsc(nextexptime);
         nextexptime += ullexpint;
      }

      #ifdef CUSTOM_CC
         if ((currtime > nextrto) && (CSeqNo::incseq(self->m_iSndCurrSeqNo) != self->m_iSndLastAck))
         {
            self->m_pCC->onTimeout();
            nextrto = currtime + self->m_pCC->m_iRTO * self->m_ullCPUFrequency;
         }
      #endif

      ////////////////////////////////////////////////////////////////////////////////////////////
      // Below is the packet receiving/processing part.

      packet.setLength(self->m_iPayloadSize);

      offset = CSeqNo::seqoff(self->m_iRcvLastAck, self->m_iNextExpect);

      // Look for a slot for the speculated data.
      if (!(self->m_pRcvBuffer->nextDataPos(&(packet.m_pcData), offset * self->m_iPayloadSize - self->m_pIrrPktList->currErrorSize(self->m_iNextExpect), self->m_iPayloadSize)))
      {
         packet.m_pcData = payload;
         nextslotfound = false;
      }
      else
         nextslotfound = true;

      // Receiving...
      *(self->m_pChannel) >> packet;

      // Got nothing?
      if (packet.getLength() <= 0)
         continue;

      // Just heard from the peer, reset the expiration count.
      self->m_iEXPCount = 1;
      ullexpint = (self->m_iRTT + 4 * self->m_iRTTVar) * self->m_ullCPUFrequency + ullsynint;
      if (CSeqNo::incseq(self->m_iSndCurrSeqNo) == self->m_iSndLastAck)
      {
         self->m_pTimer->rdtsc(nextexptime);
         nextexptime += ullexpint;
      }

      // But this is control packet, process it!
      if (packet.getFlag())
      {
         self->processCtrl(packet);

         if ((2 == packet.getType()) || (6 == packet.getType()))
         {
            ullnakint = (self->m_iRTT + 4 * self->m_iRTTVar) * self->m_ullCPUFrequency;
            //do not resent the loss report within too short period
            if (ullnakint < ullsynint)
               ullnakint = ullsynint;
         }

         self->m_pTimer->rdtsc(currtime);
         if ((2 <= packet.getType()) && (4 >= packet.getType()))
            nextexptime = currtime + ullexpint;

         continue;
      }

      #ifdef CUSTOM_CC
         // reset RTO
         if (!self->m_pCC->m_bUserDefinedRTO)
            self->m_pCC->m_iRTO = self->m_iRTT + 4 * self->m_iRTTVar;
         nextrto = currtime + self->m_pCC->m_iRTO * self->m_ullCPUFrequency;
      #endif

      // update time/delay information
      self->m_pRcvTimeWindow->onPktArrival();

      // check if it is probing packet pair
      if (0 == (packet.m_iSeqNo & 0xF))
         self->m_pRcvTimeWindow->probe1Arrival();
      else if (1 == (packet.m_iSeqNo & 0xF))
         self->m_pRcvTimeWindow->probe2Arrival();

      ++ self->m_llTraceRecv;

      offset = CSeqNo::seqoff(self->m_iRcvLastAck, packet.m_iSeqNo);
      if ((offset >= self->m_iFlightFlagSize) || (offset < 0))
         continue;

      // Oops, the speculation is wrong...
      if ((packet.m_iSeqNo != self->m_iNextExpect) || (!nextslotfound))
      {
         // Put the received data explicitly into the right slot.

         if (!(self->m_pRcvBuffer->addData(&(packet.m_pcData), offset * self->m_iPayloadSize - self->m_pIrrPktList->currErrorSize(packet.m_iSeqNo), packet.getLength())))
            continue;

         // Loss detection.
         if (CSeqNo::seqcmp(packet.m_iSeqNo, CSeqNo::incseq(self->m_iRcvCurrSeqNo)) > 0)
         {
            // If loss found, insert them to the receiver loss list
            self->m_pRcvLossList->insert(CSeqNo::incseq(self->m_iRcvCurrSeqNo), CSeqNo::decseq(packet.m_iSeqNo));

            // pack loss list for NAK
            int32_t lossdata[2];
            lossdata[0] = CSeqNo::incseq(self->m_iRcvCurrSeqNo) | 0x80000000;
            lossdata[1] = CSeqNo::decseq(packet.m_iSeqNo);

            // Generate loss report immediately.
            self->sendCtrl(3, NULL, lossdata, (CSeqNo::incseq(self->m_iRcvCurrSeqNo) == CSeqNo::decseq(packet.m_iSeqNo)) ? 1 : 2);

            self->m_iTraceRcvLoss += CSeqNo::seqlen(self->m_iRcvCurrSeqNo, packet.m_iSeqNo) - 2;
         }
      }

      // checking message bounaries...
      if (self->m_iSockType == SOCK_DGRAM)
      {
         if (packet.getMsgBoundary() != 0)
            self->m_pRcvBuffer->checkMsg(packet.getMsgBoundary(), packet.getMsgSeq(), packet.m_iSeqNo, packet.m_pcData, packet.getMsgOrderFlag(), self->m_iPayloadSize - packet.getLength());
      }
      // This is not a regular fixed size packet...
      else if (packet.getLength() != self->m_iPayloadSize)
      {
         self->m_pIrrPktList->addIrregularPkt(packet.m_iSeqNo, self->m_iPayloadSize - packet.getLength());

         //an irregular sized packet usually indicates the end of a message, so send an ACK immediately
         self->m_pTimer->rdtsc(nextacktime);
      }

      // Update the current largest sequence number that has been received.
      if (CSeqNo::seqcmp(packet.m_iSeqNo, self->m_iRcvCurrSeqNo) > 0)
      {
         self->m_iRcvCurrSeqNo = packet.m_iSeqNo;

         // Speculate next packet.
         self->m_iNextExpect = CSeqNo::incseq(self->m_iRcvCurrSeqNo);
      }
      else
      {
         // Or it is a retransmitted packet, remove it from receiver loss list.
         // rearrange receiver buffer if it is a first-come irregular packet
         // However, buffer will not be rearranged in sock_dgram mode

         if (self->m_pRcvLossList->remove(packet.m_iSeqNo) && (packet.getLength() < self->m_iPayloadSize) && (self->m_iSockType == SOCK_STREAM))
            self->m_pRcvBuffer->moveData(offset * self->m_iPayloadSize - self->m_pIrrPktList->currErrorSize(packet.m_iSeqNo) + packet.getLength(), self->m_iPayloadSize - packet.getLength());
      }

      #ifdef CUSTOM_CC
         self->m_pCC->onPktReceived(&packet);
      #endif

      #if defined (CUSTOM_CC) || defined (NO_BUSY_WAITING)
         pktcount ++;
      #endif
   }

   // acknowledge those possible unacknowledged data, if there is any
   if (0 != self->m_pRcvBuffer->getRcvDataSize())
      self->sendCtrl(2);

   #ifndef WIN32
      return NULL;
   #else
      return 0;
   #endif
}

void CUDT::sendCtrl(const int& pkttype, void* lparam, void* rparam, const int& size)
{
   CPacket ctrlpkt;

   switch (pkttype)
   {
   case 2: //010 - Acknowledgement
      {
      int32_t ack;

      // If there is no loss, the ACK is the current largest sequence number plus 1;
      // Otherwise it is the smallest sequence number in the receiver loss list.
      if (0 == m_pRcvLossList->getLossLength())
         ack = CSeqNo::incseq(m_iRcvCurrSeqNo);
      else
         ack = m_pRcvLossList->getFirstLostSeq();

      // send out a lite ACK
      // to save time on buffer processing and bandwidth/AS measurement, a lite ACK only feeds back an ACK number
      if (4 == size)
      {
         ctrlpkt.pack(2, NULL, &ack, size);
         *m_pChannel << ctrlpkt;

         ++ m_iSentACK;
               
         break;
      }

      uint64_t currtime;
      m_pTimer->rdtsc(currtime);

      // There is new received packet to acknowledge, update related information.
      if (CSeqNo::seqcmp(ack, m_iRcvLastAck) > 0)
      {
         int acksize = CSeqNo::seqlen(m_iRcvLastAck, ack) - 1;

         m_iRcvLastAck = ack;

         if (m_pRcvBuffer->ackData(acksize * m_iPayloadSize - m_pIrrPktList->currErrorSize(m_iRcvLastAck)) && m_bSynRecving)
         {
            //singal an blocking overlapped IO. 
            #ifndef WIN32
               pthread_mutex_lock(&m_OverlappedRecvLock);
               pthread_cond_signal(&m_OverlappedRecvCond);
               pthread_mutex_unlock(&m_OverlappedRecvLock);
            #else
               SetEvent(m_OverlappedRecvCond);
            #endif
         }

         m_iUserBufBorder = m_iRcvLastAck + (int32_t)ceil(double(m_pRcvBuffer->getAvailBufSize()) / m_iPayloadSize);

         if (m_iSockType == SOCK_STREAM)
         {
            // signal a waiting "recv" call if there is any data available
            #ifndef WIN32
               pthread_mutex_lock(&m_RecvDataLock);
               if ((m_bSynRecving) && (0 != m_pRcvBuffer->getRcvDataSize()))
                  pthread_cond_signal(&m_RecvDataCond);
               pthread_mutex_unlock(&m_RecvDataLock);
            #else
               if ((m_bSynRecving) && (0 != m_pRcvBuffer->getRcvDataSize()))
                  SetEvent(m_RecvDataCond);
            #endif

            m_pIrrPktList->deleteIrregularPkt(m_iRcvLastAck);
         }
         else
         {
            // message mode, check if there is any new messages...
            if (m_pRcvBuffer->ackMsg(m_iRcvLastAck, m_pRcvLossList))
            {
               #ifndef WIN32
                  pthread_mutex_lock(&m_RecvDataLock);
                  if ((m_bSynRecving) && (0 != m_pRcvBuffer->getValidMsgCount()))
                     pthread_cond_signal(&m_RecvDataCond);
                  pthread_mutex_unlock(&m_RecvDataLock);
               #else
                  if ((m_bSynRecving) && (0 != m_pRcvBuffer->getValidMsgCount()))
                     SetEvent(m_RecvDataCond);
               #endif
            }
         }
      }
      else if (ack == m_iRcvLastAck)
      {
         #ifdef CUSTOM_CC
            break;
         #endif

         if ((currtime - m_ullLastAckTime) < ((m_iRTT + 4 * m_iRTTVar) * m_ullCPUFrequency))
            break;
      }
      else
         break;

      // Send out the ACK only if has not been received by the sender before
      if (CSeqNo::seqcmp(m_iRcvLastAck, m_iRcvLastAckAck) > 0)
      {
         int32_t data[5];

         m_iAckSeqNo = CAckNo::incack(m_iAckSeqNo);
         data[0] = m_iRcvLastAck;
         data[1] = m_iRTT;
         data[2] = m_iRTTVar;

         #ifndef CUSTOM_CC
         flowControl(m_pRcvTimeWindow->getPktRcvSpeed());
         data[3] = m_iFlowControlWindow;
         if (data[3] > (int32_t)(m_pRcvBuffer->getAvailBufSize() / m_iPayloadSize))
         #endif
            data[3] = (int32_t)(m_pRcvBuffer->getAvailBufSize() / m_iPayloadSize);
         if (data[3] < 2)
            data[3] = 2;

         data[4] = m_bRcvSlowStart? 0 : m_pRcvTimeWindow->getBandwidth();

         ctrlpkt.pack(2, &m_iAckSeqNo, data, 20);
         *m_pChannel << ctrlpkt;

         m_pACKWindow->store(m_iAckSeqNo, m_iRcvLastAck);

         m_pTimer->rdtsc(m_ullLastAckTime);

         ++ m_iSentACK;
      }

      break;
      }

   case 6: //110 - Acknowledgement of Acknowledgement
      ctrlpkt.pack(6, lparam);

      *m_pChannel << ctrlpkt;

      break;

   case 3: //011 - Loss Report
      if (NULL != rparam)
      {
         if (1 == size)
         {
            // only 1 loss packet
            ctrlpkt.pack(3, NULL, (int32_t *)rparam + 1, 4);
         }
         else
         {
            // more than 1 loss packets
            ctrlpkt.pack(3, NULL, rparam, 8);
         }

         *m_pChannel << ctrlpkt;

         //Slow Start Stopped, if it is not
         m_bRcvSlowStart = false;

         ++ m_iSentNAK;
      }
      else if (m_pRcvLossList->getLossLength() > 0)
      {
         // this is periodically NAK report

         // read loss list from the local receiver loss list
         int32_t* data = (int32_t*)m_pcTmpBuf;
         int losslen;
         m_pRcvLossList->getLossArray(data, losslen, m_iPayloadSize / 4, m_iRTT + 4 * m_iRTTVar);

         if (0 < losslen)
         {
            ctrlpkt.pack(3, NULL, data, losslen * 4);
            *m_pChannel << ctrlpkt;

            //Slow Start Stopped, if it is not
            m_bRcvSlowStart = false;

            ++ m_iSentNAK;
         }
      }

      break;

   case 4: //100 - Congestion Warning
      ctrlpkt.pack(4);
      *m_pChannel << ctrlpkt;

      //Slow Start Stopped, if it is not
      m_bRcvSlowStart = false;

      m_pTimer->rdtsc(m_ullLastWarningTime);

      break;

   case 1: //001 - Keep-alive
      ctrlpkt.pack(1);
      *m_pChannel << ctrlpkt;
      
      break;

   case 0: //000 - Handshake
      ctrlpkt.pack(0, NULL, rparam, sizeof(CHandShake));
      *m_pChannel << ctrlpkt;

      break;

   case 5: //101 - Shutdown
      ctrlpkt.pack(5);
      *m_pChannel << ctrlpkt;

      break;

   case 7: //111 - Msg drop request
      ctrlpkt.pack(7, lparam, rparam, 8);
      *m_pChannel << ctrlpkt;

      break;

   case 65535: //0x7FFF - Resevered for future use
      break;

   default:
      break;
   }
}

void CUDT::processCtrl(CPacket& ctrlpkt)
{
   switch (ctrlpkt.getType())
   {
   case 2: //010 - Acknowledgement
      {
      int32_t ack;

      // process a lite ACK
      if (4 == ctrlpkt.getLength())
      {
         ack = *(int32_t *)ctrlpkt.m_pcData;
         if (CSeqNo::seqcmp(ack, const_cast<int32_t&>(m_iSndLastAck)) > 0)
            m_iSndLastAck = ack;

         #ifdef CUSTOM_CC
            m_pCC->onACK(ack);
         #endif

         ++ m_iRecvACK;

         break;
      }

      // read ACK seq. no.
      ack = ctrlpkt.getAckSeqNo();

      // send ACK acknowledgement
      sendCtrl(6, &ack);

      // Got data ACK
      ack = *(int32_t *)ctrlpkt.m_pcData;

      if (CSeqNo::seqcmp(ack, const_cast<int32_t&>(m_iSndLastAck)) > 0)
         m_iSndLastAck = ack;

      // protect packet retransmission
      #ifndef WIN32
         pthread_mutex_lock(&m_AckLock);
      #else
         WaitForSingleObject(m_AckLock, INFINITE);
      #endif

      int offset = CSeqNo::seqoff(m_iSndLastDataAck, ack);
      if (offset <= 0)
      {
         // discard it if it is a repeated ACK
         #ifndef WIN32
            pthread_mutex_unlock(&m_AckLock);
         #else
            ReleaseMutex(m_AckLock);
         #endif

         break;
      }

      // acknowledge the sending buffer
      m_pSndBuffer->ackData(offset * m_iPayloadSize, m_iPayloadSize);

      // update sending variables
      m_iSndLastDataAck = ack;
      m_pSndLossList->remove(CSeqNo::decseq(m_iSndLastDataAck));

      #ifndef WIN32
         pthread_mutex_unlock(&m_AckLock);

         pthread_cond_signal(&m_WindowCond);

         pthread_mutex_lock(&m_SendBlockLock);
         if (m_bSynSending)
            pthread_cond_signal(&m_SendBlockCond);
         pthread_mutex_unlock(&m_SendBlockLock);
      #else
         ReleaseMutex(m_AckLock);

         SetEvent(m_WindowCond);

         if (m_bSynSending)
            SetEvent(m_SendBlockCond);
      #endif

      // Update RTT
      m_iRTT = *((int32_t *)ctrlpkt.m_pcData + 1);
      m_iRTTVar = *((int32_t *)ctrlpkt.m_pcData + 2);

      // Update Flow Window Size
      m_iFlowWindowSize = *((int32_t *)ctrlpkt.m_pcData + 3);

      #ifndef CUSTOM_CC
         // quick start
         if ((m_bSndSlowStart) && (*((int32_t *)ctrlpkt.m_pcData + 4) > 0))
         {
            m_bSndSlowStart = false;
            m_ullInterval = m_iFlowWindowSize * m_ullCPUFrequency / (m_iRTT + m_iSYNInterval);
         }
      #endif

      // Update Estimated Bandwidth
      if (*((int32_t *)ctrlpkt.m_pcData + 4) > 0)
         m_iBandwidth = (m_iBandwidth * 7 + *((int32_t *)ctrlpkt.m_pcData + 4)) >> 3;

      #ifndef CUSTOM_CC
         // an ACK may activate rate control
         timeval currtime;
         gettimeofday(&currtime, 0);

         if (((currtime.tv_sec - m_LastSYNTime.tv_sec) * 1000000 + currtime.tv_usec - m_LastSYNTime.tv_usec) >= m_iSYNInterval)
         {
            m_LastSYNTime = currtime;

            rateControl();
         }
      #endif

      // Wake up the waiting sender and correct the sending rate
      m_pTimer->interrupt();

      ++ m_iRecvACK;

      break;
      }

   case 6: //110 - Acknowledgement of Acknowledgement
      {
      int32_t ack;
      int rtt = -1;
      //timeval currtime;

      // update RTT
      rtt = m_pACKWindow->acknowledge(ctrlpkt.getAckSeqNo(), ack);

      if (rtt <= 0)
         break;

      //
      // Well, I decide to temporaly disable the use of delay.
      // a good idea but the algorithm to detect it is not good enough.
      // I'll come back later...
      //

      //m_pRcvTimeWindow->ack2Arrival(rtt);

      // check packet delay trend
      //m_pTimer->rdtsc(currtime);
      //if (m_pRcvTimeWindow->getDelayTrend() && (currtime - m_ullLastWarningTime > (m_iRTT + 4 * m_iRTTVar) * m_ullCPUFrequency))
      //   sendCtrl(4);

      // RTT EWMA
      m_iRTTVar = (m_iRTTVar * 3 + abs(rtt - m_iRTT)) >> 2;
      m_iRTT = (m_iRTT * 7 + rtt) >> 3;

      // update last ACK that has been received by the sender
      if (CSeqNo::seqcmp(ack, m_iRcvLastAckAck) > 0)
         m_iRcvLastAckAck = ack;

      break;
      }

   case 3: //011 - Loss Report
      {
      #ifndef CUSTOM_CC
         //Slow Start Stopped, if it is not
         m_bSndSlowStart = false;
      #endif

      int32_t* losslist = (int32_t *)(ctrlpkt.m_pcData);

      #ifndef CUSTOM_CC
         // Congestion Control on Loss
         if (CSeqNo::seqcmp(losslist[0] & 0x7FFFFFFF, m_iLastDecSeq) > 0)
         {
            m_bFreeze = true;

            //m_ullLastDecRate = m_ullInterval;
            //m_ullInterval = (uint64_t)ceil(m_ullInterval * 1.125);

            m_iAvgNAKNum = (int)ceil((double)m_iAvgNAKNum * 0.875 + (double)m_iNAKCount * 0.125) + 1;
            m_iNAKCount = 1;
            m_iDecCount = 1;

            m_iLastDecSeq = m_iSndCurrSeqNo;

            // remove global synchronization using randomization
            srand(m_iLastDecSeq);
            m_iDecRandom = (int)(rand() * double(m_iAvgNAKNum) / (RAND_MAX + 1.0)) + 1;
         }
         else if ((m_iDecCount ++ < 5) && (0 == (++ m_iNAKCount % m_iDecRandom)))
         {
            // 0.875^5 = 0.51, rate should not be decreased by more than half within a congestion period

            m_ullInterval = (uint64_t)ceil(m_ullInterval * 1.125);

            m_iLastDecSeq = m_iSndCurrSeqNo;
         }
      #else
         m_pCC->onLoss(losslist, ctrlpkt.getLength());
      #endif

      // decode loss list message and insert loss into the sender loss list
      for (int i = 0, n = (int)(ctrlpkt.getLength() / 4); i < n; ++ i)
      {
         if (0 != (losslist[i] & 0x80000000))
         {
            if (CSeqNo::seqcmp(losslist[i] & 0x7FFFFFFF, const_cast<int32_t&>(m_iSndLastAck)) >= 0)
               m_iTraceSndLoss += m_pSndLossList->insert(losslist[i] & 0x7FFFFFFF, losslist[i + 1]);
            else if (CSeqNo::seqcmp(losslist[i + 1], const_cast<int32_t&>(m_iSndLastAck)) >= 0)
               m_iTraceSndLoss += m_pSndLossList->insert(const_cast<int32_t&>(m_iSndLastAck), losslist[i + 1]);

            ++ i;
         }
         else if (CSeqNo::seqcmp(losslist[i], const_cast<int32_t&>(m_iSndLastAck)) >= 0)
         {
            m_iTraceSndLoss += m_pSndLossList->insert(losslist[i], losslist[i]);
         }
      }

      // Wake up the waiting sender (avoiding deadlock on an infinite sleeping)
      m_pSndLossList->insert(const_cast<int32_t&>(m_iSndLastAck), const_cast<int32_t&>(m_iSndLastAck));
      m_pTimer->interrupt();

      #ifndef WIN32
         pthread_cond_signal(&m_WindowCond);
      #else
         SetEvent(m_WindowCond);
      #endif

      // loss received during this SYN
      m_bLoss = true;

      ++ m_iRecvNAK;

      break;
      }

   case 4: //100 - Delay Warning
      #ifndef CUSTOM_CC
         //Slow Start Stopped, if it is not
         m_bSndSlowStart = false;

         // One way packet delay is increasing, so decrease the sending rate
         m_ullInterval = (uint64_t)ceil(m_ullInterval * 1.125);

         m_iLastDecSeq = m_iSndCurrSeqNo;
      #endif

      break;

   case 1: //001 - Keep-alive
      // The only purpose of keep-alive packet is to tell that the peer is still alive
      // nothing needs to be done.

      break;

   case 0: //000 - Handshake
      if ((((CHandShake*)(ctrlpkt.m_pcData))->m_iReqType != -1) && (m_iPeerISN - 1 == m_iRcvCurrSeqNo) && (m_iISN == m_iSndLastAck))
      {
         // The peer side has not received the handshake message, so it keeps querying
         // resend the handshake packet

         CHandShake initdata;
         initdata.m_iISN = m_iISN;
         initdata.m_iMSS = m_iMSS;
         initdata.m_iFlightFlagSize = m_iFlightFlagSize;
         initdata.m_iReqType = -1;
         sendCtrl(0, NULL, (char *)&initdata, sizeof(CHandShake));
      }

      break;

   case 5: //101 - Shutdown
      m_bShutdown = true;
      m_bClosing = true;
      m_bBroken = true;

      // Signal the sender and recver if they are waiting for data.
      releaseSynch();

      break;

   case 7: //111 - Msg drop request
      m_pRcvBuffer->dropMsg(ctrlpkt.getMsgSeq());

      m_pRcvLossList->remove(*(int32_t*)ctrlpkt.m_pcData, *(int32_t*)(ctrlpkt.m_pcData + 4));

      break;

   case 65535: //0x7FFF - reserved and user defined messages
      #ifdef CUSTOM_CC
         m_pCC->processCustomMsg(&ctrlpkt);
      #endif

      break;

   default:
      break;
   }
}

void CUDT::rateControl()
{
   // During Slow Start, no rate increase
   if (m_bSndSlowStart)
      return;

   if (m_bLoss)
   {
      m_bLoss = false;
      return;
   }

   int B = (int)(m_iBandwidth - 1000000.0 / m_ullInterval * m_ullCPUFrequency);
   if ((m_ullInterval > m_ullLastDecRate) && ((m_iBandwidth / 9) < B))
      B = m_iBandwidth / 9;

   double inc;

   if (B <= 0)
      inc = 1.0 / m_iMSS;
   else
   {
      // inc = max(10 ^ ceil(log10( B * MSS * 8 ) * Beta / MSS, 1/MSS)
      // Beta = 1.5 * 10^(-6)

      inc = pow(10.0, ceil(log10(B * m_iMSS * 8.0))) * 0.0000015 / m_iMSS;

      if (inc < 1.0/m_iMSS)
         inc = 1.0/m_iMSS;
   }

   m_ullInterval = (uint64_t)((m_ullInterval * m_iSYNInterval * m_ullCPUFrequency) / (m_ullInterval * inc + m_iSYNInterval * m_ullCPUFrequency));

   // correct the sending interval, which should not be less than the minimum sending interval of the system
   if (m_ullInterval < (uint64_t)(m_ullCPUFrequency * m_pSndTimeWindow->getMinPktSndInt() * 0.9))
      m_ullInterval = (uint64_t)(m_ullCPUFrequency * m_pSndTimeWindow->getMinPktSndInt() * 0.9);
}

void CUDT::flowControl(const int& recvrate)
{
   if (m_bRcvSlowStart)
   {
      m_iFlowControlWindow = CSeqNo::seqlen(m_iPeerISN, m_iRcvLastAck) - 1;

      if ((recvrate > 0) && (m_iFlowControlWindow >= m_iQuickStartPkts))
      {
         // quick start
         m_bRcvSlowStart = false;
         m_iFlowControlWindow = (int)((int64_t)recvrate * (m_iRTT + m_iSYNInterval) / 1000000) + 16;
      }
   }
   else if (recvrate > 0)
      m_iFlowControlWindow = (int)ceil(m_iFlowControlWindow * 0.875 + recvrate / 1000000.0 * (m_iRTT + m_iSYNInterval) * 0.125) + 16;

   if (m_iFlowControlWindow > m_iFlightFlagSize)
   {
      m_iFlowControlWindow = m_iFlightFlagSize;
      m_bRcvSlowStart = false;
   }
}

int CUDT::send(char* data, const int& len, int* overlapped, const UDT_MEM_ROUTINE func, void* context)
{
   if (SOCK_DGRAM == m_iSockType)
      throw CUDTException(5, 10, 0);

   CGuard sendguard(m_SendLock);

   // throw an exception if not connected
   if (m_bBroken)
      throw CUDTException(2, 1, 0);
   else if (!m_bConnected)
      throw CUDTException(2, 2, 0);

   if (len <= 0)
      return 0;

   // lazy snd thread creation
   #ifndef WIN32
      if (!m_bSndThrStart)
   #else
      if (NULL == m_SndThread)
   #endif
   {
      m_pSndTimeWindow = new CPktTimeWindow();

      #ifndef WIN32
         if (0 != pthread_create(&m_SndThread, NULL, CUDT::sndHandler, this))
         {
            delete m_pSndTimeWindow;
            m_pSndTimeWindow = NULL;
            throw CUDTException(3, 1, errno);
         }
         m_bSndThrStart = true;
      #else
         DWORD threadID;
         if (NULL == (m_SndThread = CreateThread(NULL, 0, CUDT::sndHandler, this, 0, &threadID)))
         {
            delete m_pSndTimeWindow;
            m_pSndTimeWindow = NULL;
            throw CUDTException(3, 1, GetLastError());
         }
      #endif
   }

   if (m_pSndBuffer->getCurrBufSize() > m_iSndQueueLimit)
   {
      if (!m_bSynSending)
         throw CUDTException(6, 1, 0);
      else
      {
         // wait here during a blocking sending
         #ifndef WIN32
            pthread_mutex_lock(&m_SendBlockLock);
            if (m_iSndTimeOut < 0) 
            { 
               while (!m_bBroken && m_bConnected && (m_iSndQueueLimit < m_pSndBuffer->getCurrBufSize()))
                  pthread_cond_wait(&m_SendBlockCond, &m_SendBlockLock);
            }
            else
            {
               timeval currtime; 
               timespec locktime; 
    
               gettimeofday(&currtime, 0); 
               locktime.tv_sec = currtime.tv_sec + ((int64_t)m_iSndTimeOut * 1000 + currtime.tv_usec) / 1000000; 
               locktime.tv_nsec = ((int64_t)m_iSndTimeOut * 1000 + currtime.tv_usec) % 1000000 * 1000; 
    
               pthread_cond_timedwait(&m_SendBlockCond, &m_SendBlockLock, &locktime);
            }
            pthread_mutex_unlock(&m_SendBlockLock);
         #else
            if (m_iSndTimeOut < 0)
            {
               while (!m_bBroken && m_bConnected && (m_iSndQueueLimit < m_pSndBuffer->getCurrBufSize()))
                  WaitForSingleObject(m_SendBlockCond, INFINITE);
            }
            else 
               WaitForSingleObject(m_SendBlockCond, DWORD(m_iSndTimeOut)); 
         #endif

         // check the connection status
         if (m_bBroken)
            throw CUDTException(2, 1, 0);
      }
   }

   if ((m_iSndTimeOut >= 0) && (m_iSndQueueLimit < m_pSndBuffer->getCurrBufSize())) 
      return 0; 

   char* buf;
   int handle = 0;
   UDT_MEM_ROUTINE r = func;

   if (NULL == overlapped)
   {
      buf = new char[len];
      memcpy(buf, data, len);
      data = buf;
      r = CSndBuffer::releaseBuffer;
   }
   else
   {
      #ifndef WIN32
         pthread_mutex_lock(&m_HandleLock);
      #else
         WaitForSingleObject(m_HandleLock, INFINITE);
      #endif
      if (1 == m_iSndHandle)
         m_iSndHandle = 1 << 30;
      // "send" handle descriptor is POSITIVE and DECREASING
      *overlapped = handle = -- m_iSndHandle;
      #ifndef WIN32
         pthread_mutex_unlock(&m_HandleLock);
      #else
         ReleaseMutex(m_HandleLock);
      #endif
   }

   // insert the user buffer into the sening list
   m_pSndBuffer->addBuffer(data, len, handle, r, context);

   // signal the sending thread in case that it is waiting
   #ifndef WIN32
      pthread_mutex_lock(&m_SendDataLock);
      pthread_cond_signal(&m_SendDataCond);
      pthread_mutex_unlock(&m_SendDataLock);

      pthread_cond_signal(&m_WindowCond);
   #else
      SetEvent(m_SendDataCond);
      SetEvent(m_WindowCond);
   #endif

   // UDT either sends nothing or sends all 
   return len;
}

int CUDT::recv(char* data, const int& len, int* overlapped, UDT_MEM_ROUTINE func, void* context)
{
   if (SOCK_DGRAM == m_iSockType)
      throw CUDTException(5, 10, 0);

   CGuard recvguard(m_RecvLock);

   // throw an exception if not connected
   if (!m_bConnected)
      throw CUDTException(2, 2, 0);
   else if ((m_bBroken) && (0 == m_pRcvBuffer->getRcvDataSize()))
      throw CUDTException(2, 1, 0);
   else if ((m_bSynRecving || (NULL == overlapped)) && (0 < m_pRcvBuffer->getPendingQueueSize()))
      throw CUDTException(6, 4, 0);

   if (len <= 0)
      return 0;

   if ((NULL == overlapped) && (0 == m_pRcvBuffer->getRcvDataSize()))
   {
      if (!m_bSynRecving)
         throw CUDTException(6, 2, 0);
      else
      {
         #ifndef WIN32
            pthread_mutex_lock(&m_RecvDataLock);
            if (m_iRcvTimeOut < 0) 
            { 
               while (!m_bBroken && (0 == m_pRcvBuffer->getRcvDataSize()))
                  pthread_cond_wait(&m_RecvDataCond, &m_RecvDataLock);
            }
            else
            {
               timeval currtime; 
               timespec locktime; 
    
               gettimeofday(&currtime, 0); 
               locktime.tv_sec = currtime.tv_sec + ((int64_t)m_iRcvTimeOut * 1000 + currtime.tv_usec) / 1000000; 
               locktime.tv_nsec = ((int64_t)m_iRcvTimeOut * 1000 + currtime.tv_usec) % 1000000 * 1000; 
    
               pthread_cond_timedwait(&m_RecvDataCond, &m_RecvDataLock, &locktime); 
            }
            pthread_mutex_unlock(&m_RecvDataLock);
         #else
            if (m_iRcvTimeOut < 0)
            {
               while (!m_bBroken && (0 == m_pRcvBuffer->getRcvDataSize()))
                  WaitForSingleObject(m_RecvDataCond, INFINITE);
            }
            else
               WaitForSingleObject(m_RecvDataCond, DWORD(m_iRcvTimeOut));
         #endif
      }
   }

   if ((NULL == overlapped) || (m_bSynRecving && m_bBroken))
   {
      int avail = m_pRcvBuffer->getRcvDataSize();
      if (len <= avail)
         avail = len;

      if (avail > 0)
         m_pRcvBuffer->readBuffer(data, avail);
      else if (m_bBroken)
         throw CUDTException(2, 1, 0);

      return avail;
   }

   // Overlapped IO begins.
   if (!m_bSynRecving && m_bBroken)
      throw CUDTException(2, 1, 0);
   else if (m_iUDTBufSize <= m_pRcvBuffer->getPendingQueueSize())
      throw CUDTException(6, 3, 0);

   #ifndef WIN32
      pthread_mutex_lock(&m_OverlappedRecvLock);
   #else
      WaitForSingleObject(m_OverlappedRecvLock, INFINITE);
   #endif

   #ifndef WIN32
      pthread_mutex_lock(&m_HandleLock);
   #else
      WaitForSingleObject(m_HandleLock, INFINITE);
   #endif
   if (-1 == m_iRcvHandle)
      m_iRcvHandle = -(1 << 30);
   // "recv" handle descriptor is NEGATIVE and INCREASING
   *overlapped = ++ m_iRcvHandle;
   #ifndef WIN32
      pthread_mutex_unlock(&m_HandleLock);
   #else
      ReleaseMutex(m_HandleLock);
   #endif

   if (len <= m_pRcvBuffer->getRcvDataSize())
   {
      m_pRcvBuffer->readBuffer(data, len);

      if (NULL != func)
         func(data, len, context);

      #ifndef WIN32
         pthread_mutex_unlock(&m_OverlappedRecvLock);
      #else
         ReleaseMutex(m_OverlappedRecvLock);
      #endif

      return len;
   }

   m_pcTempData = data;
   m_iTempLen = len;
   m_pTempRoutine = func;
   m_pTempContext = context;
   m_bReadBuf = true;

   #ifndef WIN32
      while (m_bReadBuf && !m_bBroken)
         pthread_cond_wait(&m_OverlappedRecvCond, &m_OverlappedRecvLock);
      while (!m_bBroken && (!m_bSynRecving || (0 != m_pRcvBuffer->getPendingQueueSize())))
         pthread_cond_wait(&m_OverlappedRecvCond, &m_OverlappedRecvLock);
      pthread_mutex_unlock(&m_OverlappedRecvLock);
   #else
      ReleaseMutex(m_OverlappedRecvLock);
      while (m_bReadBuf && !m_bBroken)
         WaitForSingleObject(m_OverlappedRecvCond, INFINITE);
      while (!m_bBroken && (!m_bSynRecving || (0 != m_pRcvBuffer->getPendingQueueSize())))
         WaitForSingleObject(m_OverlappedRecvCond, INFINITE);
   #endif

   if (!m_bSynRecving)
      return 0;

   // check if the receiving is successful or the connection is broken
   if (m_bBroken)
   {
      // remove incompleted overlapped recv buffer
      m_pRcvBuffer->removeUserBuf();

      // connection broken and and no data received, report error
      if (0 == m_pRcvBuffer->getRcvDataSize())
         throw CUDTException(2, 1, 0);

      return (len <= m_pRcvBuffer->getRcvDataSize()) ? len : m_pRcvBuffer->getRcvDataSize();
   }

   return len;
}

int CUDT::sendmsg(const char* data, const int& len, const int& msttl, const bool& inorder)
{
   if (SOCK_STREAM == m_iSockType)
      throw CUDTException(5, 9, 0);

   CGuard sendguard(m_SendLock);

   // throw an exception if not connected
   if (m_bBroken)
      throw CUDTException(2, 1, 0);
   else if (!m_bConnected)
      throw CUDTException(2, 2, 0);

   if (len <= 0)
      return 0;

   // lazy snd thread creation
   #ifndef WIN32
      if (!m_bSndThrStart)
   #else
      if (NULL == m_SndThread)
   #endif
   {
      m_pSndTimeWindow = new CPktTimeWindow();

      #ifndef WIN32
         if (0 != pthread_create(&m_SndThread, NULL, CUDT::sndHandler, this))
         {
            delete m_pSndTimeWindow;
            m_pSndTimeWindow = NULL;
            throw CUDTException(3, 1, errno);
         }
         m_bSndThrStart = true;
      #else
         DWORD threadID;
         if (NULL == (m_SndThread = CreateThread(NULL, 0, CUDT::sndHandler, this, 0, &threadID)))
         {
            delete m_pSndTimeWindow;
            m_pSndTimeWindow = NULL;
            throw CUDTException(3, 1, GetLastError());
         }
      #endif
   }

   if (m_pSndBuffer->getCurrBufSize() > m_iSndQueueLimit)
   {
      if (!m_bSynSending)
         throw CUDTException(6, 1, 0);
      else
      {
         // wait here during a blocking sending
         #ifndef WIN32
            pthread_mutex_lock(&m_SendBlockLock);
            while (!m_bBroken && m_bConnected && (m_iSndQueueLimit < m_pSndBuffer->getCurrBufSize()))
               pthread_cond_wait(&m_SendBlockCond, &m_SendBlockLock);
            pthread_mutex_unlock(&m_SendBlockLock);
         #else
            while (!m_bBroken && m_bConnected && (m_iSndQueueLimit < m_pSndBuffer->getCurrBufSize()))
               WaitForSingleObject(m_SendBlockCond, INFINITE);
         #endif

         // check the connection status
         if (m_bBroken)
            throw CUDTException(2, 1, 0);
      }
   }

   char* buf;
   int handle = 0;
   UDT_MEM_ROUTINE r = CSndBuffer::releaseBuffer;

   buf = new char[len];
   memcpy(buf, data, len);
   data = buf;
   r = CSndBuffer::releaseBuffer;

   // insert the user buffer into the sening list
   m_pSndBuffer->addBuffer(data, len, handle, r, NULL, msttl, m_iSndCurrSeqNo, inorder);

   // signal the sending thread in case that it is waiting
   #ifndef WIN32
      pthread_mutex_lock(&m_SendDataLock);
      pthread_cond_signal(&m_SendDataCond);
      pthread_mutex_unlock(&m_SendDataLock);

      pthread_cond_signal(&m_WindowCond);
   #else
      SetEvent(m_SendDataCond);
      SetEvent(m_WindowCond);
   #endif

   return len;   
}

int CUDT::recvmsg(char* data, const int& len)
{
   if (SOCK_STREAM == m_iSockType)
      throw CUDTException(5, 9, 0);

   CGuard recvguard(m_RecvLock);

   // throw an exception if not connected
   if (!m_bConnected)
      throw CUDTException(2, 2, 0);
   else if ((m_bBroken) && (0 == m_pRcvBuffer->getValidMsgCount()))
      throw CUDTException(2, 1, 0);

   if (len <= 0)
      return 0;

   if (0 == m_pRcvBuffer->getValidMsgCount())
   {
      if (!m_bSynRecving)
         throw CUDTException(6, 2, 0);
      else
      {
         #ifndef WIN32
            pthread_mutex_lock(&m_RecvDataLock);
            while (!m_bBroken && (0 == m_pRcvBuffer->getValidMsgCount()))
               pthread_cond_wait(&m_RecvDataCond, &m_RecvDataLock);
            pthread_mutex_unlock(&m_RecvDataLock);
         #else
            while (!m_bBroken && (0 == m_pRcvBuffer->getValidMsgCount()))
               WaitForSingleObject(m_RecvDataCond, INFINITE);
         #endif
      }
   }

   if (m_bBroken && (0 == m_pRcvBuffer->getValidMsgCount()))
      throw CUDTException(2, 1, 0);

   return m_pRcvBuffer->readMsg(data, len);
}

bool CUDT::getOverlappedResult(const int& handle, int& progress, const bool& wait)
{
   if (SOCK_DGRAM == m_iSockType)
      throw CUDTException(5, 10, 0);

   // throw an exception if not connected
   if (m_bBroken)
      throw CUDTException(2, 1, 0);
   else if (!m_bConnected)
      throw CUDTException(2, 2, 0);

   // check sending buffer
   if (handle > 0)
   {
      bool res = m_pSndBuffer->getOverlappedResult(handle, progress);
      while (wait && !res && !m_bBroken)
      {
         #ifndef WIN32
            usleep(1);
         #else
            Sleep(1);
         #endif

         res = m_pSndBuffer->getOverlappedResult(handle, progress);
      }

      if (m_bBroken)
         throw CUDTException(2, 1, 0);

      return res;
   }

   // check receiving buffer
   CGuard recvguard(m_RecvLock);

   bool res = m_pRcvBuffer->getOverlappedResult(handle, progress);
   while (wait && !res && !m_bBroken)
   {
      #ifndef WIN32
         usleep(1);
      #else
         Sleep(1);
      #endif

      res = m_pRcvBuffer->getOverlappedResult(handle, progress);
   }

   if (m_bBroken)
      throw CUDTException(2, 1, 0);

   return res;
}

int64_t CUDT::sendfile(ifstream& ifs, const int64_t& offset, const int64_t& size, const int& block)
{
   if (SOCK_DGRAM == m_iSockType)
      throw CUDTException(5, 10, 0);

   CGuard sendguard(m_SendLock);

   if (m_bBroken)
      throw CUDTException(2, 1, 0);
   else if (!m_bConnected)
      throw CUDTException(2, 2, 0);

   if (size <= 0)
      return 0;

   // lazy snd thread creation
   #ifndef WIN32
      if (!m_bSndThrStart)
   #else
      if (NULL == m_SndThread)
   #endif
   {
      m_pSndTimeWindow = new CPktTimeWindow();

      #ifndef WIN32
         if (0 != pthread_create(&m_SndThread, NULL, CUDT::sndHandler, this))
         {
            delete m_pSndTimeWindow;
            m_pSndTimeWindow = NULL;
            throw CUDTException(3, 1, errno);
         }
         m_bSndThrStart = true;
      #else
         DWORD threadID;
         if (NULL == (m_SndThread = CreateThread(NULL, 0, CUDT::sndHandler, this, 0, &threadID)))
         {
            delete m_pSndTimeWindow;
            m_pSndTimeWindow = NULL;
            throw CUDTException(3, 1, GetLastError());
         }
      #endif
   }

   char* tempbuf = NULL;
   int64_t tosend = size;
   int unitsize;

   // positioning...
   try
   {
      ifs.seekg((streamoff)offset);
   }
   catch (...)
   {
      throw CUDTException(4, 1);
   }

   // sending block by block
   while (tosend > 0)
   {
      unitsize = (tosend >= block) ? block : tosend;

      try
      {
         tempbuf = new char[unitsize];
      }
      catch (...)
      {
         throw CUDTException(3, 2, 0);
      }

      try
      {
         ifs.read(tempbuf, unitsize);
      }
      catch (...)
      {
         delete [] tempbuf;
         throw CUDTException(4, 2);
      }

      #ifndef WIN32
         while (!m_bBroken && m_bConnected && (m_pSndBuffer->getCurrBufSize() >= m_iSndQueueLimit))
            usleep(10);
         m_pSndBuffer->addBuffer(tempbuf, unitsize, 0, CSndBuffer::releaseBuffer, NULL);

         pthread_mutex_lock(&m_SendDataLock);
         pthread_cond_signal(&m_SendDataCond);
         pthread_mutex_unlock(&m_SendDataLock);
      #else
         while (!m_bBroken && m_bConnected && (m_pSndBuffer->getCurrBufSize() >= m_iSndQueueLimit))
            Sleep(1);
         m_pSndBuffer->addBuffer(tempbuf, unitsize, 0, CSndBuffer::releaseBuffer, NULL);

         SetEvent(m_SendDataCond);
      #endif

      if (m_bBroken)
         throw CUDTException(2, 1, 0);

      tosend -= unitsize;
   }

   // Wait until all the data is sent out
   while ((!m_bBroken) && m_bConnected && (m_pSndBuffer->getCurrBufSize() > 0))
      #ifndef WIN32
         usleep(10);
      #else
         Sleep(1);
      #endif

   if (m_bBroken && (m_pSndBuffer->getCurrBufSize() > 0))
      throw CUDTException(2, 1, 0);

   return size;
}

int64_t CUDT::recvfile(ofstream& ofs, const int64_t& offset, const int64_t& size, const int& block)
{
   if (SOCK_DGRAM == m_iSockType)
      throw CUDTException(5, 10, 0);

   if ((m_bBroken) && (0 == m_pRcvBuffer->getRcvDataSize()))
      throw CUDTException(2, 1, 0);
   else if (!m_bConnected)
      throw CUDTException(2, 2, 0);

   if (size <= 0)
      return 0;

   char* tempbuf = NULL;
   int64_t torecv = size;
   int unitsize = block;
   int recvsize;

   try
   {
      tempbuf = new char[unitsize];
   }
   catch (...)
   {
      throw CUDTException(3, 2, 0);
   }

   // "recvfile" is always blocking.   
   bool syn = m_bSynRecving;
   m_bSynRecving = true;

   // positioning...
   try
   {
      ofs.seekp((streamoff)offset);
   }
   catch (...)
   {
      delete [] tempbuf;
      throw CUDTException(4, 3);
   }

   int overlapid;

   // receiving...
   while (torecv > 0)
   {
      unitsize = (torecv >= block) ? block : torecv;

      try
      {
         recvsize = recv(tempbuf, unitsize, &overlapid);
         ofs.write(tempbuf, recvsize);

         if (recvsize < unitsize)
         {
            m_bSynRecving = syn;
            delete [] tempbuf;
            return size - torecv + recvsize;
         }
      }
      catch (CUDTException e)
      {
         delete [] tempbuf;
         throw e;
      }
      catch (...)
      {
         delete [] tempbuf;
         throw CUDTException(4, 4);
      }

      torecv -= unitsize;
   }

   // recover the original receiving mode
   m_bSynRecving = syn;

   delete [] tempbuf;

   return size;
}

void CUDT::sample(CPerfMon* perf, bool clear)
{
   timeval currtime;
   gettimeofday(&currtime, 0);

   perf->msTimeStamp = (currtime.tv_sec - m_StartTime.tv_sec) * 1000 + (currtime.tv_usec - m_StartTime.tv_usec) / 1000;

   m_llSentTotal += m_llTraceSent;
   m_llRecvTotal += m_llTraceRecv;
   m_iSndLossTotal += m_iTraceSndLoss;
   m_iRcvLossTotal += m_iTraceRcvLoss;
   m_iRetransTotal += m_iTraceRetrans;
   m_iSentACKTotal += m_iSentACK;
   m_iRecvACKTotal += m_iRecvACK;
   m_iSentNAKTotal += m_iSentNAK;
   m_iRecvNAKTotal += m_iRecvNAK;

   perf->pktSentTotal = m_llSentTotal;
   perf->pktRecvTotal = m_llRecvTotal;
   perf->pktSndLossTotal = m_iSndLossTotal;
   perf->pktRcvLossTotal = m_iRcvLossTotal;
   perf->pktRetransTotal = m_iRetransTotal;
   perf->pktSentACKTotal = m_iSentACKTotal;
   perf->pktRecvACKTotal = m_iRecvACKTotal;
   perf->pktSentNAKTotal = m_iSentNAKTotal;
   perf->pktRecvNAKTotal = m_iRecvNAKTotal;

   perf->pktSent = m_llTraceSent;
   perf->pktRecv = m_llTraceRecv;
   perf->pktSndLoss = m_iTraceSndLoss;
   perf->pktRcvLoss = m_iTraceRcvLoss;
   perf->pktRetrans = m_iTraceRetrans;
   perf->pktSentACK = m_iSentACK;
   perf->pktRecvACK = m_iRecvACK;
   perf->pktSentNAK = m_iSentNAK;
   perf->pktRecvNAK = m_iRecvNAK;

   double interval = (currtime.tv_sec - m_LastSampleTime.tv_sec) * 1000000.0 + currtime.tv_usec - m_LastSampleTime.tv_usec;

   perf->mbpsSendRate = double(m_llTraceSent) * m_iPayloadSize * 8.0 / interval;
   perf->mbpsRecvRate = double(m_llTraceRecv) * m_iPayloadSize * 8.0 / interval;

   perf->usPktSndPeriod = m_ullInterval / double(m_ullCPUFrequency);
   perf->pktFlowWindow = m_iFlowWindowSize;
   perf->pktCongestionWindow = (int)m_dCongestionWindow;
   perf->pktFlightSize = CSeqNo::seqlen(const_cast<int32_t&>(m_iSndLastAck), m_iSndCurrSeqNo);
   perf->msRTT = m_iRTT/1000.0;
   perf->mbpsBandwidth = m_iBandwidth * m_iPayloadSize * 8.0 / 1000000.0;

   #ifndef WIN32
      if (0 == pthread_mutex_trylock(&m_ConnectionLock))
   #else
      if (WAIT_OBJECT_0 == WaitForSingleObject(m_ConnectionLock, 0))
   #endif
   {
      perf->byteAvailSndBuf = (NULL == m_pSndBuffer) ? 0 : m_iSndQueueLimit - m_pSndBuffer->getCurrBufSize();
      perf->byteAvailRcvBuf = (NULL == m_pRcvBuffer) ? 0 : m_pRcvBuffer->getAvailBufSize();

      #ifndef WIN32
         pthread_mutex_unlock(&m_ConnectionLock);
      #else
         ReleaseMutex(m_ConnectionLock);
      #endif
   }
   else
   {
      perf->byteAvailSndBuf = 0;
      perf->byteAvailRcvBuf = 0;
   }

   if (clear)
   {
      m_llTraceSent = m_llTraceRecv = m_iTraceSndLoss = m_iTraceSndLoss = m_iTraceRetrans = m_iSentACK = m_iRecvACK = m_iSentNAK = m_iRecvNAK = 0;
      m_LastSampleTime = currtime;
   }
}

void CUDT::initSynch()
{
   #ifndef WIN32
      pthread_mutex_init(&m_SendDataLock, NULL);
      pthread_cond_init(&m_SendDataCond, NULL);
      pthread_mutex_init(&m_SendBlockLock, NULL);
      pthread_cond_init(&m_SendBlockCond, NULL);
      pthread_mutex_init(&m_RecvDataLock, NULL);
      pthread_cond_init(&m_RecvDataCond, NULL);
      pthread_mutex_init(&m_OverlappedRecvLock, NULL);
      pthread_cond_init(&m_OverlappedRecvCond, NULL);
      pthread_mutex_init(&m_SendLock, NULL);
      pthread_mutex_init(&m_RecvLock, NULL);
      pthread_mutex_init(&m_AckLock, NULL);
      pthread_mutex_init(&m_ConnectionLock, NULL);
      pthread_mutex_init(&m_WindowLock, NULL);
      pthread_cond_init(&m_WindowCond, NULL);
      pthread_mutex_init(&m_HandleLock, NULL);
   #else
      m_SendDataLock = CreateMutex(NULL, false, NULL);
      m_SendDataCond = CreateEvent(NULL, false, false, NULL);
      m_SendBlockLock = CreateMutex(NULL, false, NULL);
      m_SendBlockCond = CreateEvent(NULL, false, false, NULL);
      m_RecvDataLock = CreateMutex(NULL, false, NULL);
      m_RecvDataCond = CreateEvent(NULL, false, false, NULL);
      m_OverlappedRecvLock = CreateMutex(NULL, false, NULL);
      m_OverlappedRecvCond = CreateEvent(NULL, false, false, NULL);
      m_SendLock = CreateMutex(NULL, false, NULL);
      m_RecvLock = CreateMutex(NULL, false, NULL);
      m_AckLock = CreateMutex(NULL, false, NULL);
      m_ConnectionLock = CreateMutex(NULL, false, NULL);
      m_WindowLock = CreateMutex(NULL, false, NULL);
      m_WindowCond = CreateEvent(NULL, false, false, NULL);
      m_HandleLock = CreateMutex(NULL, false, NULL);
   #endif
}

void CUDT::destroySynch()
{
   #ifndef WIN32
      pthread_mutex_destroy(&m_SendDataLock);
      pthread_cond_destroy(&m_SendDataCond);
      pthread_mutex_destroy(&m_SendBlockLock);
      pthread_cond_destroy(&m_SendBlockCond);
      pthread_mutex_destroy(&m_RecvDataLock);
      pthread_cond_destroy(&m_RecvDataCond);
      pthread_mutex_destroy(&m_OverlappedRecvLock);
      pthread_cond_destroy(&m_OverlappedRecvCond);
      pthread_mutex_destroy(&m_SendLock);
      pthread_mutex_destroy(&m_RecvLock);
      pthread_mutex_destroy(&m_AckLock);
      pthread_mutex_destroy(&m_ConnectionLock);
      pthread_mutex_destroy(&m_WindowLock);
      pthread_cond_destroy(&m_WindowCond);
      pthread_mutex_destroy(&m_HandleLock);
   #else
      CloseHandle(m_SendDataLock);
      CloseHandle(m_SendDataCond);
      CloseHandle(m_SendBlockLock);
      CloseHandle(m_SendBlockCond);
      CloseHandle(m_RecvDataLock);
      CloseHandle(m_RecvDataCond);
      CloseHandle(m_OverlappedRecvLock);
      CloseHandle(m_OverlappedRecvCond);
      CloseHandle(m_SendLock);
      CloseHandle(m_RecvLock);
      CloseHandle(m_AckLock);
      CloseHandle(m_ConnectionLock);
      CloseHandle(m_WindowLock);
      CloseHandle(m_WindowCond);
      CloseHandle(m_HandleLock);
   #endif
}

void CUDT::releaseSynch()
{
   #ifndef WIN32
      // wake up sending thread
      pthread_cond_signal(&m_WindowCond);

      pthread_mutex_lock(&m_SendDataLock);
      pthread_cond_signal(&m_SendDataCond);
      pthread_mutex_unlock(&m_SendDataLock);

      // wake up user calls
      pthread_mutex_lock(&m_SendBlockLock);
      pthread_cond_signal(&m_SendBlockCond);
      pthread_mutex_unlock(&m_SendBlockLock);

      pthread_mutex_lock(&m_SendLock);
      pthread_mutex_unlock(&m_SendLock);

      pthread_mutex_lock(&m_RecvDataLock);
      pthread_cond_signal(&m_RecvDataCond);
      pthread_mutex_unlock(&m_RecvDataLock);

      pthread_mutex_lock(&m_OverlappedRecvLock);
      pthread_cond_signal(&m_OverlappedRecvCond);
      pthread_mutex_unlock(&m_OverlappedRecvLock);

      pthread_mutex_lock(&m_RecvLock);
      pthread_mutex_unlock(&m_RecvLock);
   #else
      SetEvent(m_WindowCond);
      SetEvent(m_SendDataCond);

      SetEvent(m_SendBlockCond);
      WaitForSingleObject(m_SendLock, INFINITE);
      ReleaseMutex(m_SendLock);
      SetEvent(m_RecvDataCond);
      SetEvent(m_OverlappedRecvCond);
      WaitForSingleObject(m_RecvLock, INFINITE);
      ReleaseMutex(m_RecvLock);
   #endif
}
