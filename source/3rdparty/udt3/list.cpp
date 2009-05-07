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
This file contains the implementation of UDT loss lists and irregular packet
list management modules.

All the lists are static linked lists in ascending order of sequence numbers.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 03/23/2006
*****************************************************************************/

#include "list.h"


CSndLossList::CSndLossList(const int& size):
m_piData1(NULL),
m_piData2(NULL),
m_piNext(NULL),
m_iSize(size)
{
   m_piData1 = new int32_t [m_iSize];
   m_piData2 = new int32_t [m_iSize];
   m_piNext = new int [m_iSize];

   // -1 means there is no data in the node
   for (int i = 0; i < size; ++ i)
   {
      m_piData1[i] = -1;
      m_piData2[i] = -1;
   }

   m_iLength = 0;
   m_iHead = -1;
   m_iLastInsertPos = -1;

   // sender list needs mutex protection
   #ifndef WIN32
      pthread_mutex_init(&m_ListLock, 0);
   #else
      m_ListLock = CreateMutex(NULL, false, NULL);
   #endif
}

CSndLossList::~CSndLossList()
{
   delete [] m_piData1;
   delete [] m_piData2;
   delete [] m_piNext;

   #ifndef WIN32
      pthread_mutex_destroy(&m_ListLock);
   #else
      CloseHandle(m_ListLock);
   #endif
}

