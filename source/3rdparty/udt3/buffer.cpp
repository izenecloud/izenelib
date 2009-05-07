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
This file contains the implementation of UDT sending and receiving buffer
management modules.

The sending buffer is a linked list of application data to be sent.
The receiving buffer is a logically circular memeory block.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 06/07/2007
*****************************************************************************/

#include <cstring>
#include <cmath>
#include "buffer.h"


CSndBuffer::CSndBuffer(const int& mss):
m_pBlock(NULL),
m_pLastBlock(NULL),
m_pCurrSendBlk(NULL),
m_pCurrAckBlk(NULL),
m_iCurrBufSize(0),
m_iCurrSendPnt(0),
m_iCurrAckPnt(0),
m_iNextMsgNo(0),
m_iMSS(mss)
{
   #ifndef WIN32
      pthread_mutex_init(&m_BufLock, NULL);
   #else
      m_BufLock = CreateMutex(NULL, false, NULL);
   #endif
}

CSndBuffer::~CSndBuffer()
{
   Block* pb = m_pBlock;

   // Release allocated data structure if there is any
   while (NULL != m_pBlock)
   {
      pb = pb->m_next;

      // process user data according with the routine provided by applications
      if (NULL != m_pBlock->m_pMemRoutine)
         m_pBlock->m_pMemRoutine(m_pBlock->m_pcData, m_pBlock->m_iLength, m_pBlock->m_pContext);

      delete m_pBlock;
      m_pBlock = pb;
   }

   #ifndef WIN32
      pthread_mutex_destroy(&m_BufLock);
   #else
      CloseHandle(m_BufLock);
   #endif
}

void CSndBuffer::addBuffer(const char* data, const int& len, const int& handle, const UDT_MEM_ROUTINE func, void* context, const int& ttl, const int32_t& seqno, const bool& order)
{
   CGuard bufferguard(m_BufLock);

   if (NULL == m_pBlock)
   {
      // Insert a block to the empty list   
  
      m_pBlock = new Block;
      m_pBlock->m_pcData = const_cast<char *>(data);
      m_pBlock->m_iLength = len;
      gettimeofday(&m_pBlock->m_OriginTime, 0);
      m_pBlock->m_iTTL = ttl;
      m_pBlock->m_iMsgNo = m_iNextMsgNo;
      m_pBlock->m_iSeqNo = seqno;
      m_pBlock->m_iInOrder = order;
      m_pBlock->m_iInOrder <<= 29;
      m_pBlock->m_iHandle = handle;
      m_pBlock->m_pMemRoutine = func;
      m_pBlock->m_pContext = context;
      m_pBlock->m_next = NULL;
      m_pLastBlock = m_pBlock;
      m_pCurrSendBlk = m_pBlock;
      m_iCurrSendPnt = 0;
      m_pCurrAckBlk = m_pBlock;
      m_iCurrAckPnt = 0;
   }
   else
   {
      // Insert a new block to the tail of the list

      int32_t lastseq = m_pLastBlock->m_iSeqNo;
      int offset = m_pLastBlock->m_iLength;

      m_pLastBlock->m_next = new Block;
      m_pLastBlock = m_pLastBlock->m_next;
      m_pLastBlock->m_pcData = const_cast<char *>(data);
      m_pLastBlock->m_iLength = len;
      gettimeofday(&m_pLastBlock->m_OriginTime, 0);
      m_pLastBlock->m_iTTL = ttl;
      m_pLastBlock->m_iMsgNo = m_iNextMsgNo;
      m_pLastBlock->m_iSeqNo = lastseq + (int32_t)ceil(double(offset) / m_iMSS);
      m_pLastBlock->m_iInOrder = order;
      m_pLastBlock->m_iInOrder <<= 29;
      m_pLastBlock->m_iHandle = handle;
      m_pLastBlock->m_pMemRoutine = func;
      m_pLastBlock->m_pContext = context;
      m_pLastBlock->m_next = NULL;
      if (NULL == m_pCurrSendBlk)
         m_pCurrSendBlk = m_pLastBlock;
   }

   m_iNextMsgNo = CMsgNo::incmsg(m_iNextMsgNo);

   m_iCurrBufSize += len;
}

