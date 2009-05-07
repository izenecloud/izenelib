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
   Yunhong Gu [gu@lac.uic.edu], last updated 04/07/2006
*****************************************************************************/

#include <cmath>
#include "common.h"
#include "window.h"


CACKWindow::CACKWindow():
m_piACKSeqNo(NULL),
m_piACK(NULL),
m_pTimeStamp(NULL),
m_iSize(1024),
m_iHead(0),
m_iTail(0)
{
   m_piACKSeqNo = new int32_t[m_iSize];
   m_piACK = new int32_t[m_iSize];
   m_pTimeStamp = new timeval[m_iSize];

   m_piACKSeqNo[0] = -1;
}

CACKWindow::CACKWindow(const int& size):
m_piACKSeqNo(NULL),
m_piACK(NULL),
m_pTimeStamp(NULL),
m_iSize(size),
m_iHead(0),
m_iTail(0)
{
   m_piACKSeqNo = new int32_t[m_iSize];
   m_piACK = new int32_t[m_iSize];
   m_pTimeStamp = new timeval[m_iSize];

   m_piACKSeqNo[0] = -1;
}

CACKWindow::~CACKWindow()
{
   delete [] m_piACKSeqNo;
   delete [] m_piACK;
   delete [] m_pTimeStamp;
}

void CACKWindow::store(const int32_t& seq, const int32_t& ack)
{
   m_piACKSeqNo[m_iHead] = seq;
   m_piACK[m_iHead] = ack;
   gettimeofday(m_pTimeStamp + m_iHead, 0);

   m_iHead = (m_iHead + 1) % m_iSize;

   // overwrite the oldest ACK since it is not likely to be acknowledged
   if (m_iHead == m_iTail)
      m_iTail = (m_iTail + 1) % m_iSize;
}

int CACKWindow::acknowledge(const int32_t& seq, int32_t& ack)
{
   if (m_iHead >= m_iTail)
   {
      // Head has not exceeded the physical boundary of the window

      for (int i = m_iTail, n = m_iHead; i < n; ++ i)
         // looking for indentical ACK Seq. No.
         if (seq == m_piACKSeqNo[i])
         {
            // return the Data ACK it carried
            ack = m_piACK[i];

            // calculate RTT
            timeval currtime;
            gettimeofday(&currtime, 0);
            int rtt = (currtime.tv_sec - m_pTimeStamp[i].tv_sec) * 1000000 + currtime.tv_usec - m_pTimeStamp[i].tv_usec;
            if (i == m_iHead)
            {
               m_iTail = m_iHead = 0;
               m_piACKSeqNo[0] = -1;
            }
            else
               m_iTail = (i + 1) % m_iSize;

            return rtt;
         }

      // Bad input, the ACK node has been overwritten
      return -1;
   }

   // Head has exceeded the physical window boundary, so it is behind tail
   for (int j = m_iTail, n = m_iHead + m_iSize; j < n; ++ j)
      // looking for indentical ACK seq. no.
      if (seq == m_piACKSeqNo[j % m_iSize])
      {
         // return Data ACK
         j %= m_iSize;
         ack = m_piACK[j];

         // calculate RTT
         timeval currtime;
         gettimeofday((timeval *)&currtime, 0);
         int rtt = (currtime.tv_sec - m_pTimeStamp[j].tv_sec) * 1000000 + currtime.tv_usec - m_pTimeStamp[j].tv_usec;
         if (j == m_iHead)
         {
            m_iTail = m_iHead = 0;
            m_piACKSeqNo[0] = -1;
         }
         else
            m_iTail = (j + 1) % m_iSize;

         return rtt;
      }

   // bad input, the ACK node has been overwritten
   return -1;
}

////////////////////////////////////////////////////////////////////////////////

