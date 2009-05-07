/*****************************************************************************
Copyright © 2001 - 2006, The Board of Trustees of the University of Illinois.
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
This header file contains the definition of UDT buffer structure and operations.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 09/07/2006
*****************************************************************************/

#ifndef __UDT_BUFFER_H__
#define __UDT_BUFFER_H__


#include "udt.h"
#include "list.h"


class CSndBuffer
{
public:
   CSndBuffer(const int& mss);
   ~CSndBuffer();

      // Functionality:
      //    Insert a user buffer into the sending list.
      // Parameters:
      //    0) [in] data: pointer to the user data block.
      //    1) [in] len: size of the block.
      //    2) [in] handle: handle of this request IO.
      //    3) [in] func: routine to process the buffer after IO completed.
      //    4) [in] context: context parameter for the buffer process routine
      //    5) [in] ttl: time to live in milliseconds
      //    6) [in] seqno: sequence number of the first packet in the block, for DGRAM only
      //    7) [in] order: if the block should be delivered in order, for DGRAM only
      // Returned value:
      //    None.

   void addBuffer(const char* data, const int& len, const int& handle, const UDT_MEM_ROUTINE func, void* context, const int& ttl = -1, const int32_t& seqno = 0, const bool& order = false);

      // Functionality:
      //    Find data position to pack a DATA packet from the furthest reading point.
      // Parameters:
      //    0) [out] data: the pointer to the data position.
      //    1) [in] len: Expected data length.
      //    2) [out] msgno: message number of the packet.
      // Returned value:
      //    Actual length of data read.

   int readData(char** data, const int& len, int32_t& msgno);

      // Functionality:
      //    Find data position to pack a DATA packet for a retransmission.
      // Parameters:
      //    0) [out] data: the pointer to the data position.
      //    1) [in] offset: offset from the last ACK point.
      //    2) [in] len: Expected data length.
      //    3) [out] msgno: message number of the packet.
      //    4) [out] seqno: sequence number of the first packet in the message
      //    5) [out] msglen: length of the message
      // Returned value:
      //    Actual length of data read.

   int readData(char** data, const int offset, const int& len, int32_t& msgno, int32_t& seqno, int& msglen);

      // Functionality:
      //    Update the ACK point and may release/unmap/return the user data according to the flag.
      // Parameters:
      //    0) [in] len: size of data acknowledged.
      //    1) [in] payloadsize: regular payload size that UDT always try to read.
      // Returned value:
      //    None.

   void ackData(const int& len, const int& payloadsize);

      // Functionality:
      //    Read size of data still in the sending list.
      // Parameters:
      //    None.
      // Returned value:
      //    Current size of the data in the sending list.

   int getCurrBufSize() const;

      // Functionality:
      //    Query the progress of the buffer sending identified by handle.
      // Parameters:
      //    1) [in] handle: descriptor of this overlapped IO
      //    2) [out] progress: the current progress of the overlapped IO
      // Returned value:
      //    if the overlapped IO is completed.

   bool getOverlappedResult(const int& handle, int& progress);

      // Functionality:
      //    helper function to release the user buffer.
      // Parameters:
      //    1) [in]: pointer to the buffer
      //    2) [in]: buffer size
      // Returned value:
      //    Current size of the data in the sending list

  static void releaseBuffer(char* buf, int, void*);

private:
   pthread_mutex_t m_BufLock;           // used to synchronize buffer operation

   struct Block
   {
      char* m_pcData;                   // pointer to the data block
      int m_iLength;                    // length of the block

      timeval m_OriginTime;             // original request time
      int m_iTTL;                       // time to live
      int32_t m_iMsgNo;                 // message number
      int32_t m_iSeqNo;                 // sequence number of first packet
      int m_iInOrder;                   // flag indicating if the block should be delivered in order

      int m_iHandle;                    // a unique handle to represent this senidng request
      UDT_MEM_ROUTINE m_pMemRoutine;    // function to process buffer after sending
      void* m_pContext;                 // context parameter for the memory processing routine

      Block* m_next;                    // next block
   } *m_pBlock, *m_pLastBlock, *m_pCurrSendBlk, *m_pCurrAckBlk;