int CSndLossList::insert(const int32_t& seqno1, const int32_t& seqno2)
{
   CGuard listguard(m_ListLock);

   if (0 == m_iLength)
   {
      // insert data into an empty list

      m_iHead = 0;
      m_piData1[m_iHead] = seqno1;
      if (seqno2 != seqno1)
         m_piData2[m_iHead] = seqno2;

      m_piNext[m_iHead] = -1;
      m_iLastInsertPos = m_iHead;

      m_iLength += CSeqNo::seqlen(seqno1, seqno2);

      return m_iLength;
   }

   // otherwise find the position where the data can be inserted
   int origlen = m_iLength;
   int offset = CSeqNo::seqoff(m_piData1[m_iHead], seqno1);
   int loc = (m_iHead + offset + m_iSize) % m_iSize;

   if (offset < 0)
   {
      // Insert data prior to the head pointer

      m_piData1[loc] = seqno1;
      if (seqno2 != seqno1)
         m_piData2[loc] = seqno2;

      // new node becomes head
      m_piNext[loc] = m_iHead;
      m_iHead = loc;
      m_iLastInsertPos = loc;

      m_iLength += CSeqNo::seqlen(seqno1, seqno2);
   }
   else if (offset > 0)
   {
      if (seqno1 == m_piData1[loc])
      {
         m_iLastInsertPos = loc;

         // first seqno is equivlent, compare the second
         if (-1 == m_piData2[loc])
         {
            if (seqno2 != seqno1)
            {
               m_iLength += CSeqNo::seqlen(seqno1, seqno2) - 1;
               m_piData2[loc] = seqno2;
            }
         }
         else if (CSeqNo::seqcmp(seqno2, m_piData2[loc]) > 0)
         {
            // new seq pair is longer than old pair, e.g., insert [3, 7] to [3, 5], becomes [3, 7]
            m_iLength += CSeqNo::seqlen(m_piData2[loc], seqno2) - 1;
            m_piData2[loc] = seqno2;
         }
         else
            // Do nothing if it is already there
            return 0;
      }
      else
      {
         // searching the prior node
         int i;
         if ((-1 != m_iLastInsertPos) && (CSeqNo::seqcmp(m_piData1[m_iLastInsertPos], seqno1) < 0))
            i = m_iLastInsertPos;
         else
            i = m_iHead;

         while ((-1 != m_piNext[i]) && (CSeqNo::seqcmp(m_piData1[m_piNext[i]], seqno1) < 0))
            i = m_piNext[i];

         if ((-1 == m_piData2[i]) || (CSeqNo::seqcmp(m_piData2[i], seqno1) < 0))
         {
            m_iLastInsertPos = loc;

            // no overlap, create new node
            m_piData1[loc] = seqno1;
            if (seqno2 != seqno1)
               m_piData2[loc] = seqno2;

            m_piNext[loc] = m_piNext[i];
            m_piNext[i] = loc;

            m_iLength += CSeqNo::seqlen(seqno1, seqno2);
         }
         else
         {
            m_iLastInsertPos = i;

            // overlap, coalesce with prior node, insert(3, 7) to [2, 5], ... becomes [2, 7]
            if (CSeqNo::seqcmp(m_piData2[i], seqno2) < 0)
            {
               m_iLength += CSeqNo::seqlen(m_piData2[i], seqno2) - 1;
               m_piData2[i] = seqno2;

               loc = i;
            }
            else
               return 0;
         }
      }
   }
   else
   {
      m_iLastInsertPos = m_iHead;

      // insert to head node
      if (seqno2 != seqno1)
      {
         if (-1 == m_piData2[loc])
         {
            m_iLength += CSeqNo::seqlen(seqno1, seqno2) - 1;
            m_piData2[loc] = seqno2;
         }
         else if (CSeqNo::seqcmp(seqno2, m_piData2[loc]) > 0)
         {
            m_iLength += CSeqNo::seqlen(m_piData2[loc], seqno2) - 1;
            m_piData2[loc] = seqno2;
         }
         else 
            return 0;
      }
      else
         return 0;
   }

   // coalesce with next node. E.g., [3, 7], ..., [6, 9] becomes [3, 9] 
   while ((-1 != m_piNext[loc]) && (-1 != m_piData2[loc]))
   {
      int i = m_piNext[loc];

      if (CSeqNo::seqcmp(m_piData1[i], CSeqNo::incseq(m_piData2[loc])) <= 0)
      {
         // coalesce if there is overlap
         if (-1 != m_piData2[i])
         {
            if (CSeqNo::seqcmp(m_piData2[i], m_piData2[loc]) > 0)
            {
               if (CSeqNo::seqcmp(m_piData2[loc], m_piData1[i]) >= 0)
                  m_iLength -= CSeqNo::seqlen(m_piData1[i], m_piData2[loc]);

               m_piData2[loc] = m_piData2[i];
            }
            else
               m_iLength -= CSeqNo::seqlen(m_piData1[i], m_piData2[i]);
         }
         else
         {
            if (m_piData1[i] == CSeqNo::incseq(m_piData2[loc]))
               m_piData2[loc] = m_piData1[i];
            else
               m_iLength --;
         }

         m_piData1[i] = -1;
         m_piData2[i] = -1;
         m_piNext[loc] = m_piNext[i];
      }
      else
         break;
   }

   return m_iLength - origlen;
}