int CSndBuffer::readData(char** data, const int& len, int32_t& msgno)
{
   CGuard bufferguard(m_BufLock);

   // No data to read
   if (NULL == m_pCurrSendBlk)
      return 0;

   // read data in the current sending block
   if (m_iCurrSendPnt + len < m_pCurrSendBlk->m_iLength)
   {
      *data = m_pCurrSendBlk->m_pcData + m_iCurrSendPnt;

      msgno = m_pCurrSendBlk->m_iMsgNo | m_pCurrSendBlk->m_iInOrder;
      if (0 == m_iCurrSendPnt)
         msgno |= 0x80000000;
      if (m_pCurrSendBlk->m_iLength == m_iCurrSendPnt + len)
         msgno |= 0x40000000;

      m_iCurrSendPnt += len;

      return len;
   }

   // Not enough data to read. 
   // Read an irregular packet and move the current sending block pointer to the next block
   int readlen = m_pCurrSendBlk->m_iLength - m_iCurrSendPnt;
   *data = m_pCurrSendBlk->m_pcData + m_iCurrSendPnt;

   if (0 == m_iCurrSendPnt)
      msgno = m_pCurrSendBlk->m_iMsgNo | 0xC0000000 | m_pCurrSendBlk->m_iInOrder;
   else
      msgno = m_pCurrSendBlk->m_iMsgNo | 0x40000000 | m_pCurrSendBlk->m_iInOrder;

   m_pCurrSendBlk = m_pCurrSendBlk->m_next;
   m_iCurrSendPnt = 0;

   return readlen;
}

int CSndBuffer::readData(char** data, const int offset, const int& len, int32_t& msgno, int32_t& seqno, int& msglen)
{
   CGuard bufferguard(m_BufLock);

   Block* p = m_pCurrAckBlk;

   // No data to read
   if (NULL == p)
      return 0;

   // Locate to the data position by the offset
   int loffset = offset + m_iCurrAckPnt;
   while (p->m_iLength <= loffset)
   {
      loffset -= p->m_iLength;
      loffset -= len - ((0 == p->m_iLength % len) ? len : (p->m_iLength % len));
      p = p->m_next;
      if (NULL == p)
         return 0;
   }

   if (p->m_iTTL >= 0)
   {
      timeval currtime;
      gettimeofday(&currtime, 0);

      int e = (currtime.tv_sec - p->m_OriginTime.tv_sec) * 1000000 + currtime.tv_usec - p->m_OriginTime.tv_usec;

      if (e > p->m_iTTL)
      {
         msgno = p->m_iMsgNo;
         seqno = p->m_iSeqNo;
         msglen = p->m_iLength;

         return -1;
      }
   }

   // Read a regular data
   if (loffset + len <= p->m_iLength)
   {
      *data = p->m_pcData + loffset;
      msgno = p->m_iMsgNo | p->m_iInOrder;

      if (0 == loffset)
         msgno |= 0x80000000;
      if (p->m_iLength == loffset + len)
         msgno |= 0x40000000;

      return len;
   }

   // Read an irrugular data at the end of a block
   *data = p->m_pcData + loffset;
   msgno = p->m_iMsgNo | p->m_iInOrder;

   if (0 == loffset)
      msgno |= 0xC0000000;
   else
      msgno |= 0x40000000;

   return p->m_iLength - loffset;
}

void CSndBuffer::ackData(const int& len, const int& payloadsize)
{
   CGuard bufferguard(m_BufLock);

   m_iCurrAckPnt += len;

   // Remove the block if it is acknowledged
   while (m_iCurrAckPnt >= m_pCurrAckBlk->m_iLength)
   {
      m_iCurrAckPnt -= m_pCurrAckBlk->m_iLength;

      // Update the size error between regular and irregular packets
      if (0 != m_pCurrAckBlk->m_iLength % payloadsize)
         m_iCurrAckPnt -= payloadsize - m_pCurrAckBlk->m_iLength % payloadsize;

      m_iCurrBufSize -= m_pCurrAckBlk->m_iLength;
      m_pCurrAckBlk = m_pCurrAckBlk->m_next;

      // process user data according with the routine provided by applications
      if (NULL != m_pBlock->m_pMemRoutine)
         m_pBlock->m_pMemRoutine(m_pBlock->m_pcData, m_pBlock->m_iLength, m_pBlock->m_pContext);

      delete m_pBlock;
      m_pBlock = m_pCurrAckBlk;

      if (NULL == m_pBlock)
         break;
   }
}

int CSndBuffer::getCurrBufSize() const
{
   return m_iCurrBufSize - m_iCurrAckPnt;
}

bool CSndBuffer::getOverlappedResult(const int& handle, int& progress)
{
   CGuard bufferguard(m_BufLock);

   if (NULL != m_pCurrAckBlk)
   {
      if (handle == m_pCurrAckBlk->m_iHandle)
      {
         progress = m_iCurrAckPnt;
         return false;
      }
      else 
      {
         if (((m_pLastBlock->m_iHandle < m_pCurrAckBlk->m_iHandle) && (handle < m_pCurrAckBlk->m_iHandle) && (m_pLastBlock->m_iHandle <= handle))
            || ((m_pLastBlock->m_iHandle > m_pCurrAckBlk->m_iHandle) && ((handle < m_pCurrAckBlk->m_iHandle) || (m_pLastBlock->m_iHandle <= handle))))
         {
            progress = 0;
            return false;
         }
      }
   }

   progress = 0;
   return true;
}

