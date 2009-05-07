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
This header file contains the definition of UDT/CCC base class.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu [gu@lac.uic.edu], last updated 01/07/2007
*****************************************************************************/


#include "core.h"
#include "ccc.h"


CCC::CCC():
m_dPktSndPeriod(1.0),
m_dCWndSize(16.0),
m_iACKPeriod(0),
m_iACKInterval(0),
m_iRTO(-1),
m_bUserDefinedRTO(false)
{
}

void CCC::setACKTimer(const int& msINT)
{
   m_iACKPeriod = msINT;
}

void CCC::setACKInterval(const int& pktINT)
{
   m_iACKInterval = pktINT;
}

void CCC::setRTO(const int& usRTO)
{
   m_iRTO = usRTO;
   m_bUserDefinedRTO = true;
}

void CCC::sendCustomMsg(CPacket& pkt) const
{
   CUDT* u = CUDT::getUDTHandle(m_UDT);
   if (NULL != u)
      *(u->m_pChannel) << pkt;
}

const CPerfMon* CCC::getPerfInfo()
{
   CUDT* u = CUDT::getUDTHandle(m_UDT);
   if (NULL != u)
      u->sample(&m_PerfInfo, false);

   return &m_PerfInfo;
}