void CSndLossList::remove(const int32_t& seqno)
{
   CGuard listguard(m_ListLock);

   if (0 == m_iLength)
      return;

   // Remove all from the head pointer to a node with a larger seq. no. or the list is empty
   int offset = CSeqNo::seqoff(m_piData1[m_iHead], seqno);
   int loc = (m_iHead + offset + m_iSize) % m_iSize;

   if (0 == offset)
   {
      // It is the head. Remove the head and point to the next node
      loc = (loc + 1) % m_iSize;

      if (-1 == m_piData2[m_iHead])
         loc = m_piNext[m_iHead];
      else
      {
         m_piData1[loc] = CSeqNo::incseq(seqno);
         if (CSeqNo::seqcmp(m_piData2[m_iHead], CSeqNo::incseq(seqno)) > 0)
            m_piData2[loc] = m_piData2[m_iHead];

         m_piData2[m_iHead] = -1;

         m_piNext[loc] = m_piNext[m_iHead];
      }

      m_piData1[m_iHead] = -1;

      if (m_iLastInsertPos == m_iHead)
         m_iLastInsertPos = -1;

      m_iHead = loc;

      m_iLength --;
   }
   else if (offset > 0)
   {
      int h = m_iHead;

      if (seqno == m_piData1[loc])
      {
         // target node is not empty, remove part/all of the seqno in the node.
         int temp = loc;
         loc = (loc + 1) % m_iSize;

         if (-1 == m_piData2[temp])
            m_iHead = m_piNext[temp];
         else
         {
            // remove part, e.g., [3, 7] becomes [], [4, 7] after remove(3)
            m_piData1[loc] = CSeqNo::incseq(seqno);
            if (CSeqNo::seqcmp(m_piData2[temp], m_piData1[loc]) > 0)
               m_piData2[loc] = m_piData2[temp];
            m_iHead = loc;
            m_piNext[loc] = m_piNext[temp];
            m_piNext[temp] = loc;
            m_piData2[temp] = -1;
         }
      }
      else
      {
         // targe node is empty, check prior node
         int i = m_iHead;
         while ((-1 != m_piNext[i]) && (CSeqNo::seqcmp(m_piData1[m_piNext[i]], seqno) < 0))
            i = m_piNext[i];

         loc = (loc + 1) % m_iSize;

         if (-1 == m_piData2[i])
            m_iHead = m_piNext[i];
         else if (CSeqNo::seqcmp(m_piData2[i], seqno) > 0)
         {
            // remove part/all seqno in the prior node
            m_piData1[loc] = CSeqNo::incseq(seqno);
            if (CSeqNo::seqcmp(m_piData2[i], m_piData1[loc]) > 0)
               m_piData2[loc] = m_piData2[i];

            m_piData2[i] = seqno;

            m_piNext[loc] = m_piNext[i];
            m_piNext[i] = loc;

            m_iHead = loc;
         }
         else
            m_iHead = m_piNext[i];
      }

      // Remove all nodes prior to the new head
      while (h != m_iHead)
      {
         if (m_piData2[h] != -1)
         {
            m_iLength -= CSeqNo::seqlen(m_piData1[h], m_piData2[h]);
            m_piData2[h] = -1;
         }
         else
            m_iLength --;

         m_piData1[h] = -1;

         if (m_iLastInsertPos == h)
            m_iLastInsertPos = -1;

         h = m_piNext[h];
      }
   }
}

int CSndLossList::getLossLength()
{
   CGuard listguard(m_ListLock);

   return m_iLength;
}

int32_t CSndLossList::getLostSeq()
{
   if (0 == m_iLength)
     return -1;

   CGuard listguard(m_ListLock);

   if (0 == m_iLength)
     return -1;

   if (m_iLastInsertPos == m_iHead)
      m_iLastInsertPos = -1;

   // return the first loss seq. no.
   int32_t seqno = m_piData1[m_iHead];

   // head moves to the next node
   if (-1 == m_piData2[m_iHead])
   {
      //[3, -1] becomes [], and head moves to next node in the list
      m_piData1[m_iHead] = -1;
      m_iHead = m_piNext[m_iHead];
   }
   else
   {
      // shift to next node, e.g., [3, 7] becomes [], [4, 7]
      int loc = (m_iHead + 1) % m_iSize;

      m_piData1[loc] = CSeqNo::incseq(seqno);
      if (CSeqNo::seqcmp(m_piData2[m_iHead], m_piData1[loc]) > 0)
         m_piData2[loc] = m_piData2[m_iHead];

      m_piData1[m_iHead] = -1;
      m_piData2[m_iHead] = -1;

      m_piNext[loc] = m_piNext[m_iHead];
      m_iHead = loc;
   }

   m_iLength --;

   return seqno;
}

////////////////////////////////////////////////////////////////////////////////