void CSndBuffer::releaseBuffer(char* buf, int, void*)
{
   delete [] buf;
}

////////////////////////////////////////////////////////////////////////////////

CRcvBuffer::CRcvBuffer(const int& mss):
m_pcData(NULL),
m_iSize(40960000),
m_iStartPos(0),
m_iLastAckPos(0),
m_iMaxOffset(0),
m_pcUserBuf(NULL),
m_iUserBufSize(0),
m_pPendingBlock(NULL),
m_pLastBlock(NULL),
m_iPendingSize(0),
m_pMessageList(NULL),
m_iMSS(mss)
{
   m_pcData = new char [m_iSize];

   #ifndef WIN32
      pthread_mutex_init(&m_MsgLock, NULL);
   #else
      m_MsgLock = CreateMutex(NULL, false, NULL);
   #endif
}

CRcvBuffer::CRcvBuffer(const int& mss, const int& bufsize):
m_pcData(NULL),
m_iSize(bufsize),
m_iStartPos(0),
m_iLastAckPos(0),
m_iMaxOffset(0),
m_pcUserBuf(NULL),
m_iUserBufSize(0),
m_pPendingBlock(NULL),
m_pLastBlock(NULL),
m_iPendingSize(0),
m_pMessageList(NULL),
m_iMSS(mss)
{
   m_pcData = new char [m_iSize];

   #ifndef WIN32
      pthread_mutex_init(&m_MsgLock, NULL);
   #else
      m_MsgLock = CreateMutex(NULL, false, NULL);
   #endif
}

CRcvBuffer::~CRcvBuffer()
{
   delete [] m_pcData;

   Block* p = m_pPendingBlock;

   while (NULL != p)
   {
     m_pPendingBlock = m_pPendingBlock->m_next;
     delete p;
     p = m_pPendingBlock;
   }

   if (NULL != m_pMessageList)
      delete [] m_pMessageList;

   #ifndef WIN32
      pthread_mutex_destroy(&m_MsgLock);
   #else
      CloseHandle(m_MsgLock);
   #endif
}

bool CRcvBuffer::nextDataPos(char** data, int offset, const int& len)
{
   // Search the user data block first
   if (NULL != m_pcUserBuf)
   {
      if (m_iUserBufAck + offset + len <= m_iUserBufSize)
      {
         // find a position in user buffer
         *data = m_pcUserBuf + m_iUserBufAck + offset;
         return true;
      }
      else if (m_iUserBufAck + offset < m_iUserBufSize)
      {
         // Meet the end of the user buffer and there is not enough space for a regular packet
         return false;
      }
      else
         // offset is larger than user buffer size
         offset -= m_iUserBufSize - m_iUserBufAck;
   }

   // Remember the position of the furthest "dirty" data
   int origoff = m_iMaxOffset;
   if (offset + len > m_iMaxOffset)
      m_iMaxOffset = offset + len;

   if (m_iLastAckPos >= m_iStartPos)
   {
      if (m_iLastAckPos + offset + len <= m_iSize)
      {
         *data = m_pcData + m_iLastAckPos + offset;
         return true;
      }
      else if ((m_iLastAckPos + offset > m_iSize) && (m_iLastAckPos + offset - m_iSize + len < m_iStartPos))
      {
         *data = m_pcData + offset - (m_iSize - m_iLastAckPos);
         return true;
      }
   }
   else if (m_iLastAckPos + offset + len < m_iStartPos)
   {
      *data = m_pcData + m_iLastAckPos + offset;
      return true;
   }

   // recover this pointer if no space is found
   m_iMaxOffset = origoff;

   return false;
}

