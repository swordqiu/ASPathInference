/*
 *  Copyright 2003-2007 Jian Qiu <swordqiu@gmail.com>
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
 * SimpleRoutingTable.h
 *
 * Jian Qiu, jqiu@ecs.umass.edu
 * 12/24/2004
 */

#ifndef _SIMPLE_ROUTING_TABLE
#define _SIMPLE_ROUTING_TABLE

namespace bgplib {

#pragma pack(1)

class CSimpleRoutingTableRecord
{
public:
	CSimpleRoutingTableRecord();
	CSimpleRoutingTableRecord(const CSimpleRoutingTableRecord& clone);
	~CSimpleRoutingTableRecord();
	char* toString();
public:
	CPrefix m_prefix;
	u_int8_t m_pathlen;
	u_int32_t m_counter;
	//float m_counter_ave;
	off_t m_pathptr;
};

class CASNPathPair
{
	public:
		CASNPathPair();
		CASNPathPair(const CASNPathPair& clone);
		~CASNPathPair();
		char* toString();
	public:
		u_int32_t m_asn;
		u_int32_t m_counter;
		u_int32_t m_counter_rev;
		u_int8_t m_pathlen;
		off_t m_pathptr;
};

class CPrefixASNPair
{
	public:
		CPrefixASNPair();
		CPrefixASNPair(const CPrefixASNPair& clone);
		~CPrefixASNPair();
		char* toString();
	public:
		CPrefix m_prefix;
		u_int32_t m_asn;
		u_int32_t m_counter;
};

#pragma pack()

class CSimpleRoutingTable {
public:
	CSimpleRoutingTable();
	CSimpleRoutingTable(char* path, u_int32_t asn, bool isReadOnly = false);
	~CSimpleRoutingTable();
	void Init(char* path, u_int32_t asn, bool isReadOnly = false);
	void Release();
	void GetPath(off_t offset, u_int8_t len, u_int32_t *m_pathstr);
	off_t AddPath(u_int8_t len, u_int32_t* pathstr);
	char* GetPathString(off_t offset, u_int8_t len);
	bool FindByPrefix(CPrefix *prefix, long* pindex);
	bool FindByASN(u_int32_t asn, long* pindex);
	int ComparePath(u_int8_t len1, u_int32_t* str1, u_int8_t len2, u_int32_t* str2);
	bool FindByPrefixPathPair(CPrefix *prefix, u_int8_t pathlen, u_int32_t* pathstr, long* pindex);
	bool FindByASNPathPair(u_int32_t asn, u_int8_t pathlen, u_int32_t* pathstr, long* pindex);
	bool PrintAllPath(CPrefix *prefix, u_int32_t asn);
	bool GetAllPathStr(CPrefix *prefix, u_int32_t asn, char* str);
	void AddRecord(CPrefix* prefix, u_int8_t pathlen, u_int32_t* pathstr, u_int8_t pathlen_rev, u_int32_t* pathstr_rev);
	void AddRecord2(CPrefix* prefix, u_int8_t pathlen, u_int32_t* pathstr);
private:
	CSVMEM m_prefixtable;
	CSVMEM m_asntable;
	CVMEM m_pathmem;
};

class CPrefixASNMap: public CSVMEM {
public:
	CPrefixASNMap();
	CPrefixASNMap(char* filename, bool isReadOnly = false);
	~CPrefixASNMap();
public:
	void Init(char* filename, bool isReadOnly = false);
	bool FindByPrefixASNPair(CPrefix* prefix, u_int32_t asn, long* pindex);
	void AddRecord(CPrefix* prefix, u_int32_t asn);
	bool FindByPrefix(CPrefix* prefix, long* pindex);
	bool PrintAllASNOfPrefix(CPrefix* prefix);
	bool GetAllASNOfPrefix(CPrefix* prefix, CDynaArray* array);
};

} /* end of namespace bgplib */

#endif
