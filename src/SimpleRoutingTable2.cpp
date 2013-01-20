/*
 *  Copyright 2003-2012 Jian Qiu <swordqiu@gmail.com>
 */

/*  This file is part of ASPathInference, a tool inferring AS level paths
 *  from any source AS to any destination prefix.
 *
 *  ASPathInference is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ASPathInference is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ASPathInference; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */




/*
 * SimpleRoutingTable.cpp
 *
 * Jian Qiu, swordqiu@gmail.com
 *
 * 12/24/2004
 * Last modified: 9/20/2012
 */

#include "StdAfx.h"
#include "VMEMstd.h"
#include "VMEM.h"
#include "Address.h"
#include "MyUtil.h"
#include "SimpleRoutingTable2.h"

namespace bgplib {

CSimpleRoutingTableRecord::CSimpleRoutingTableRecord() {
	m_pathlen = 0;
	m_pathptr = -1;
	m_counter = 0;
	//m_counter_ave = 0.0;
}

CSimpleRoutingTableRecord::CSimpleRoutingTableRecord(const CSimpleRoutingTableRecord& clone) {
	m_pathlen = clone.m_pathlen;
	m_prefix  = clone.m_prefix;
	m_pathptr = clone.m_pathptr;
	m_counter = clone.m_counter;
	//m_counter_ave = clone.m_counter_ave;
}

CSimpleRoutingTableRecord::~CSimpleRoutingTableRecord() {
}

char* CSimpleRoutingTableRecord::toString() {
	static char temp[1024];
	sprintf(temp, "%18s %d", m_prefix.toString(), m_pathlen);
	return temp;
}

/***********************************************/

CPrefixASNPair::CPrefixASNPair() {
	m_asn = 0;
	m_counter = 0;
}

CPrefixASNPair::CPrefixASNPair(const CPrefixASNPair& clone) {
	m_prefix = clone.m_prefix;
	m_asn = clone.m_asn;
	m_counter = clone.m_counter;
}

CPrefixASNPair::~CPrefixASNPair() {
}

char* CPrefixASNPair::toString() {
    static char temp[1024];
    sprintf(temp, "%18s %d %d", m_prefix.toString(), m_asn, m_counter);
    return temp;
}

/***************************************************/

CASNPathPair::CASNPathPair() {
	m_asn = 0;
	m_pathptr = -1;
	m_pathlen = 0;
	m_counter = 0;
	m_counter_rev = 0;
}

CASNPathPair::CASNPathPair(const CASNPathPair& clone) {
	m_asn = clone.m_asn;
	m_pathptr = clone.m_pathptr;
	m_pathlen = clone.m_pathlen;
	m_counter = clone.m_counter;
	m_counter_rev = clone.m_counter_rev;
}

CASNPathPair::~CASNPathPair() {
}

char* CASNPathPair::toString() {
	static char temp[256];
	return temp;
}

/*****************************************************/

CSimpleRoutingTable::CSimpleRoutingTable() {
}

CSimpleRoutingTable::CSimpleRoutingTable(char* path, u_int32_t asn, bool isReadOnly) {
	Init(path, asn, isReadOnly);
}

CSimpleRoutingTable::~CSimpleRoutingTable() {
	Release();
}

void CSimpleRoutingTable::Init(char* path, u_int32_t asn, bool isReadOnly) {
	char temp[256];

	sprintf(temp, "%s/%s_prefix", path, asn_n2a(asn));
	//printf("%s", temp);
	m_prefixtable.InitSMEM(sizeof(CSimpleRoutingTableRecord), temp, isReadOnly);

	sprintf(temp, "%s/%s_asn", path, asn_n2a(asn));
	//printf("%s", temp);
	m_asntable.InitSMEM(sizeof(CASNPathPair), temp, isReadOnly);

	sprintf(temp, "%s/%s_path", path, asn_n2a(asn));
	//printf("%s", temp);
	m_pathmem.Init(temp, isReadOnly);
}

void CSimpleRoutingTable::Release() {
	m_prefixtable.Release();
	m_asntable.Release();
	m_pathmem.Release();
}

void CSimpleRoutingTable::GetPath(off_t offset, u_int8_t len, u_int32_t* pathstr) {
	m_pathmem.VSeek(offset, SEEK_SET);
	m_pathmem.VRead(pathstr, sizeof(u_int32_t)*len);
}

off_t CSimpleRoutingTable::AddPath(u_int8_t len, u_int32_t* pathstr) {
	off_t offset = m_pathmem.VSeek(0, SEEK_END);
	m_pathmem.VWrite(pathstr, sizeof(u_int32_t)*len);
	return offset;
}

bool CSimpleRoutingTable::FindByPrefix(CPrefix *prefix, long* pindex) {
	long start = 0;
	long end = m_prefixtable.length - 1;
	while(start <= end) {
		long j = (start + end)/2;
		CSimpleRoutingTableRecord* srtr = (CSimpleRoutingTableRecord*)m_prefixtable.Get(j);
		int result = srtr->m_prefix.CompareTo(prefix);
		if(result < 0) {
			start = j + 1;
		}else if(result > 0) {
			end = j - 1;
		}else {
			*pindex = j;
			return true;
		}
	}
	*pindex = start;
	return false;
}

bool CSimpleRoutingTable::FindByASN(u_int32_t asn, long* pindex) {
	long start = 0;
	long end = m_asntable.length - 1;
	while(start <= end) {
		long j = (start + end)/2;
		CASNPathPair* srtr = (CASNPathPair*)m_asntable.Get(j);
		int result = 0;
		if (srtr->m_asn > asn) {
			result = 1;
		}else if (srtr->m_asn < asn) {
			result = -1;
		}
		if(result < 0) {
			start = j + 1;
		}else if(result > 0) {
			end = j - 1;
		}else {
			*pindex = j;
			return true;
		}
	}
	*pindex = start;
	return false;
}

int CSimpleRoutingTable::ComparePath(u_int8_t len1, u_int32_t* str1, u_int8_t len2, u_int32_t* str2) {
	if(len1 != len2) {
		return len1 - len2;
	}else {
		for(u_int8_t i = 0; i < len1; i ++) {
			if(str1[i] != str2[i]) {
				if (str1[i] > str2[i]) {
					return 1;
				}else if(str1[i] < str2[i]) {
					return -1;
				}
				//return str1[i] - str2[i];
			}
		}
	}
	return 0;
}

bool CSimpleRoutingTable::FindByPrefixPathPair(CPrefix *prefix, u_int8_t pathlen, u_int32_t* pathstr, long* pindex) {
	long start = 0;
	long end = m_prefixtable.length - 1;
	while(start <= end) {
		long j = (start + end)/2;
		CSimpleRoutingTableRecord* srtr = (CSimpleRoutingTableRecord*)m_prefixtable.Get(j);
		int result = srtr->m_prefix.CompareTo(prefix);
		if(result < 0) {
			start = j + 1;
		}else if(result > 0) {
			end = j - 1;
		}else {
			u_int32_t* tmppathstr = new u_int32_t[srtr->m_pathlen];
			GetPath(srtr->m_pathptr, srtr->m_pathlen, tmppathstr);
			result = ComparePath(srtr->m_pathlen, tmppathstr, pathlen, pathstr);
			delete tmppathstr;
			if(result < 0) {
				start = j + 1;
			}else if(result > 0) {
				end = j - 1;
			}else {
				*pindex = j;
				return true;
			}
		}
	}
	*pindex = start;
	return false;
}

bool CSimpleRoutingTable::FindByASNPathPair(u_int32_t asn, u_int8_t pathlen, u_int32_t* pathstr, long* pindex) {
	long start = 0;
	long end = m_asntable.length - 1;
	while(start <= end) {
		long j = (start + end)/2;
		CASNPathPair* srtr = (CASNPathPair*)m_asntable.Get(j);
		int result = 0;
		if (srtr->m_asn > asn) {
			result = 1;
		}else if (srtr->m_asn < asn) {
			result = -1;
		}
		//srtr->m_asn - asn;
		if(result < 0) {
			start = j + 1;
		}else if(result > 0) {
			end = j - 1;
		}else {
			u_int32_t* tmppathstr = new u_int32_t[srtr->m_pathlen];
			GetPath(srtr->m_pathptr, srtr->m_pathlen, tmppathstr);
			result = ComparePath(srtr->m_pathlen, tmppathstr, pathlen, pathstr);
			delete tmppathstr;
			if(result < 0) {
				start = j + 1;
			}else if(result > 0) {
				end = j - 1;
			}else {
				*pindex = j;
				return true;
			}
		}
	}
	*pindex = start;
	return false;
}

char* CSimpleRoutingTable::GetPathString(off_t offset, u_int8_t len) {
	u_int32_t* pathstr = new u_int32_t[len];
	GetPath(offset, len, pathstr);
	static char temp[1024];
	temp[0]='\0';
	for(int i = 0; i < len; i ++) {
		sprintf(temp, "%s %d", temp, pathstr[i]);
	}
	delete pathstr;
	return temp;
}

bool CSimpleRoutingTable::PrintAllPath(CPrefix *prefix, u_int32_t asn) {
	char temp[4096];
	if(GetAllPathStr(prefix, asn, temp)) {
		printf("%s", temp);
		return true;
	}else {
		return false;
	}
}

bool CSimpleRoutingTable::GetAllPathStr(CPrefix *prefix, u_int32_t asn, char* str) {
	long index = -1;
	long length = -1;
	if(prefix != NULL) {
		length = m_prefixtable.length;
	}else if(asn != 0) {
		length = m_asntable.length;
	}
	//printf("prefix: %s %ld %ld %ld %d %d\n", prefix->toString(), length, m_prefixtable.length, m_asntable.length, sizeof(CSimpleRoutingTableRecord), sizeof(CASNPathPair));
	if((prefix != NULL && FindByPrefix(prefix, &index)) || (asn != 0 && FindByASN(asn, &index))) {
		str[0] = '\0';
		bool upstop = false;
		bool downstop = false;
		long index2 = index;
		while(!upstop || !downstop) {
			//printf("%ld %ld %ld %d %d\n", index2, index, length, upstop, downstop);
			bool result = true;
			u_int32_t counter = 0;
			u_int32_t counter2 = 0;
			u_int8_t pathlen = 0;
			off_t pathptr = 0;
			if(prefix != NULL) {
				CSimpleRoutingTableRecord* srtr = (CSimpleRoutingTableRecord*) m_prefixtable.Get(index2);
				result = (srtr->m_prefix.CompareTo(prefix) == 0);
				counter = srtr->m_counter;
				counter2 = 0; //(int)srtr->m_counter_ave;
				pathlen = srtr->m_pathlen;
				pathptr = srtr->m_pathptr;
			}else if(asn != 0) {
				CASNPathPair* app;
				app = (CASNPathPair*) m_asntable.Get(index2);
				result = (app->m_asn == asn);
				counter = app->m_counter;
				counter2 = app->m_counter_rev;
				pathlen = app->m_pathlen;
				pathptr = app->m_pathptr;
			}
			if(result) {
				if(asn != 0) {
					sprintf(str, "%s%d %d%s\n", str, counter, counter2, GetPathString(pathptr, pathlen));
				}else {
					sprintf(str, "%s%d%s\n", str, counter, GetPathString(pathptr, pathlen));
				}
			}else {
				if(!upstop) {
					upstop = true;
					index2 = index;
				}else if(!downstop) {
					downstop = true;
				}
			}
			if(!upstop) {
				index2 --;
				if(index2 < 0) {
					upstop = true;
					index2 = index;
				}
			}
			if(upstop && !downstop) {
				index2 ++;
				if(index2 >= length) {
					downstop = true;
				}
			}
		}
		//printf("XXXX return: %s", str);
		return true;
	}else {
		return false;
	}
}

void CSimpleRoutingTable::AddRecord(CPrefix* prefix, u_int8_t pathlen, u_int32_t* pathstr, u_int8_t pathlen_rev, u_int32_t* pathstr_rev) {
	long index = -1;
	off_t pathptr = -1;
	if(FindByPrefixPathPair(prefix, pathlen, pathstr, &index)) {
		CSimpleRoutingTableRecord srtr = *((CSimpleRoutingTableRecord*)m_prefixtable.Get(index));
		srtr.m_counter ++;
		m_prefixtable.Set(&srtr, index);
	}else {
		CSimpleRoutingTableRecord srtr;
		srtr.m_prefix = *prefix;
		srtr.m_counter = 1;
		//srtr.m_counter_ave = 0.0;
		srtr.m_pathlen = pathlen;
		pathptr = AddPath(pathlen, pathstr);
		srtr.m_pathptr = pathptr;
		m_prefixtable.Insert(&srtr, index);
	}
	for(int len = pathlen; len >= 2; len --) {
		//MyUtil::ShowPath(pathstr, len);
		if(FindByASNPathPair(pathstr[len-1], len, pathstr, &index)) {
			CASNPathPair app = *((CASNPathPair*) m_asntable.Get(index));
			app.m_counter ++;
			m_asntable.Set(&app, index);
		}else {
			CASNPathPair app;
			app.m_asn = pathstr[len - 1];
			app.m_counter = 1;
			app.m_counter_rev = 0;
			app.m_pathlen = len;
			pathptr = AddPath(len, pathstr);
			app.m_pathptr = pathptr;
			m_asntable.Insert(&app, index);
		}
	}
	for(int len = pathlen_rev; len >= 2; len --) {
		//MyUtil::ShowPath(pathstr_rev, len);
		if(FindByASNPathPair(pathstr_rev[len-1], len, pathstr_rev, &index)) {
			CASNPathPair app = *((CASNPathPair*) m_asntable.Get(index));
			app.m_counter_rev ++;
			m_asntable.Set(&app, index);
		}else {
			CASNPathPair app;
			app.m_asn = pathstr_rev[len - 1];
			app.m_counter = 0;
			app.m_counter_rev = 1;
			app.m_pathlen = len;
			pathptr = AddPath(len, pathstr_rev);
			app.m_pathptr = pathptr;
			m_asntable.Insert(&app, index);
		}
	}
}

void CSimpleRoutingTable::AddRecord2(CPrefix* prefix, u_int8_t pathlen, u_int32_t* pathstr) {
	long index = -1;
	off_t pathptr = -1;
	if(FindByPrefixPathPair(prefix, pathlen, pathstr, &index)) {
		CSimpleRoutingTableRecord srtr = *((CSimpleRoutingTableRecord*)m_prefixtable.Get(index));
		srtr.m_counter ++;
		m_prefixtable.Set(&srtr, index);
	}else {
		CSimpleRoutingTableRecord srtr;
		srtr.m_prefix = *prefix;
		srtr.m_counter = 1;
		//srtr.m_counter_ave = 0.0;
		srtr.m_pathlen = pathlen;
		pathptr = AddPath(pathlen, pathstr);
		srtr.m_pathptr = pathptr;
		m_prefixtable.Insert(&srtr, index);
	}
}

/***********************************************************/

CPrefixASNMap::CPrefixASNMap():CSVMEM() {
}

CPrefixASNMap::CPrefixASNMap(char* filename, bool isReadOnly) {
	Init(filename, isReadOnly);
}

void CPrefixASNMap::Init(char* filename, bool isReadOnly) {
    char temp[256];
    if(is_directory(filename)) {
        sprintf(temp, "%s/prefix_asn_map", filename);
    }else {
        sprintf(temp, "%s", filename);
    }
	InitSMEM(sizeof(CPrefixASNPair), temp, isReadOnly);
}

CPrefixASNMap::~CPrefixASNMap() {
}

bool CPrefixASNMap::FindByPrefixASNPair(CPrefix* prefix, u_int32_t asn, long* pindex) {
	long start = 0;
	long end = length - 1;
	while(start <= end) {
		long j = (start + end)/2;
		CPrefixASNPair* pair = (CPrefixASNPair*) Get(j);
		if (pair->m_asn < asn) {
			start = j + 1;
		}else if(pair->m_asn > asn) {
			end = j - 1;
		}else {
		    int result = pair->m_prefix.CompareTo(prefix);
		    if(result < 0) {
				start = j + 1;
		    }else if(result > 0) {
				end = j - 1;
			}else {
				*pindex = j;
				return true;
			}
		}
	}
	*pindex = start;
	return false;
}

void CPrefixASNMap::AddRecord(CPrefix* prefix, u_int32_t asn) {
	static long index = -1L;
    // use cache first
    if (index >= 0 && index < length) {
        CPrefixASNPair pap = *((CPrefixASNPair*) Get(index));
        if (pap.m_asn == asn && pap.m_prefix.CompareTo(prefix) == 0) {
            pap.m_counter++;
            Set(&pap, index);
            return;
        }
    }
	if(FindByPrefixASNPair(prefix, asn, &index)) {
		CPrefixASNPair pap = *((CPrefixASNPair*) Get(index));
		pap.m_counter ++;
		Set(&pap, index);
        //printf("Find %s\n", pap.toString());
	}else {
		CPrefixASNPair pap;
		pap.m_prefix = *prefix;
		pap.m_asn = asn;
		pap.m_counter = 1;
		Insert(&pap, index);
        //printf("New %s\n", pap.toString());
	}
}

bool CPrefixASNMap::FindByASN(u_int32_t asn, long* pindex) {
	long start = 0;
	long end = length - 1;
	while(start <= end) {
		long j = (start + end)/2;
		CPrefixASNPair* pair = (CPrefixASNPair*) Get(j);
		if(pair->m_asn < asn) {
			start = j + 1;
		}else if(pair->m_asn > asn) {
			end = j - 1;
		}else {
			*pindex = j;
			return true;
		}
	}
	*pindex = start;
	return false;
}

bool CPrefixASNMap::PrintAllPrefixOfASN(u_int32_t asn) {
	CDynaArray array(sizeof(CPrefixASNPair));
	if(GetAllPrefixOfASN(asn, &array)) {
		for(int i = 0; i < array.GetLength(); i ++) {
			CPrefixASNPair* pap = (CPrefixASNPair*)array.Get(i);
			printf("%18s %d %d\n", pap->m_prefix.toString(),
                                pap->m_counter, pap->m_asn);
		}
		return true;
	}else{
		return false;
	}
}

bool CPrefixASNMap::GetAllPrefixOfASN(u_int32_t asn, CDynaArray* array) {
	long index = -1;
	if(FindByASN(asn, &index)) {
		bool upstop = false;
		bool downstop = false;
		long index2 = index;
		while(!upstop || !downstop) {
			//printf("%ld %ld %ld %d %d\n", index2, index, length, upstop, downstop);
			CPrefixASNPair* pap = (CPrefixASNPair*) Get(index2);
			if(pap->m_asn == asn) {
				//printf("%d %d\n", pap->m_counter, pap->m_asn);
				array->Add(pap);
			}else {
				if(!upstop) {
					upstop = true;
					index2 = index;
				}else if(!downstop) {
					downstop = true;
				}
			}
			if(!upstop) {
				index2 --;
				if(index2 < 0) {
					upstop = true;
					index2 = index;
				}
			}
			if(upstop && !downstop) {
				index2 ++;
				if(index2 >= length) {
					downstop = true;
				}
			}
		}
		return true;
	}else {
		return false;
	}
}

}