bool CRcvBuffer::addData(char** data, int offset, int len)
{
   // Check the user buffer first
   if (NULL != m_pcUserBuf)
   {
      if (m_iUserBufAck + offset + len <= m_iUserBufSize)
      {
         // write data into the user buffer
         memcpy(m_pcUserBuf + m_iUserBufAck + offset, *data, len);
         return true;
      }
      else if (m_iUserBufAck + offset < m_iUserBufSize)
      {
         // write part of the data to the user buffer
         memcpy(m_pcUserBuf + m_iUserBufAck + offset, *data, m_iUserBufSize - (m_iUserBufAck + offset));
         *data += m_iUserBufSize - (m_iUserBufAck + offset);
         len -= m_iUserBufSize - (m_iUserBufAck + offset);
         offset = 0;
      }
      else
         // offset is larger than size of user buffer
         offset -= m_iUserBufSize - m_iUserBufAck;
   }

   // Record this value in case that the method is failed
   int origoff = m_iMaxOffset;
   if (offset + len > m_iMaxOffset)
      m_iMaxOffset = offset + len;

   if (m_iLastAckPos >= m_iStartPos)
   {
      if (m_iLastAckPos + offset + len <= m_iSize)
      {
         memcpy(m_pcData + m_iLastAckPos + offset, *data, len);
         *data = m_pcData + m_iLastAckPos + offset;
         return true;
      }
      else if ((m_iLastAckPos + offset < m_iSize) && (m_iLastAckPos + offset + len - m_iSize < m_iStartPos))
      {
         memcpy(m_pcData + m_iLastAckPos + offset, *data, m_iSize - m_iLastAckPos - offset);
         memcpy(m_pcData, *data + m_iSize - m_iLastAckPos - offset, m_iLastAckPos + offset + len - m_iSize);
         *data = m_pcData + m_iLastAckPos + offset;
         return true;
      }
      else if ((m_iLastAckPos + offset >= m_iSize) && (m_iLastAckPos + offset + len - m_iSize < m_iStartPos))
      {
         memcpy(m_pcData + m_iLastAckPos + offset - m_iSize, *data, len);
         *data = m_pcData + m_iLastAckPos + offset - m_iSize;
         return true;
      }
   }
   else if (m_iLastAckPos + offset + len < m_iStartPos)
   {
      memcpy(m_pcData + m_iLastAckPos + offset, *data, len);
      *data = m_pcData + m_iLastAckPos + offset;
      return true;
   }

   // recover the offset pointer since the write is failed
   m_iMaxOffset = origoff;

   return false;
}

void CRcvBuffer::moveData(int offset, const int& len)
{
   // check the user buffer first
   if (NULL != m_pcUserBuf)
   {
      if (m_iUserBufAck + offset + len < m_iUserBufSize)
      {
         // move data in user buffer
         memmove(m_pcUserBuf + m_iUserBufAck + offset, m_pcUserBuf + m_iUserBufAck + offset + len, m_iUserBufSize - (m_iUserBufAck + offset + len));

         // move data from protocol buffer
         if (m_iMaxOffset > 0)
         {
            int reallen = len;
            if (m_iMaxOffset < len)
               reallen = m_iMaxOffset;

            if (m_iSize < m_iLastAckPos + reallen)
            {
               memcpy(m_pcUserBuf + m_iUserBufSize - len, m_pcData + m_iLastAckPos, m_iSize - m_iLastAckPos);
               memcpy(m_pcUserBuf + m_iUserBufSize - len + m_iSize - m_iLastAckPos, m_pcData, m_iLastAckPos + reallen - m_iSize);
            }
            else
               memcpy(m_pcUserBuf + m_iUserBufSize - len, m_pcData + m_iLastAckPos, reallen);
         }

         offset = 0; 
      }
      else if (m_iUserBufAck + offset < m_iUserBufSize)
      {
         if (m_iMaxOffset > m_iUserBufAck + offset + len - m_iUserBufSize)
         {
            int reallen = m_iUserBufSize - (m_iUserBufAck + offset);
            int startpos = m_iLastAckPos + len - reallen;
            if (m_iMaxOffset < len)
               reallen -= len - m_iMaxOffset;

            // Be sure that the m_iSize is at least 1 packet size, whereas len cannot be greater than this value, checked in setOpt().
            if (m_iSize < startpos)
               memcpy(m_pcUserBuf + m_iUserBufAck + offset, m_pcData + startpos - m_iSize, reallen);
            else if (m_iSize < startpos + reallen)
            {
               memcpy(m_pcUserBuf + m_iUserBufAck + offset, m_pcData + startpos, m_iSize - startpos);
               memcpy(m_pcUserBuf + m_iUserBufAck + offset + m_iSize - startpos, m_pcData, startpos + reallen - m_iSize);
            }
            else
               memcpy(m_pcUserBuf + m_iUserBufAck + offset, m_pcData + startpos, reallen);
         }

         offset = 0;
      }
      else
         // offset is larger than size of user buffer
         offset -= m_iUserBufSize - m_iUserBufAck;
   }

   // No data to move
   if (m_iMaxOffset - offset < len)
   {
      m_iMaxOffset = offset;
      return;
   }

   // Move data in protocol buffer.
   if (m_iLastAckPos + m_iMaxOffset <= m_iSize)
      memmove(m_pcData + m_iLastAckPos + offset, m_pcData + m_iLastAckPos + offset + len, m_iMaxOffset - offset - len);
   else if (m_iLastAckPos + offset >= m_iSize)
      memmove(m_pcData + m_iLastAckPos + offset - m_iSize, m_pcData + m_iLastAckPos + offset + len - m_iSize, m_iMaxOffset - offset - len);
   else if (m_iLastAckPos + offset + len <= m_iSize)
   {
      memmove(m_pcData + m_iLastAckPos + offset, m_pcData + m_iLastAckPos + offset + len, m_iSize - m_iLastAckPos - offset - len);
      if (m_iLastAckPos + m_iMaxOffset - m_iSize > len)
      {
         memmove(m_pcData + m_iSize - len, m_pcData, len);
         memmove(m_pcData, m_pcData + len, m_iLastAckPos + m_iMaxOffset - m_iSize - len);
      }
      else
      {
         memmove(m_pcData + m_iSize - len, m_pcData, m_iLastAckPos + m_iMaxOffset - m_iSize);
      }
   }
   else
   {
      memmove(m_pcData + m_iLastAckPos + offset, m_pcData + len - (m_iSize - m_iLastAckPos - offset), m_iSize - m_iLastAckPos - offset);
      memmove(m_pcData, m_pcData + len, m_iLastAckPos + m_iMaxOffset - m_iSize - len);
   }

   // Update the offset pointer
   m_iMaxOffset -= len;
}

