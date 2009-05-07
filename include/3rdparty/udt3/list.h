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
This header file contains the definition of List structures used in UDT.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 03/23/2006
*****************************************************************************/

#ifndef __UDT_LIST_H__
#define __UDT_LIST_H__


#include "udt.h"
#include "common.h"


class CSndLossList
{
public:
   CSndLossList(const int& size);
   ~CSndLossList();

      // Functionality:
      //    Insert a seq. no. into the sender loss list.
      // Parameters:
      //    0) [in] seqno1: sequence number starts.
      //    1) [in] seqno2: sequence number ends.
      // Returned value:
      //    number of packets that are not in the list previously.

   int insert(const int32_t& seqno1, const int32_t& seqno2);

      // Functionality:
      //    Remove ALL the seq. no. that are not greater than the parameter.
      // Parameters:
      //    0) [in] seqno: sequence number.
      // Returned value:
      //    None.

   void remove(const int32_t& seqno);

      // Functionality:
      //    Read the loss length.
      // Parameters:
      //    None.
      // Returned value:
      //    The length of the list.

   int getLossLength();

      // Functionality:
      //    Read the first (smallest) loss seq. no. in the list and remove it.
      // Parameters:
      //    None.
      // Returned value:
      //    The seq. no. or -1 if the list is empty.

   int32_t getLostSeq();

private:
   int32_t* m_piData1;                  // sequence number starts
   int32_t* m_piData2;                  // seqnence number ends
   int* m_piNext;                       // next node in the list

   int m_iHead;                         // first node
   int m_iLength;                       // loss length
   int m_iSize;                         // size of the static array
   int m_iLastInsertPos;                // position of last insert node

   pthread_mutex_t m_ListLock;          // used to synchronize list operation
};

////////////////////////////////////////////////////////////////////////////////

class CRcvLossList
{
public:
   CRcvLossList(const int& size);
   ~CRcvLossList();

      // Functionality:
      //    Insert a series of loss seq. no. between "seqno1" and "seqno2" into the receiver's loss list.
      // Parameters:
      //    0) [in] seqno1: sequence number starts.
      //    1) [in] seqno2: seqeunce number ends.
      // Returned value:
      //    None.

   void insert(const int32_t& seqno1, const int32_t& seqno2);

      // Functionality:
      //    Remove a loss seq. no. from the receiver's loss list.
      // Parameters:
      //    0) [in] seqno: sequence number.
      // Returned value:
      //    if the packet is removed (true) or no such lost packet is found (false).

   bool remove(const int32_t& seqno);

      // Functionality:
      //    Remove all packets between seqno1 and seqno2.
      // Parameters:
      //    0) [in] seqno1: start sequence number.
      //    1) [in] seqno2: end sequence number.
      // Returned value:
      //    if the packet is removed (true) or no such lost packet is found (false).

   bool remove(const int32_t& seqno1, const int32_t& seqno2);

      // Functionality:
      //    Find if there is any lost packets whose sequence number falling seqno1 and seqno2.
      // Parameters:
      //    0) [in] seqno1: start sequence number.
      //    1) [in] seqno2: end sequence number.
      // Returned value:
      //    True if found; otherwise false.

   bool find(const int32_t& seqno1, const int32_t& seqno2) const;

      // Functionality:
      //    Read the loss length.
      // Parameters:
      //    None.
      // Returned value:
      //    the length of the list.

   int getLossLength() const;

      // Functionality:
      //    Read the first (smallest) seq. no. in the list.
      // Parameters:
      //    None.
      // Returned value:
      //    the sequence number or -1 if the list is empty.

   int getFirstLostSeq() const;

      // Functionality:
      //    Get a encoded loss array for NAK report.
      // Parameters:
      //    0) [out] array: the result list of seq. no. to be included in NAK.
      //    1) [out] physical length of the result array.
      //    2) [in] limit: maximum length of the array.
      //    3) [in] threshold: Time threshold from last NAK report.
      // Returned value:
      //    None.

   void getLossArray(int32_t* array, int& len, const int& limit, const int& threshold);

private:
   int32_t* m_piData1;                  // sequence number starts
   int32_t* m_piData2;                  // sequence number ends
   int* m_piNext;                       // next node in the list
   int* m_piPrior;                      // prior node in the list;

   int m_iHead;                         // first node in the list
   int m_iTail;                         // last node in the list;
   int m_iLength;                       // loss length
   int m_iSize;                         // size of the static array

   timeval m_TimeStamp;			// last list update time or NAK feedback time
};

////////////////////////////////////////////////////////////////////////////////

class CIrregularPktList
{
public:
   CIrregularPktList(const int& size);
   ~CIrregularPktList();

      // Functionality:
      //    Read the total size error of all the irregular packets prior to "seqno".
      // Parameters:
      //    0) [in] seqno: sequence number.
      // Returned value:
      //    the total size error of all the irregular packets prior to (excluding) "seqno".

   int currErrorSize(const int32_t& seqno) const;

      // Functionality:
      //    Insert an irregular packet into the list.
      // Parameters:
      //    0) [in] seqno: sequence number.
      //    1) [in] errsize: size error of the current packet.
      // Returned value:
      //    None

   void addIrregularPkt(const int32_t& seqno, const int& errsize);

      // Functionality:
      //    Remove ALL the packets prior to "seqno".
      // Parameters:
      //    0) [in] seqno: sequence number.
      // Returned value:
      //    None

   void deleteIrregularPkt(const int32_t& seqno);

private:
   int32_t* m_piData;                   // sequence number
   int* m_piErrorSize;                  // size error of the node
   int* m_piNext;                       // next node in the list

   int m_iHead;                         // first node in the list
   int m_iLength;                       // number of irregular packets in the list
   int m_iSize;                         // size of the static array
   int m_iInsertPos;                    // last node insert position
};


#endif