CRcvLossList::CRcvLossList(const int& size):
m_piData1(NULL),
m_piData2(NULL),
m_piNext(NULL),
m_piPrior(NULL),
m_iSize(size)
{
   m_piData1 = new int32_t [m_iSize];
   m_piData2 = new int32_t [m_iSize];
   m_piNext = new int [m_iSize];
   m_piPrior = new int [m_iSize];

   // -1 means there is no data in the node
   for (int i = 0; i < size; ++ i)
   {
      m_piData1[i] = -1;
      m_piData2[i] = -1;
   }

   m_iLength = 0;
   m_iHead = -1;
   m_iTail = -1;

   gettimeofday(&m_TimeStamp, 0);
}

CRcvLossList::~CRcvLossList()
{
   delete [] m_piData1;
   delete [] m_piData2;
   delete [] m_piNext;
   delete [] m_piPrior;
}

void CRcvLossList::insert(const int32_t& seqno1, const int32_t& seqno2)
{
   gettimeofday(&m_TimeStamp, 0);

   // Data to be inserted must be larger than all those in the list
   // guaranteed by the UDT receiver

   if (0 == m_iLength)
   {
      // insert data into an empty list
      m_iHead = 0;
      m_iTail = 0;
      m_piData1[m_iHead] = seqno1;
      if (seqno2 != seqno1)
         m_piData2[m_iHead] = seqno2;

      m_piNext[m_iHead] = -1;
      m_piPrior[m_iHead] = -1;
      m_iLength += CSeqNo::seqlen(seqno1, seqno2);

      return;
   }

   // otherwise searching for the position where the node should be
   int offset = CSeqNo::seqoff(m_piData1[m_iHead], seqno1);
   int loc = (m_iHead + offset) % m_iSize;

   if ((-1 != m_piData2[m_iTail]) && (CSeqNo::incseq(m_piData2[m_iTail]) == seqno1))
   {
      // coalesce with prior node, e.g., [2, 5], [6, 7] becomes [2, 7]
      loc = m_iTail;
      m_piData2[loc] = seqno2;
   }
   else
   {
      // create new node
      m_piData1[loc] = seqno1;

      if (seqno2 != seqno1)
         m_piData2[loc] = seqno2;

      m_piNext[m_iTail] = loc;
      m_piPrior[loc] = m_iTail;
      m_piNext[loc] = -1;
      m_iTail = loc;
   }

   m_iLength += CSeqNo::seqlen(seqno1, seqno2);
}