bool CRcvBuffer::readBuffer(char* data, const int& len)
{
   if (m_iStartPos + len <= m_iLastAckPos)
   {
      // Simplest situation, read "len" data from start position
      memcpy(data, m_pcData + m_iStartPos, len);
      m_iStartPos += len;
      return true;
   }
   else if (m_iLastAckPos < m_iStartPos)
   {
      if (m_iStartPos + len < m_iSize)
      {
         // Data is not cover the physical boundary of the buffer
         memcpy(data, m_pcData + m_iStartPos, len);
         m_iStartPos += len;
         return true;
      }
      if (len - (m_iSize - m_iStartPos) <= m_iLastAckPos)
      {
         // data length exceeds the physical boundary, read twice
         memcpy(data, m_pcData + m_iStartPos, m_iSize - m_iStartPos);
         memcpy(data + m_iSize - m_iStartPos, m_pcData, len - (m_iSize - m_iStartPos));
         m_iStartPos = len - (m_iSize - m_iStartPos);
         return true;
      }
   }

   // No enough data to read
   return false;
}

int CRcvBuffer::ackData(const int& len)
{
   int ret = 0;

   if (NULL != m_pcUserBuf)
      if (m_iUserBufAck + len < m_iUserBufSize)
      {
         // update user buffer ACK pointer
         m_iUserBufAck += len;
         return 0;
      }
      else
      {
         // user buffer is fulfilled
         // update protocol ACK pointer
         m_iLastAckPos += m_iUserBufAck + len - m_iUserBufSize;
         m_iMaxOffset -= m_iUserBufAck + len - m_iUserBufSize;

         // process received data using user-defined function
         if (NULL != m_pMemRoutine)
            m_pMemRoutine(m_pcUserBuf, m_iUserBufSize, m_pContext);

         // the overlapped IO is completed, a pending buffer should be activated
         m_pcUserBuf = NULL;
         m_iUserBufSize = 0;
         if (NULL != m_pPendingBlock)
         {
            //TODO
            // release as many buffer as possible, until there is no enough data left for one buffer
            // change return value to int, and signal as many waiting recv() call

            registerUserBuf(m_pPendingBlock->m_pcData, m_pPendingBlock->m_iLength, m_pPendingBlock->m_iHandle, m_pPendingBlock->m_pMemRoutine, m_pPendingBlock->m_pContext);
            m_iPendingSize -= m_pPendingBlock->m_iLength;
            m_pPendingBlock = m_pPendingBlock->m_next;
            if (NULL == m_pPendingBlock)
               m_pLastBlock = NULL;
         }

         // returned value is 1 means user buffer is fulfilled
         ret = 1;
      }
   else
   {
      // there is no user buffer
      m_iLastAckPos += len;
      m_iMaxOffset -= len;
   }

   m_iLastAckPos %= m_iSize;

   return ret;
}

