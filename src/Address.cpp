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





#include "StdAfx.h"
#include "Address.h"
#include "MyUtil.h"

CIPAddress::CIPAddress()
{
	m_address=0;
}

CIPAddress::CIPAddress(const CIPAddress& clone)
{
	m_address=clone.m_address;
}

bool CIPAddress::ParseStr(const char* address)
{
	u_int32_t a,b,c,d;
	char temp[10];
	char addrtmp[256];
	strcpy(addrtmp, address);

	char* q=addrtmp;
	char* p=strchr(q,'.');
	strncpy(temp,q,p-q);
	temp[p-q]='\0';
	sscanf(temp,"%d",&a);

	q=p+1;
	p=strchr(q,'.');
	strncpy(temp,q,p-q);
	temp[p-q]='\0';
	sscanf(temp,"%d",&b);

	q=p+1;
	p=strchr(q,'.');
	strncpy(temp,q,p-q);
	temp[p-q]='\0';
	sscanf(temp,"%d",&c);
	
	q=p+1;
	sscanf(q,"%d",&d);

	m_address=(a<<24)|(b<<16)|(c<<8)|d;
	
	return true;
}

bool CIPAddress::ParseBitStr(const char* bitstr) {
	unsigned int i = 0;
	m_address = 0;
	while(i < strlen(bitstr)) {
		if(bitstr[i] == '1') {
			m_address+=(1<<(31-i));
		}
		i++;
	}
	return true;
}

CIPAddress::CIPAddress(const char* address)
{
	ParseStr(address);
}

CIPAddress::CIPAddress(void* data)
{
	Set(data);
}

void CIPAddress::Set(void* data) {
	memcpy(&m_address,data,sizeof(u_int32_t));
}

char* CIPAddress::toString()
{
	static char temp[MAX_ADDRESS_LEN];
	sprintf(temp,"%d.%d.%d.%d",(m_address&0xff000000)>>24,(m_address&0x00ff0000)>>16,(m_address&0x0000ff00)>>8,m_address&0x000000ff);
	return temp;
}

char* CIPAddress::toBitString()
{
	static char temp[33];
	u_int32_t mask = 0x80000000;
	for(int i = 0; i < 32; i ++)
	{
		if((m_address & (mask>>i)) != 0)
		{
			temp[i] = '1';
		}else
		{
			temp[i] = '0';
		}
	}
	temp[32] = '\0';
	return temp;
}

void CIPAddress::ConvType()
{
	MyUtil::ConvType(&m_address,sizeof(u_int32_t));
}

u_int32_t CIPAddress::GetData()
{
	return m_address;
}

u_int16_t CIPAddress::GetAddressType()
{
	if(m_address == 0)
	{
		return IPV4_ADDRESS_DEFAULT;
	}else if((m_address & 0xFF000000) == 0)
	{
		return IPV4_ADDRESS_INVALID;
	}else if((m_address & 0xFF000000) == 0x7F000000)
	{
		return IPV4_ADDRESS_LOOPBACK;
	}else if((m_address & 0x80000000) == 0)
	{
		return IPV4_ADDRESS_A;
	}else if((m_address & 0xC0000000) == 0x80000000)
	{
		return IPV4_ADDRESS_B;
	}else if((m_address & 0xE0000000) == 0xC0000000)
	{
		return IPV4_ADDRESS_C;
	}else if((m_address & 0xF0000000) == 0xE0000000)
	{
		return IPV4_ADDRESS_D;
	}else if((m_address & 0xF8000000) == 0xF0000000)
	{
		return IPV4_ADDRESS_E;
	}else
	{
		return IPV4_ADDRESS_INVALID;
	}
}

u_int8_t CIPAddress::GetA() {
	return (u_int8_t)((m_address & 0xFF000000) >> 24);
}
u_int8_t CIPAddress::GetB() {
	return (u_int8_t)((m_address & 0x00FF0000) >> 16);
}
u_int8_t CIPAddress::GetC() {
	return (u_int8_t)((m_address & 0x0000FF00) >> 8);
}
u_int8_t CIPAddress::GetD() {
	return (u_int8_t)((m_address & 0x000000FF));
}