CPktTimeWindow::CPktTimeWindow():
m_iAWSize(16),
m_piPktWindow(NULL),
m_iRWSize(16),
m_piRTTWindow(NULL),
m_piPCTWindow(NULL),
m_piPDTWindow(NULL),
m_iPWSize(16),
m_piProbeWindow(NULL)
{
   m_piPktWindow = new int[m_iAWSize];
   m_piRTTWindow = new int[m_iRWSize];
   m_piPCTWindow = new int[m_iRWSize];
   m_piPDTWindow = new int[m_iRWSize];
   m_piProbeWindow = new int[m_iPWSize];

   m_iPktWindowPtr = 0;
   m_iRTTWindowPtr = 0;
   m_iProbeWindowPtr = 0;

   gettimeofday(&m_LastArrTime, 0);

   m_iLastSentTime = 0;
   m_iMinPktSndInt = 1000000;

   for (int i = 0; i < m_iAWSize; ++ i)
      m_piPktWindow[i] = 1;

   for (int j = 0; j < m_iRWSize; ++ j)
      m_piRTTWindow[j] = m_piPCTWindow[j] = m_piPDTWindow[j] = 0;

   for (int k = 0; k < m_iPWSize; ++ k)
      m_piProbeWindow[k] = 1000;
}

CPktTimeWindow::CPktTimeWindow(const int& s1, const int& s2, const int& s3):
m_iAWSize(s1),
m_piPktWindow(NULL),
m_iRWSize(s2),
m_piRTTWindow(NULL),
m_piPCTWindow(NULL),
m_piPDTWindow(NULL),
m_iPWSize(s3),
m_piProbeWindow(NULL)
{
   m_piPktWindow = new int[m_iAWSize];
   m_piRTTWindow = new int[m_iRWSize];
   m_piPCTWindow = new int[m_iRWSize];
   m_piPDTWindow = new int[m_iRWSize];
   m_piProbeWindow = new int[m_iPWSize];

   m_iPktWindowPtr = 0;
   m_iRTTWindowPtr = 0;
   m_iProbeWindowPtr = 0;

   gettimeofday(&m_LastArrTime, 0);

   m_iLastSentTime = 0;
   m_iMinPktSndInt = 1000000;

   for (int i = 0; i < m_iAWSize; ++ i)
      m_piPktWindow[i] = 1;

   for (int j = 0; j < m_iRWSize; ++ j)
      m_piRTTWindow[j] = m_piPCTWindow[j] = m_piPDTWindow[j] = 0;

   for (int k = 0; k < m_iPWSize; ++ k)
      m_piProbeWindow[k] = 1000;
}

CPktTimeWindow::~CPktTimeWindow()
{
   delete [] m_piPktWindow;
   delete [] m_piRTTWindow;
   delete [] m_piPCTWindow;
   delete [] m_piPDTWindow;
   delete [] m_piProbeWindow;
}

int CPktTimeWindow::getMinPktSndInt() const
{
   return m_iMinPktSndInt;
}

int CPktTimeWindow::getPktRcvSpeed() const
{
   // sorting
   int temp;
   for (int i = 0, n = (m_iAWSize >> 1) + 1; i < n; ++ i)
      for (int j = i, m = m_iAWSize; j < m; ++ j)
         if (m_piPktWindow[i] > m_piPktWindow[j])
         {
            temp = m_piPktWindow[i];
            m_piPktWindow[i] = m_piPktWindow[j];
            m_piPktWindow[j] = temp;
         }

   // read the median value
   int median = (m_piPktWindow[(m_iAWSize >> 1) - 1] + m_piPktWindow[m_iAWSize >> 1]) >> 1;
   int count = 0;
   int sum = 0;
   int upper = median << 3;
   int lower = median >> 3;

   // median filtering
   for (int k = 0, l = m_iAWSize; k < l; ++ k)
      if ((m_piPktWindow[k] < upper) && (m_piPktWindow[k] > lower))
      {
         ++ count;
         sum += m_piPktWindow[k];
      }

   // claculate speed, or return 0 if not enough valid value
   if (count > (m_iAWSize >> 1))
      return (int)ceil(1000000.0 / (sum / count));
   else
      return 0;
}