int CRcvBuffer::registerUserBuf(char* buf, const int& len, const int& handle, const UDT_MEM_ROUTINE func, void* context)
{
   if (NULL != m_pcUserBuf)
   {
      // there is ongoing recv, new buffer is put into pending list.

      Block *nb = new Block;
      nb->m_pcData = buf;
      nb->m_iLength = len;
      nb->m_iHandle = handle;
      nb->m_pMemRoutine = func;
      nb->m_pContext = context;
      nb->m_next = NULL;

      if (NULL == m_pPendingBlock)
         m_pLastBlock = m_pPendingBlock = nb;
      else
      {
         m_pLastBlock->m_next = nb;
         m_pLastBlock = nb;
      }

      m_iPendingSize += len;

      return 0;
   }

   m_iUserBufAck = 0;
   m_iUserBufSize = len;
   m_pcUserBuf = buf;
   m_iHandle = handle;
   m_pMemRoutine = func;
   m_pContext = context;

   // find the furthest "dirty" data that need to be copied
   int currwritepos = (m_iLastAckPos + m_iMaxOffset) % m_iSize;

   // copy data from protocol buffer into user buffer
   if (m_iStartPos <= currwritepos)
      if (currwritepos - m_iStartPos <= len)
      {
         memcpy(m_pcUserBuf, m_pcData + m_iStartPos, currwritepos - m_iStartPos);
         m_iMaxOffset = 0;
      }
      else
      {
         memcpy(m_pcUserBuf, m_pcData + m_iStartPos, len);
         m_iMaxOffset -= m_iStartPos + len - m_iLastAckPos;
      }
   else
      if (m_iSize + currwritepos - m_iStartPos <= len)
      {
         memcpy(m_pcUserBuf, m_pcData + m_iStartPos, m_iSize - m_iStartPos);
         memcpy(m_pcUserBuf + m_iSize - m_iStartPos, m_pcData, currwritepos);
         m_iMaxOffset = 0;
      }
      else
      {
         if (m_iSize - m_iStartPos <= len)
         {
            memcpy(m_pcUserBuf, m_pcData + m_iStartPos, m_iSize - m_iStartPos);
            memcpy(m_pcUserBuf + m_iSize - m_iStartPos, m_pcData, len - (m_iSize - m_iStartPos));
         }
         else
            memcpy(m_pcUserBuf, m_pcData + m_iStartPos, len);

         m_iMaxOffset = m_iSize + currwritepos - m_iStartPos - len;
      }

   // Update the user buffer pointer
   if (m_iStartPos <= m_iLastAckPos)
      m_iUserBufAck += m_iLastAckPos - m_iStartPos;
   else
      m_iUserBufAck += m_iSize - m_iStartPos + m_iLastAckPos;

   // update the protocol buffer pointer, step up by "len"
   m_iStartPos = (m_iStartPos + len) % m_iSize;
   // assume there is no enough data for the user buffer, ie, ACK - Start < len
   m_iLastAckPos = m_iStartPos;

   return m_iUserBufAck;
}

void CRcvBuffer::removeUserBuf()
{
   m_pcUserBuf = NULL;
   m_iUserBufAck = 0;
}

int CRcvBuffer::getAvailBufSize() const
{
   int bs = m_iSize;

   bs -= m_iLastAckPos - m_iStartPos;

   if (m_iLastAckPos < m_iStartPos)
      bs -= m_iSize;

   if (NULL != m_pcUserBuf)
      bs += m_iUserBufSize - m_iUserBufAck;

   return bs;
}

int CRcvBuffer::getRcvDataSize() const
{
   return (m_iLastAckPos - m_iStartPos + m_iSize) % m_iSize;
}

bool CRcvBuffer::getOverlappedResult(const int& handle, int& progress)
{
   if ((NULL != m_pcUserBuf) && (handle == m_iHandle))
   {
      progress = m_iUserBufAck;
      return false;
   }

   progress = 0;

   if (NULL != m_pPendingBlock)
   {
      if (((m_pLastBlock->m_iHandle >= m_pPendingBlock->m_iHandle) && (m_pPendingBlock->m_iHandle <= handle) && (handle <= m_pLastBlock->m_iHandle))
         || ((m_pLastBlock->m_iHandle < m_pPendingBlock->m_iHandle) && ((m_pPendingBlock->m_iHandle <= handle) || (handle <= m_pLastBlock->m_iHandle))))
         return false;
   }

   return true;
}

int CRcvBuffer::getPendingQueueSize() const
{
   return m_iPendingSize + m_iUserBufSize;
}

void CRcvBuffer::initMsgList()
{
   // the message list should contain the maximum possible number of messages: when each packet is a message
   m_iMsgInfoSize = m_iSize / m_iMSS + 1;

   m_pMessageList = new MsgInfo[m_iMsgInfoSize];

   m_iPtrFirstMsg = -1;
   m_iPtrRecentACK = -1;
   m_iLastMsgNo = 0;
   m_iValidMsgCount = 0;

   for (int i = 0; i < m_iMsgInfoSize; ++ i)
   {
      m_pMessageList[i].m_pcData = NULL;
      m_pMessageList[i].m_iMsgNo = -1;
      m_pMessageList[i].m_iStartSeq = -1;
      m_pMessageList[i].m_iEndSeq = -1;
      m_pMessageList[i].m_iSizeDiff = 0;
      m_pMessageList[i].m_bValid = false;
      m_pMessageList[i].m_bDropped = false;
      m_pMessageList[i].m_bInOrder = false;
   }
}