CIPAddress CIPAddress::Mask(u_int16_t masklen)
{
	u_int32_t mask = 0;
	CIPAddress addr;

	for(int i = 0; i < masklen; i++)
	{
		mask = mask | (1<<(31-i));	
	}

	addr.m_address = m_address & mask;

	return addr;
}

CIPAddress6::CIPAddress6()
{
	m_address[0]=0;
	m_address[1]=0;
	m_address[2]=0;
	m_address[3]=0;
}

CIPAddress6::CIPAddress6(const CIPAddress6& clone)
{
	memcpy(m_address,clone.m_address,sizeof(u_int32_t)*4);
}

bool CIPAddress6::ParseStr(const char* address)
{
	char* q=MyUtil::ConverStd6((char*)address);
	char temp[10];
	u_int32_t tmpadd;
 	int idx=0;
	char* p;
	
	while((p=strchr(q,':'))!=NULL)
	{
		strncpy(temp,q,p-q);
		temp[p-q]='\0';
		sscanf(temp,"%x",&tmpadd);
		if(idx%2==0)
			m_address[idx/2]=tmpadd<<16;
		else
			m_address[idx/2]|=tmpadd;
		idx++;
		q=p+1;
	}
	sscanf(q,"%x",&tmpadd);
	m_address[3]|=tmpadd;
	return true;
}

CIPAddress6::CIPAddress6(const char* address)
{
	ParseStr(address);
}

CIPAddress6::CIPAddress6(void* data)
{
	memcpy(m_address,data,sizeof(u_int32_t)*4);
}

char* CIPAddress6::toString()
{
	static char temp[MAX_ADDRESS_LEN];

	sprintf(temp,"%x:%x:%x:%x:%x:%x:%x:%x",(m_address[0]&0xffff0000)>>16,m_address[0]&0x0000ffff,(m_address[1]&0xffff0000)>>16,m_address[1]&0x0000ffff,(m_address[2]&0xffff0000)>>16,m_address[2]&0x0000ffff,(m_address[3]&0xffff0000)>>16,m_address[3]&0x0000ffff);

	return MyUtil::ConvNStd6(temp);
}

char* CIPAddress6::toBitString()
{
	static char temp[129];
	u_int32_t mask = 0x80000000;

	for(int i = 0; i < 4; i ++) 
	{
		for(int j = 0; j < 32; j ++) 
		{
			if((m_address[i] & (mask >> j)) != 0)
			{
				temp[i*32 + j] = '1';
			}else
			{
				temp[i*32 + j] = '0';
			}
		}
	}
	temp[129] = '\0';

	return temp;
}

void CIPAddress6::ConvType()
{
	MyUtil::ConvType(&m_address[0],sizeof(u_int32_t));
	MyUtil::ConvType(&m_address[1],sizeof(u_int32_t));
	MyUtil::ConvType(&m_address[2],sizeof(u_int32_t));
	MyUtil::ConvType(&m_address[3],sizeof(u_int32_t));
}

u_int32_t CIPAddress6::GetData(int index)
{
	return m_address[index];
}