bool CRcvLossList::remove(const int32_t& seqno)
{
   gettimeofday(&m_TimeStamp, 0);

   if (0 == m_iLength)
      return false; 

   // locate the position of "seqno" in the list
   int offset = CSeqNo::seqoff(m_piData1[m_iHead], seqno);
   if (offset < 0)
      return false;

   int loc = (m_iHead + offset) % m_iSize;

   if (seqno == m_piData1[loc])
   {
      // This is a seq. no. that starts the loss sequence

      if (-1 == m_piData2[loc])
      {
         // there is only 1 loss in the sequence, delete it from the node
         if (m_iHead == loc)
         {
            m_iHead = m_piNext[m_iHead];
            if (-1 != m_iHead)
               m_piPrior[m_iHead] = -1;
         }
         else
         {
            m_piNext[m_piPrior[loc]] = m_piNext[loc];
            if (-1 != m_piNext[loc])
               m_piPrior[m_piNext[loc]] = m_piPrior[loc];
            else
               m_iTail = m_piPrior[loc];
         }

         m_piData1[loc] = -1;
      }
      else
      {
         // there are more than 1 loss in the sequence
         // move the node to the next and update the starter as the next loss inSeqNo(seqno)

         // find next node
         int i = (loc + 1) % m_iSize;

         // remove the "seqno" and change the starter as next seq. no.
         m_piData1[i] = CSeqNo::incseq(m_piData1[loc]);

         // process the sequence end
         if (CSeqNo::seqcmp(m_piData2[loc], CSeqNo::incseq(m_piData1[loc])) > 0)
            m_piData2[i] = m_piData2[loc];

         // remove the current node
         m_piData1[loc] = -1;
         m_piData2[loc] = -1;
 
         // update list pointer
         m_piNext[i] = m_piNext[loc];
         m_piPrior[i] = m_piPrior[loc];

         if (m_iHead == loc)
            m_iHead = i;
         else
            m_piNext[m_piPrior[i]] = i;

         if (m_iTail == loc)
            m_iTail = i;
         else
            m_piPrior[m_piNext[i]] = i;
      }

      m_iLength --;

      return true;
   }

   // There is no loss sequence in the current position
   // the "seqno" may be contained in a previous node

   // searching previous node
   int i = (loc - 1 + m_iSize) % m_iSize;
   while (-1 == m_piData1[i])
      i = (i - 1 + m_iSize) % m_iSize;

   // not contained in this node, return
   if ((-1 == m_piData2[i]) || (CSeqNo::seqcmp(seqno, m_piData2[i]) > 0))
       return false;

   if (seqno == m_piData2[i])
   {
      // it is the sequence end

      if (seqno == CSeqNo::incseq(m_piData1[i]))
         m_piData2[i] = -1;
      else
         m_piData2[i] = CSeqNo::decseq(seqno);
   }
   else
   {
      // split the sequence

      // construct the second sequence from CSeqNo::incseq(seqno) to the original sequence end
      // located at "loc + 1"
      loc = (loc + 1) % m_iSize;

      m_piData1[loc] = CSeqNo::incseq(seqno);
      if (CSeqNo::seqcmp(m_piData2[i], m_piData1[loc]) > 0)      
         m_piData2[loc] = m_piData2[i];

      // the first (original) sequence is between the original sequence start to CSeqNo::decseq(seqno)
      if (seqno == CSeqNo::incseq(m_piData1[i]))
         m_piData2[i] = -1;
      else
         m_piData2[i] = CSeqNo::decseq(seqno);

      // update the list pointer
      m_piNext[loc] = m_piNext[i];
      m_piNext[i] = loc;
      m_piPrior[loc] = i;

      if (m_iTail == i)
         m_iTail = loc;
      else
         m_piPrior[m_piNext[loc]] = loc;
   }

   m_iLength --;

   return true;
}

bool CRcvLossList::remove(const int32_t& seqno1, const int32_t& seqno2)
{
   if (seqno1 <= seqno2)
   {
      for (int32_t i = seqno1; i <= seqno2; ++ i)
         remove(i);
   }
   else
   {
      for (int32_t j = seqno1; j < CSeqNo::m_iMaxSeqNo; ++ j)
         remove(j);
      for (int32_t k = 0; k <= seqno2; ++ k)
         remove(k);
   }

   return true;
}

bool CRcvLossList::find(const int32_t& seqno1, const int32_t& seqno2) const
{
   if (0 == m_iLength)
      return false;

   int p = m_iHead;

   while (-1 != p)
   {
      if ((CSeqNo::seqcmp(m_piData1[p], seqno1) == 0) ||
          ((CSeqNo::seqcmp(m_piData1[p], seqno1) > 0) && (CSeqNo::seqcmp(m_piData1[p], seqno2) <= 0)) ||
          ((CSeqNo::seqcmp(m_piData1[p], seqno1) < 0) && (m_piData2[p] != -1) && CSeqNo::seqcmp(m_piData2[p], seqno1) >= 0))
          return true;

      p = m_piNext[p];
   }

   return false;
}

int CRcvLossList::getLossLength() const
{
   return m_iLength;
}

int CRcvLossList::getFirstLostSeq() const
{
   if (0 == m_iLength)
      return -1;

   return m_piData1[m_iHead];
}