void CRcvBuffer::checkMsg(const int& type, const int32_t& msgno, const int32_t& seqno, const char* ptr, const bool& inorder, const int& diff)
{
   CGuard msgguard(m_MsgLock);

   int pos;

   if (-1 == m_iPtrFirstMsg)
   {
      pos = m_iPtrFirstMsg = 0;
      m_iPtrRecentACK = -1;
   }
   else
   {
      pos = CMsgNo::msgoff(m_pMessageList[m_iPtrFirstMsg].m_iMsgNo, msgno);

      if (pos >= 0)
         pos = (m_iPtrFirstMsg + pos) % m_iMsgInfoSize;
      else
      {
         pos = (m_iPtrFirstMsg + pos + m_iMsgInfoSize) % m_iMsgInfoSize;
         m_iPtrFirstMsg = pos;
      }
   }

   MsgInfo* p = m_pMessageList + pos;

   p->m_iMsgNo = msgno;

   switch (type)
   {
   case 3: // 11
      // single packet message
      p->m_pcData = (char*)ptr;
      p->m_iStartSeq = p->m_iEndSeq = seqno;
      p->m_bInOrder = inorder;
      p->m_iSizeDiff = diff;

      break;

   case 2: // 10
      // first packet of the message
      p->m_pcData = (char*)ptr;
      p->m_iStartSeq = seqno;
      p->m_bInOrder = inorder;

      break;

   case 1: // 01
      // last packet of the message
      p->m_iEndSeq = seqno;
      p->m_iSizeDiff = diff;

      break;
   }

   // update the largest msg no so far
   if (CMsgNo::msgcmp(m_iLastMsgNo, msgno) < 0)
      m_iLastMsgNo = msgno;
}

bool CRcvBuffer::ackMsg(const int32_t& ack, const CRcvLossList* rll)
{
   CGuard msgguard(m_MsgLock);

   // no message exist, return
   if (-1 == m_iPtrFirstMsg)
   {
      // also means no message is valid

      m_iStartPos = m_iLastAckPos;

      return false;
   }

   int ptr;
   int len;

   if (-1 == m_iPtrRecentACK)
   {
      // all messages are new, check from the start
      ptr = m_iPtrFirstMsg;
      len = CMsgNo::msglen(m_pMessageList[ptr].m_iMsgNo, m_iLastMsgNo);
   }
   else
   {
      // check from the last ACK point
      ptr = m_iPtrRecentACK + 1;

      if (ptr == m_iMsgInfoSize)
         ptr = 0;

      len = CMsgNo::msglen(m_pMessageList[ptr].m_iMsgNo, m_iLastMsgNo);
   }

   for (int i = 0; i < len; ++ i)
   {
      if ((m_pMessageList[ptr].m_iStartSeq != -1) &&
          (m_pMessageList[ptr].m_iEndSeq != -1) &&
          (!m_pMessageList[ptr].m_bDropped) &&
          (!(rll->find(m_pMessageList[ptr].m_iStartSeq, m_pMessageList[ptr].m_iEndSeq))) &&
          (!m_pMessageList[ptr].m_bInOrder || CSeqNo::seqcmp(m_pMessageList[ptr].m_iEndSeq, ack) <= 0))
      {
         m_pMessageList[ptr].m_bValid = true;
         m_pMessageList[ptr].m_iLength = CSeqNo::seqlen(m_pMessageList[ptr].m_iStartSeq, m_pMessageList[ptr].m_iEndSeq) * m_iMSS - m_pMessageList[ptr].m_iSizeDiff;

         ++ m_iValidMsgCount;
      }

      if ((m_pMessageList[ptr].m_iEndSeq != -1) && (CSeqNo::seqcmp(m_pMessageList[ptr].m_iEndSeq, ack) <= 0))
         m_iPtrRecentACK = ptr;

      ++ ptr;

      if (ptr == m_iMsgInfoSize)
         ptr = 0;
   }

   return (m_iValidMsgCount > 0);
}

void CRcvBuffer::dropMsg(const int32_t& msgno)
{
   CGuard msgguard(m_MsgLock);

   // no message exist, return
   if (-1 == m_iPtrFirstMsg)
      return;

   int ptr = m_iPtrFirstMsg + CMsgNo::msglen(m_pMessageList[m_iPtrFirstMsg].m_iMsgNo, msgno);
   if (ptr >= m_iMsgInfoSize)
      ptr -= m_iMsgInfoSize;

   m_pMessageList[ptr].m_iMsgNo = msgno;
   m_pMessageList[ptr].m_bDropped = true;

   // update the largest msg no so far
   if (CMsgNo::msgcmp(m_iLastMsgNo, msgno) < 0)
      m_iLastMsgNo = msgno;
}