u_int16_t CIPAddress6::GetAddressType()
{
	if(m_address[0] == 0 && m_address[1] == 0 && m_address[2] == 0 && m_address[3] == 0)
	{
		return IPV6_ADDRESS_UNSPECIFIC;
	}else if(m_address[0] == 0 && m_address[1] == 0 && m_address[2] == 0 && m_address[3] == 1)
	{
		return IPV6_ADDRESS_LOOPBACK;
	}else if(m_address[0] == 0 && m_address[1] == 0 && m_address[2] == 0 && m_address[4] != 0)
	{
		return IPV6_ADDRESS_IPV4_COMPATIBLE;
	}else if(m_address[0] == 0 && m_address[1] == 0 && m_address[2] == 0xFF && m_address[4] != 0)
	{
		return IPV6_ADDRESS_IPV4_MAPPED;
	}else if((m_address[0] & 0xFF000000) == 0xFF000000)
	{
		return IPV6_ADDRESS_MULTICAST;
	}else if((m_address[0] & 0xFFC00000) == 0xFE800000)
	{
		return IPV6_ADDRESS_LINK_LOCAL;
	}else if((m_address[0] & 0xFFC00000) == 0xFEC00000)
	{
		return IPV6_ADDRESS_SITE_LOCAL;
	}else if((m_address[0] & 0xFFFF0000) == 0x3FFE0000)
	{
		return IPV6_ADDRESS_GLOBAL_TEST;
	}else if((m_address[0] & 0xFFFF0000) == 0x20010000)
	{
		return IPV6_ADDRESS_GLOBAL_VALID;
	}else if((m_address[0] & 0xE0000000) == 0x20000000)
	{
		return IPV6_ADDRESS_GLOBAL;
	}else
		return IPV6_ADDRESS_INVALID;
}

CPrefix::CPrefix()
{
	m_prefixlen = 32;
}

CPrefix::CPrefix(CIPAddress &addr, u_int16_t prelen)
{
	m_prefixlen = prelen;
	m_address   = addr;
}

void CPrefix::Set(const CIPAddress &addr, u_int16_t prelen)
{
	m_prefixlen = prelen;
	m_address   = addr;
}

void CPrefix::Set(void* data, u_int16_t preflen){
	CIPAddress addr(data);
	Set(addr, preflen);
}

CPrefix::CPrefix(const CPrefix& clone)
{
	m_address = clone.m_address;
	m_prefixlen = clone.m_prefixlen;
}

CPrefix::CPrefix(const char* str)
{
	ParseStr(str);
}

CPrefix::~CPrefix()
{
}

char* CPrefix::toString()
{
	static char temp[128];
	u_int8_t a = m_address.GetA();
	if((a < 128 && m_prefixlen == 8) || (a >= 128 && a < 192 && m_prefixlen == 16) || (a >= 192 && m_prefixlen == 24)) {
		sprintf(temp,"%s", m_address.toString());
	}else {
		sprintf(temp,"%s/%u", m_address.toString(), m_prefixlen);
	}
	return temp;
}

char* CPrefix::toBitString()
{
	static char bitstr[33];
	strncpy(bitstr, m_address.toBitString(), m_prefixlen);
	bitstr[m_prefixlen] = '\0';
	return bitstr;
}

bool CPrefix::ParseStr(const char* str)
{
	char* p;
	char temp[128];

	p = (char*)strchr(str, '/');
	if(p != NULL)
	{
		memcpy(temp, str, p-str);
		temp[p-str] = '\0';
		m_address.ParseStr(temp);
		m_prefixlen = atoi(p+1);
	}else
	{
		m_address.ParseStr(str);
		switch(m_address.GetAddressType())
		{
			case IPV4_ADDRESS_A: m_prefixlen = 8; break;
			case IPV4_ADDRESS_B: m_prefixlen = 16; break;
			case IPV4_ADDRESS_C: m_prefixlen = 24; break;
			default: return false;
		}
		if(m_address.GetD() != 0 && m_prefixlen <= 24) {
			m_prefixlen = 32;
		}else if(m_address.GetC() != 0 && m_prefixlen <= 16) {
			m_prefixlen = 24;
		}else if(m_address.GetB() != 0 && m_prefixlen <= 8) {
			m_prefixlen = 16;
		}else if(m_address.GetA() != 0 && m_prefixlen <= 0) {
			m_prefixlen = 8;
		}
		/*while(m_address.Mask(m_prefixlen) < m_address && m_prefixlen < 32) {
			m_prefixlen++;
		}*/
	}
	return true;
}