   // m_pBlock:         The first block
   // m_pLastBlock:     The last block
   // m_pCurrSendBlk:   The block contains the data with the largest seq. no. that has been sent
   // m_pCurrAckBlk:    The block contains the data with the latest ACK (= m_pBlock)

   int m_iCurrBufSize;                  // Total size of the blocks
   int m_iCurrSendPnt;                  // pointer to the data with the largest current seq. no.
   int m_iCurrAckPnt;                   // pointer to the data with the latest ACK

   int32_t m_iNextMsgNo;                // next message number

   int m_iMSS;                          // maximum seqment/packet size
};

////////////////////////////////////////////////////////////////////////////////

class CRcvBuffer
{
public:
   CRcvBuffer(const int& mss);
   CRcvBuffer(const int& mss, const int& bufsize);
   ~CRcvBuffer();

      // Functionality:
      //    Find a position in the buffer to receive next packet.
      // Parameters:
      //    0) [out] data: the pointer to the next data position.
      //    1) [in] offset: offset from last ACK point.
      //    2) [in] len: size of data to be written.
      // Returned value:
      //    true if found, otherwise false.

   bool nextDataPos(char** data, int offset, const int& len);

      // Functionality:
      //    Write data into the buffer.
      // Parameters:
      //    0) [in/out] data: [in] pointer to data to be copied, [out] pointer to the protocol buffer location where the data is added.
      //    1) [in] offset: offset from last ACK point.
      //    2) [in] len: size of data to be written.
      // Returned value:
      //    true if a position that can hold the data is found, otherwise false.

   bool addData(char** data, int offset, int len);

      // Functionality:
      //    Move part of the data in buffer to the direction of the ACK point by some length.
      // Parameters:
      //    0) [in] offset: From where to move the data.
      //    1) [in] len: How much to move.
      // Returned value:
      //    None.

   void moveData(int offset, const int& len);

      // Functionality:
      //    Read data from the buffer into user buffer.
      // Parameters:
      //    0) [out] data: data read from protocol buffer.
      //    1) [in] len: size of data to be read.
      // Returned value:
      //    true if there is enough data to read, otherwise return false.

   bool readBuffer(char* data, const int& len);

      // Functionality:
      //    Update the ACK point of the buffer.
      // Parameters:
      //    0) [in] len: size of data to be acknowledged.
      // Returned value:
      //    1 if a user buffer is fulfilled, otherwise 0.

   int ackData(const int& len);

      // Functionality:
      //    Insert the user buffer into the protocol buffer.
      // Parameters:
      //    0) [in] buf: pointer to the user buffer.
      //    1) [in] len: size of the user buffer.
      //    2) [in] handle: descriptor of this overlapped receiving.
      //    3) [in] func: buffer process routine after an overlapped IO is completed.
      //    3) [in] context parameter for the buffer process routine.
      // Returned value:
      //    Size of data that has been received by now.

   int registerUserBuf(char* buf, const int& len, const int& handle, const UDT_MEM_ROUTINE func, void* context);

      // Functionality:
      //    remove the user buffer from the protocol buffer.
      // Parameters:
     //    None
      // Returned value:
      //    None.

   void removeUserBuf();

      // Functionality:
      //    Query how many buffer space left for data receiving.
      // Parameters:
      //    None.
      // Returned value:
      //    size of available buffer space (including user buffer) for data receiving.

   int getAvailBufSize() const;

      // Functionality:
      //    Query how many data has been continuously received (for reading).
      // Parameters:
      //    None.
      // Returned value:
      //    size of valid (continous) data for reading.

   int getRcvDataSize() const;

      // Functionality:
      //    Query the progress of the buffer sending identified by handle.
      // Parameters:
      //    1) [in] handle: descriptor of this overlapped IO
      //    2) [out] progress: the current progress of the overlapped IO
      // Returned value:
      //    if the overlapped IO is completed.

   bool getOverlappedResult(const int& handle, int& progress);

      // Functionality:
      //    Query the total size of overlapped recv buffers.
      // Parameters:
      //    None.
      // Returned value:
      //    Total size of the pending overlapped recv buffers.

   int getPendingQueueSize() const;

      // Functionality:
      //    Initialize the received message list.
      // Parameters:
      //    None.
      // Returned value:
      //    None.