void CRcvLossList::getLossArray(int32_t* array, int& len, const int& limit, const int& threshold)
{
   timeval currtime;
   gettimeofday(&currtime, 0);

   len = 0;

   // do not feedback NAK unless no retransmission is received within a certain interval
   if ((currtime.tv_sec - m_TimeStamp.tv_sec) * 1000000 + currtime.tv_usec - m_TimeStamp.tv_usec < threshold)
      return;

   int i = m_iHead;

   while ((len < limit - 1) && (-1 != i))
   {
      array[len] = m_piData1[i];
      if (-1 != m_piData2[i])
      {
         // there are more than 1 loss in the sequence
         array[len] |= 0x80000000;
         ++ len;
         array[len] = m_piData2[i];
      }

      ++ len;

      i = m_piNext[i];
   }

   gettimeofday(&m_TimeStamp, 0);
}

////////////////////////////////////////////////////////////////////////////////

CIrregularPktList::CIrregularPktList(const int& size):
m_piData(NULL),
m_piErrorSize(NULL),
m_piNext(NULL),
m_iSize(size)
{
   m_piData = new int32_t [m_iSize];
   m_piErrorSize = new int [m_iSize];
   m_piNext = new int [m_iSize];

   // -1 means there is no data in the node
   for (int i = 0; i < size; ++ i)
      m_piData[i] = -1;

   m_iLength = 0;
   m_iHead = -1;
   m_iInsertPos = -1;
}

CIrregularPktList::~CIrregularPktList()
{
   delete [] m_piData;
   delete [] m_piErrorSize;
   delete [] m_piNext;
}

int CIrregularPktList::currErrorSize(const int32_t& seqno) const
{
   if (0 == m_iLength)
      return 0;

   int size = 0;
   int i = m_iHead;

   // calculate the sum of the size error until the node with a seq. no. not less than "seqno"
   while ((-1 != i) && (CSeqNo::seqcmp(m_piData[i], seqno) < 0))
   {
      size += m_piErrorSize[i];
      i = m_piNext[i];
   }

   return size;
}

void CIrregularPktList::addIrregularPkt(const int32_t& seqno, const int& errsize)
{
   if (0 == m_iLength)
   {
      // insert into an empty list

      m_iHead = 0;
      m_piData[m_iHead] = seqno;
      m_piErrorSize[m_iHead] = errsize;
      m_piNext[m_iHead] = -1;
      ++ m_iLength;
      m_iInsertPos = m_iHead;

      return;
   }

   // positioning...
   int offset = CSeqNo::seqoff(m_piData[m_iHead], seqno);
   int loc = (m_iHead + offset + m_iSize) % m_iSize;

   if (offset < 0)
   {
      // insert at head

      m_piData[loc] = seqno;
      m_piErrorSize[loc] = errsize;
      m_piNext[loc] = m_iHead;
      m_iHead = loc;
      ++ m_iLength;
   }
   else if (offset > 0)
   {
      // return if it is already there
      if (seqno == m_piData[loc])
         return;

      // locate previous node
      int i;

      if ((-1 != m_iInsertPos) && (CSeqNo::seqcmp(m_piData[m_iInsertPos], seqno) < 0))
         i = m_iInsertPos;
      else
         i = m_iHead;

      while ((-1 != m_piNext[i]) && (CSeqNo::seqcmp(m_piData[m_piNext[i]], seqno) < 0))
         i = m_piNext[i];

      // insert the node
      m_piNext[loc] = m_piNext[i];
      m_piNext[i] = loc;

      m_piData[loc] = seqno;
      m_piErrorSize[loc] = errsize;
      ++ m_iLength;
   }

   m_iInsertPos = loc;
}

void CIrregularPktList::deleteIrregularPkt(const int32_t& seqno)
{
   // remove all node until the one with seq. no. larger than the parameter

   int i = m_iHead;
   while ((-1 != i) && (CSeqNo::seqcmp(m_piData[i], seqno) <= 0))
   {
      m_piData[i] = -1;
      m_iLength --;

      if (i == m_iInsertPos)
         m_iInsertPos = -1;

      i = m_piNext[i];
   }

   m_iHead = i;
}