bool CPrefix::ParseBitStr(const char* bitstr) {
	m_address.ParseBitStr(bitstr);
	m_prefixlen = strlen(bitstr);
	return true;
}

void CPrefix::SetPrefixlen(u_int16_t preflen) {
	m_prefixlen = preflen;
	m_address = m_address.Mask(preflen);
}

u_int32_t CPrefix::GetHash() {
	u_int32_t data = m_address.GetData();
	return (data>>16) + (data & 0x0000FFFF) + (u_int32_t)m_prefixlen;
}

u_int16_t CPrefix::Compare(CPrefix* pref)
{
	int minlen;
	if(m_prefixlen < pref->m_prefixlen)
		minlen = m_prefixlen;
	else
		minlen = pref->m_prefixlen;
	if(m_address.Mask(minlen) == pref->m_address.Mask(minlen))
	{
		if(m_prefixlen < pref->m_prefixlen)
			return ADDRSPACE_CONTAIN_LARGER;
		else if(m_prefixlen == pref->m_prefixlen)
			return ADDRSPACE_EQUAL;
		else
			return ADDRSPACE_CONTAIN_SMALLER;
	}else
	{
		if(m_address > pref->m_address)
			return ADDRSPACE_LARGER;
		else
			return ADDRSPACE_SMALLER;
	}

}

int CPrefix::CompareTo(CPrefix* prefix) {
	if(m_address < prefix->m_address) {
		return -1;
	}else if(m_address > prefix->m_address) {
		return 1;
	}else {
		if(m_prefixlen < prefix->m_prefixlen) {
			return -1;
		}else if(m_prefixlen > prefix->m_prefixlen) {
			return 1;
		}else {
			return 0;
		}
	}
}

u_int32_t CPrefix::Distance(CPrefix* pref) {
	u_int32_t myval = this->m_address.GetData();
	u_int32_t val = pref->m_address.GetData();

	u_int32_t dist;
	if(myval > val) {
		dist = myval - val - pref->Content();
	}else {
		dist = val - myval - this->Content();
	}
	return dist;
}

bool CPrefix::isPublic() {
	if(m_address.GetA() == 0 || m_address.GetA() == 10 || m_address.GetA() == 127 || (m_address.GetA() == 192 && m_address.GetB() == 168) || (m_address.GetA() == 172 && m_address.Mask(12).GetB() == 16)) {
		return false;
	}else {
		return true;
	}
}

u_int32_t CPrefix::Content()
{
	u_int32_t cont=1;
	cont = cont <<(32-m_prefixlen);
	return cont;
}

CPrefix6::CPrefix6()
{
	m_prefixlen = 128;
}

CPrefix6::CPrefix6(CIPAddress6& addr, u_int16_t preflen)
{
	m_prefixlen = preflen;
	m_address   = addr;
}

CPrefix6::CPrefix6(const CPrefix6& clone)
{
	m_prefixlen = clone.m_prefixlen;
	m_address = clone.m_address;
}

CPrefix6::CPrefix6(const char* str)
{
	ParseStr(str);
}

CPrefix6::~CPrefix6()
{
}

char* CPrefix6::toString()
{
	static char temp[128];
	sprintf(temp, "%s/%u", m_address.toString(), m_prefixlen);
	return temp;
}

char* CPrefix6::toBitString()
{
	static char temp[129];
	strncpy(temp, m_address.toBitString(), m_prefixlen);
	temp[m_prefixlen] = '\0';
	return temp;
}

bool CPrefix6::ParseStr(const char* str)
{
	char* p;
	char temp[128];

	p = (char*)strchr(str, '/');
	if(p != NULL)
	{
		memcpy(temp, str, p-str);
		temp[p-str] = '\0';
		m_address.ParseStr(temp);
		m_prefixlen = atoi(p+1);
	}else
	{
		return false;
	}
	return true;
}

u_int16_t CPrefix6::Compare(CPrefix6* pref)
{
	//... under implementation
	return ADDRSPACE_EQUAL;
}