int CRcvBuffer::readMsg(char* data, const int& len)
{
   CGuard msgguard(m_MsgLock);

   // no message exist, return
   if ((-1 == m_iPtrFirstMsg) || (-1 == m_iPtrRecentACK))
      return 0;

   int ptr = m_iPtrFirstMsg;

   // searching first valid message
   while (m_pMessageList[ptr].m_iMsgNo != m_iLastMsgNo)
   {
      if (m_pMessageList[ptr].m_bValid)
         break;

      ++ ptr;
      if (ptr == m_iMsgInfoSize)
         ptr = 0;
   }

   int size = 0;

   if (m_pMessageList[ptr].m_bValid)
   {
      if ((m_pMessageList[ptr].m_bInOrder) || (CSeqNo::seqcmp(m_pMessageList[ptr].m_iEndSeq, m_pMessageList[m_iPtrRecentACK].m_iEndSeq) <= 0))
      {
         m_iStartPos = m_pMessageList[ptr].m_pcData + CSeqNo::seqlen(m_pMessageList[ptr].m_iStartSeq, m_pMessageList[ptr].m_iEndSeq) * m_iMSS - m_pcData;
         if (m_iStartPos > m_iSize)
            m_iStartPos -= m_iSize;
      }
      else
      {
         if (NULL != m_pMessageList[m_iPtrRecentACK].m_pcData)
            m_iStartPos = m_pMessageList[m_iPtrRecentACK].m_pcData - m_pcData;
      }

      size = (len > m_pMessageList[ptr].m_iLength) ? m_pMessageList[ptr].m_iLength : len;

      if (m_pMessageList[ptr].m_pcData - m_pcData + size < m_iSize)
      {
         memcpy(data, m_pMessageList[ptr].m_pcData, size);
      }
      else
      {
         int partial = m_pMessageList[ptr].m_pcData - m_pcData + size - m_iSize;

         memcpy(data, m_pMessageList[ptr].m_pcData, size - partial);
         memcpy(data + size - partial, m_pcData, partial);
      }

      // already read, will not be read again
      m_pMessageList[ptr].m_bValid = false;
      // mark this msg as dropped so that it will not be set to valid again in checkMsg()
      m_pMessageList[ptr].m_bDropped = true;

      -- m_iValidMsgCount;
   }

   // all messages prior to the first valid message before the recent ACK point are permanently invalid
   if (CMsgNo::msgcmp(m_pMessageList[ptr].m_iMsgNo, m_pMessageList[m_iPtrRecentACK].m_iMsgNo) <= 0)
   {
      while (ptr != m_iPtrRecentACK)
      {
         ptr ++;
         if (ptr == m_iMsgInfoSize)
            ptr = 0;

         if (m_pMessageList[ptr].m_bValid)
            break;
      }
   }
   else 
      ptr = m_iPtrRecentACK;

   // release the invalid message items
   while (m_iPtrFirstMsg != ptr)
   {
      m_pMessageList[m_iPtrFirstMsg].m_pcData = NULL;
      m_pMessageList[m_iPtrFirstMsg].m_iMsgNo = -1;
      m_pMessageList[m_iPtrFirstMsg].m_iStartSeq = -1;
      m_pMessageList[m_iPtrFirstMsg].m_iEndSeq = -1;
      m_pMessageList[m_iPtrFirstMsg].m_iLength = -1;
      m_pMessageList[m_iPtrFirstMsg].m_bValid = false;
      m_pMessageList[m_iPtrFirstMsg].m_bDropped = false;
      m_pMessageList[m_iPtrFirstMsg].m_bInOrder = false;

      ++ m_iPtrFirstMsg;
      if (m_iPtrFirstMsg == m_iMsgInfoSize)
         m_iPtrFirstMsg = 0;
   }

   // all messages are invalid, re-init the message list
   if ((m_pMessageList[m_iPtrFirstMsg].m_iMsgNo == m_iLastMsgNo) && !(m_pMessageList[m_iPtrFirstMsg].m_bValid))
   {
      m_pMessageList[m_iPtrFirstMsg].m_pcData = NULL;
      m_pMessageList[m_iPtrFirstMsg].m_iMsgNo = -1;
      m_pMessageList[m_iPtrFirstMsg].m_iStartSeq = -1;
      m_pMessageList[m_iPtrFirstMsg].m_iEndSeq = -1;
      m_pMessageList[m_iPtrFirstMsg].m_iLength = -1;
      m_pMessageList[m_iPtrFirstMsg].m_bValid = false;
      m_pMessageList[m_iPtrFirstMsg].m_bDropped = false;
      m_pMessageList[m_iPtrFirstMsg].m_bInOrder = false;

      m_iPtrFirstMsg = -1;
      m_iPtrRecentACK = -1;

      m_iStartPos = m_iLastAckPos;
   }

   return size;
}

int CRcvBuffer::getValidMsgCount()
{
   CGuard msgguard(m_MsgLock);

   return m_iValidMsgCount;
}
