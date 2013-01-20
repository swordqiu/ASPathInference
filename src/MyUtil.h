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




// MyUtil.h: interface for the MyUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYUTIL_H__C255F8D1_E924_442D_BBC4_EDE2C8DE7F80__INCLUDED_)
#define AFX_MYUTIL_H__C255F8D1_E924_442D_BBC4_EDE2C8DE7F80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class UINT128
{
public:
	UINT128();
	UINT128(UINT128& u128);
	virtual ~UINT128();
public:
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
};

class MyUtil  
{
public:
	MyUtil();
	virtual ~MyUtil();

public:
	static bool isPathStatus(char* str);
	static bool writelog(char* dir, char* filename,char* str);
	static unsigned int INET_A2N(char* addr);
	static char* INET_N2A(unsigned int addr);
	static UINT_64 INET6_A2N(char* addr);
	static char* INET6_N2A(UINT_64 addr);
	static unsigned long hexchar2long(char* hex);
	static char* ConverStd6(char* addr);
	static char* ConvNStd6(char* addr);
	static unsigned int GetOriginASN(char* aspath);
	static void ConvType(void* data, unsigned int size);
	static void Sort(void* data, size_t len,unsigned int count,int (*compfunc)(void* data1,void* data2),bool asender=false);
	static void ShowPath(u_int32_t* pathstr, u_int8_t pathlen);
};

int getstring(char* dest,char* src,int start, char sep = ' ');
int getnstring(char* dest,char* src,int start,int len);
char* gettick();
bool str2long(char* str, long &num);
void QTRACE(const char *fmt, ...);
int strchrcount(char* str, char ch);

void ASSERT(bool val, const char* format, ...);
bool safe_sprintf(char* dest, size_t size, char* format, ...);

u_int32_t asn_a2n(const char* asn_str);
const char* asn_n2a(u_int32_t asn);
bool is_valid_asn(u_int32_t asn);

#define IsOrigin(x) ((strchr(PATH_ORIGIN,x)==NULL)? false:true)
#define IsStatus(x) ((strchr(PATH_STATUS,x)==NULL)? false:true)

bool is_directory(const char* path);
bool is_regular_file(const char* path);

#endif // !defined(AFX_MYUTIL_H__C255F8D1_E924_442D_BBC4_EDE2C8DE7F80__INCLUDED_)
