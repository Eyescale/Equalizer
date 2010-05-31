/*****************************************************************************
Copyright (c) 2001 - 2009, The Board of Trustees of the University of Illinois.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the
  above copyright notice, this list of conditions
  and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the University of Illinois
  nor the names of its contributors may be used to
  endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu, last updated 05/06/2009
*****************************************************************************/

#ifndef __UDT_CACHE_H__
#define __UDT_CACHE_H__

#include "udt.h"
#include "common.h"
#include <set>
#include <map>


class CUDT;

struct CInfoBlock
{
   uint32_t m_piIP[4];
   int m_iIPversion;
   uint64_t m_ullTimeStamp;
   int m_iRTT;
   int m_iBandwidth;
   int m_iLossRate;
   int m_iReorderDistance;
   double m_dInterval;
   double m_dCWnd;
};

struct CIPComp
{
   bool operator()(const CInfoBlock* ib1, const CInfoBlock* ib2) const;
};

struct CTSComp
{
   bool operator()(const CInfoBlock* ib1, const CInfoBlock* ib2) const;
};

class CCache
{
public:
   CCache();
   CCache(const unsigned int& size);
   ~CCache();

public:
   int lookup(const sockaddr* addr, const int& ver, CInfoBlock* hb);
   void update(const sockaddr* addr, const int& ver, CInfoBlock* hb);

private:
   void convert(const sockaddr* addr, const int& ver, uint32_t* ip);

private:
   unsigned int m_uiSize;
   std::set<CInfoBlock*, CIPComp> m_sIPIndex;
   std::set<CInfoBlock*, CTSComp> m_sTSIndex;

   pthread_mutex_t m_Lock;

private:
   CCache(const CCache&);
   CCache& operator=(const CCache&);
};

#endif