   void initMsgList();

      // Functionality:
      //    Check the message boundaries.
      // Parameters:
      //    0) [in] type: boundary type: start and/or end.
      //    1) [in] msgno: message number
      //    2) [in] seqno: sequence number
      //    3) [in] ptr: pointer to the protocol buffer
      //    4) [in] diff: size difference of an irredular packet
      // Returned value:
      //    None.

   void checkMsg(const int& type, const int32_t& msgno, const int32_t& seqno, const char* ptr, const bool& inorder, const int& diff);

      // Functionality:
      //    acknowledgment check for the message list.
      // Parameters:
      //    0) [in] ackno: latest acknowledged sequence number.
      //    1) [in] rll: receiver's loss list
      // Returned value:
      //    None.

   bool ackMsg(const int32_t& ackno, const CRcvLossList* rll);

      // Functionality:
      //    mark the message to be dropped from the message list.
      // Parameters:
      //    0) [in] msgno: message nuumer.
      // Returned value:
      //    None.

   void dropMsg(const int32_t& msgno);

      // Functionality:
      //    read a message.
      // Parameters:
      //    0) [out] data: buffer to write the message into.
      //    1) [in] len: size of the buffer.
      // Returned value:
      //    actuall size of data read.

   int readMsg(char* data, const int& len);

      // Functionality:
      //    get the number of valid message currently available.
      // Parameters:
      //    None.
      // Returned value:
      //    number of valid message.

   int getValidMsgCount();


private:
   char* m_pcData;                      // pointer to the protocol buffer
   int m_iSize;                         // size of the protocol buffer

   int m_iStartPos;                     // the head position for I/O (inclusive)
   int m_iLastAckPos;                   // the last ACKed position (exclusive)
					// EMPTY: m_iStartPos = m_iLastAckPos   FULL: m_iStartPos = m_iLastAckPos + 1
   int m_iMaxOffset;                    // the furthest "dirty" position (absolute distance from m_iLastAckPos)

   char* m_pcUserBuf;                   // pointer to the user registered buffer
   int m_iUserBufSize;                  // size of the user buffer
   int m_iUserBufAck;                   // last ACKed position of the user buffer
   int m_iHandle;                       // unique handle to represet this IO request
   UDT_MEM_ROUTINE m_pMemRoutine;       // function to process user buffer after receiving
   void* m_pContext;                    // context parameter for the buffer processing routine

   struct Block
   {
      char* m_pcData;                   // pointer to the overlapped recv buffer
      int m_iLength;                    // length of the block

      int m_iHandle;                    // a unique handle to represent this receiving request
      UDT_MEM_ROUTINE m_pMemRoutine;    // function to process buffer after a complete receiving
      void* m_pContext;                 // context parameter for the buffer processing routine

      Block* m_next;                    // next block
   } *m_pPendingBlock, *m_pLastBlock;

   // m_pPendingBlock:                  // the list of pending overlapped recv buffers
   // m_pLastBlock:                     // the last block of pending buffers

   int m_iPendingSize;                  // total size of pending recv buffers

   struct MsgInfo
   {
      char* m_pcData;	                // location of the message in the protocol buffer
      int32_t m_iMsgNo;	                // message number
      int32_t m_iStartSeq;              // sequence number of the first packet in the message
      int32_t m_iEndSeq;                // sequence number of the last packet in the message
      int m_iSizeDiff;	                // the size difference of the last packet (that may be an irregular sized packet)
      int m_iLength;	                // length of this message
      bool m_bValid;                    // if the message is valid
      bool m_bDropped;                  // if the message is droped by the sender
      bool m_bInOrder;                  // if the message should be delivered in order
   } *m_pMessageList;                   // a list of the received message

   int m_iMsgInfoSize;	                // size of the message info list
   int m_iPtrFirstMsg;                  // pointer to the first message in the list
   int m_iPtrRecentACK;                 // the most recent ACK'ed message
   int32_t m_iLastMsgNo;                // the last msg no ever received

   pthread_mutex_t m_MsgLock;           // used to synchronize MsgInfo operation

   int m_iValidMsgCount;                // number valid message

   int m_iMSS;                          // maximum seqment/packet size
};


#endif
