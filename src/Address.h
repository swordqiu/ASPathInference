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




#ifndef ADDRESS_H
#define ADDRESS_H


#define IPV4_ADDRESS_INVALID	0 // 0.0.0.1/8
#define IPV4_ADDRESS_DEFAULT	1 // 0.0.0.0/32
#define IPV4_ADDRESS_A	2  // 0  1.0.0.0 - 126.255.255.255
#define IPV4_ADDRESS_B	3  // 10 128.0.0.0 - 191.255.255.255
#define IPV4_ADDRESS_C	4  // 110 192.0.0.0 - 223.255.255.255
#define IPV4_ADDRESS_D	5  // 1110 224.0.0.0 - 239.255.255.255
#define IPV4_ADDRESS_E	6  // 11110 240.0.0.0 - 255.255.255.255
#define IPV4_ADDRESS_LOOPBACK	6  // 0 or 127.xxx.xxx.xxx
#define IPV4_ADDRESS_MULTICAST	4  // class D

#define IPV6_ADDRESS_INVALID	0
#define IPV6_ADDRESS_UNSPECIFIC	1  // ::/128
#define IPV6_ADDRESS_LOOPBACK	2  // ::1/128
#define IPV6_ADDRESS_MULTICAST	3  // FF::/8 
#define IPV6_ADDRESS_LINK_LOCAL	4  // FE80::/10 
#define IPV6_ADDRESS_SITE_LOCAL	5  // FEC0::/10
#define IPV6_ADDRESS_GLOBAL	6  // 2000::/3
#define IPV6_ADDRESS_GLOBAL_TEST	7  // 3ffe::/16
#define IPV6_ADDRESS_GLOBAL_VALID	8  // 2001::/16
#define IPV6_ADDRESS_IPV4_COMPATIBLE	9  // ::xxx.xxx.xxx.xxx/128
#define IPV6_ADDRESS_IPV4_MAPPED	10  // ::FFFF:xxx.xxx.xxx.xxx/128


#define ADDRSPACE_CONTAIN_LARGER	1
#define ADDRSPACE_CONTAIN_SMALLER 	2
#define ADDRSPACE_LARGER		3
#define ADDRSPACE_SMALLER		4
#define ADDRSPACE_EQUAL			5

#pragma pack(1)

class CIPAddress{
public:
	CIPAddress();
	CIPAddress(const CIPAddress& clone);
	CIPAddress(const char* address);
	CIPAddress(void* data);
private:
	u_int32_t m_address;
public:
	char* toString();
	char* toBitString();
	void ConvType();
	u_int32_t GetData();
	void Set(void* data);
	bool ParseStr(const char* address);
	bool ParseBitStr(const char* bitstr);
	u_int16_t GetAddressType();
	CIPAddress Mask(u_int16_t masklen);
	u_int8_t GetA();
	u_int8_t GetB();
	u_int8_t GetC();
	u_int8_t GetD();
public:
	bool operator == (const CIPAddress& ip)
	{
		return m_address==ip.m_address;
	}
	bool operator > (const CIPAddress& ip)
	{
		return m_address > ip.m_address;
	}
	bool operator < (const CIPAddress& ip)
	{
		return m_address < ip.m_address;
	}
	bool operator != (const CIPAddress& ip)
	{
		return m_address != ip.m_address;
	}
};

class CIPAddress6 {
public:
	CIPAddress6();
	CIPAddress6(const CIPAddress6& clone);
	CIPAddress6(const char* address);
	CIPAddress6(void* data);
private:
	u_int32_t m_address[4];
public:
	char* toString();
	char* toBitString();
	void ConvType();
	u_int32_t GetData(int index);
	bool ParseStr(const char* address);
	u_int16_t GetAddressType();
public:
	bool operator == (const CIPAddress6& ip)
	{
		return m_address[0]==ip.m_address[0]&&m_address[1]==ip.m_address[1]&&m_address[2]==ip.m_address[2]&&m_address[3]==ip.m_address[3];
	}
};

class CPrefix {
public:
	CPrefix();
	CPrefix(const CPrefix& clone);
	CPrefix(const char* prefix);
	CPrefix(CIPAddress &addr, u_int16_t preflen);
	~CPrefix();
public:
	CIPAddress m_address;
	u_int16_t  m_prefixlen;
public:
	char* toString();
	char* toBitString();
	u_int32_t Distance(CPrefix* ppref);
	bool ParseStr(const char* str);
	bool ParseBitStr(const char* bitstr);
	void Set(const CIPAddress &addr, u_int16_t preflen);
	void Set(void* data, u_int16_t preflen);
	void SetPrefixlen(u_int16_t preflen);
	u_int16_t Compare(CPrefix* p);
	int CompareTo(CPrefix* p);
	u_int32_t Content();
	bool isPublic();
	u_int32_t  GetHash();
	bool operator == (const CPrefix& pref)
	{
		return (m_address==pref.m_address) && (m_prefixlen == pref.m_prefixlen);
	}
};

class CPrefix6 {
public:
	CPrefix6();
	CPrefix6(const CPrefix6& clone);
	CPrefix6(const char * prefix);
	CPrefix6(CIPAddress6 &addr, u_int16_t preflen);
	~CPrefix6();
public:
	CIPAddress6 m_address;
	u_int16_t m_prefixlen;
public:
	char* toString();
	char* toBitString();
	bool ParseStr(const char* str);
	u_int16_t Compare(CPrefix6* p);
};

#pragma pack()

#endif