bool CPktTimeWindow::getDelayTrend() const
{
   double pct = 0.0;
   double pdt = 0.0;

   for (int i = 0, n = m_iRWSize; i < n; ++ i)
      if (i != m_iRTTWindowPtr)
      {
         pct += m_piPCTWindow[i];
         pdt += m_piPDTWindow[i];
      }

   // calculate PCT and PDT value
   pct /= m_iRWSize - 1;
   if (0 != pdt)
      pdt = (m_piRTTWindow[(m_iRTTWindowPtr - 1 + m_iRWSize) % m_iRWSize] - m_piRTTWindow[m_iRTTWindowPtr]) / pdt;

   // PCT/PDT judgement
   // reference: M. Jain, C. Dovrolis, Pathload: a measurement tool for end-to-end available bandwidth
   return ((pct > 0.66) && (pdt > 0.45)) || ((pct > 0.54) && (pdt > 0.55));
}

int CPktTimeWindow::getBandwidth() const
{
   // sorting
   int temp;
   for (int i = 0, n = (m_iPWSize >> 1) + 1; i < n; ++ i)
      for (int j = i, m = m_iPWSize; j < m; ++ j)
         if (m_piProbeWindow[i] > m_piProbeWindow[j])
         {
            temp = m_piProbeWindow[i];
            m_piProbeWindow[i] = m_piProbeWindow[j];
            m_piProbeWindow[j] = temp;
         }

   // read the median value
   int median = (m_piProbeWindow[(m_iPWSize >> 1) - 1] + m_piProbeWindow[m_iPWSize >> 1]) >> 1;
   int count = 1;
   int sum = median;
   int upper = median << 3;
   int lower = median >> 3;

   // median filtering
   for (int k = 0, l = m_iPWSize; k < l; ++ k)
      if ((m_piProbeWindow[k] < upper) && (m_piProbeWindow[k] > lower))
      {
         ++ count;
         sum += m_piProbeWindow[k];
      }

   return (int)ceil(1000000.0 / (double(sum) / double(count)));
}

void CPktTimeWindow::onPktSent(const int& currtime)
{
   int interval = currtime - m_iLastSentTime;

   if ((interval < m_iMinPktSndInt) && (interval > 0))
      m_iMinPktSndInt = interval;

   m_iLastSentTime = currtime;
}

void CPktTimeWindow::onPktArrival()
{
   gettimeofday(&m_CurrArrTime, 0);

   // record the packet interval between the current and the last one
   m_piPktWindow[m_iPktWindowPtr] = (m_CurrArrTime.tv_sec - m_LastArrTime.tv_sec) * 1000000 + m_CurrArrTime.tv_usec - m_LastArrTime.tv_usec;

   // the window is logically circular
   m_iPktWindowPtr = (m_iPktWindowPtr + 1) % m_iAWSize;

   // remember last packet arrival time
   m_LastArrTime = m_CurrArrTime;
}

void CPktTimeWindow::ack2Arrival(const int& rtt)
{
   // record RTT, comparison (1 or 0), and absolute difference
   m_piRTTWindow[m_iRTTWindowPtr] = rtt;
   m_piPCTWindow[m_iRTTWindowPtr] = (rtt > m_piRTTWindow[(m_iRTTWindowPtr - 1 + m_iRWSize) % m_iRWSize]) ? 1 : 0;
   m_piPDTWindow[m_iRTTWindowPtr] = abs(rtt - m_piRTTWindow[(m_iRTTWindowPtr - 1 + m_iRWSize) % m_iRWSize]);

   // the window is logically circular
   m_iRTTWindowPtr = (m_iRTTWindowPtr + 1) % m_iRWSize;
}

void CPktTimeWindow::probe1Arrival()
{
   gettimeofday(&m_ProbeTime, 0);
}

void CPktTimeWindow::probe2Arrival()
{
   gettimeofday(&m_CurrArrTime, 0);

   // record the probing packets interval
   m_piProbeWindow[m_iProbeWindowPtr] = (m_CurrArrTime.tv_sec - m_ProbeTime.tv_sec) * 1000000 + m_CurrArrTime.tv_usec - m_ProbeTime.tv_usec;
   // the window is logically circular
   m_iProbeWindowPtr = (m_iProbeWindowPtr + 1) % m_iPWSize;
}
