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




#ifndef _BGP_UPDATE_H
#define _BGP_UPDATE_H

namespace bgplib {

#define BGP_UPDATE_MESSAGE_HEADER_LENGTH 19

#pragma pack(1)

class CBGPUpdateLog: public BufFile {
public:
	CBGPUpdateLog();
	CBGPUpdateLog(char* logfile);
	CBGPUpdateLog(CBGPUpdateLog& bul);
	~CBGPUpdateLog();

	void* GetValue(size_t size);

private:
	char* m_logfile;
public:
	unsigned long m_updatenum;
	unsigned long m_opennum;
	unsigned long m_notifynum;
	unsigned long m_keepalive;
};

/*
 * total length: 4*3 = 12 bytes
 *
 * 31          15            0
 *         m_time
 *    m_type       m_subtype
 *         m_length
 */

class CBGPUpdateHeader{
public:
	CBGPUpdateHeader();
	CBGPUpdateHeader(CBGPUpdateLog* bul);
	CBGPUpdateHeader(CBGPUpdateHeader& clone);
public:
	time_t m_time;
	u_int16_t m_type;
	u_int16_t m_subtype;
	u_int32_t m_length;
public:
	char* toString();
};

/*
 *
 * total length: 4*2 = 8
 *
 *   31          15           0
 *     src_as        dest_as
 *     interface     address family
 */

class CBGPUpdateASHeader{
public:
	CBGPUpdateASHeader();
	CBGPUpdateASHeader(CBGPUpdateLog* bul);
	CBGPUpdateASHeader(CBGPUpdateASHeader& clone);
	~CBGPUpdateASHeader();
public:
	u_int16_t m_src_as;
	u_int16_t m_dest_as;
	u_int16_t m_interface;
	u_int16_t m_af;
public:
	char* toString();
};

class CBGPUpdateViewHeader{
public:
	CBGPUpdateViewHeader();
	CBGPUpdateViewHeader(CBGPUpdateLog* bul);
	CBGPUpdateViewHeader(CBGPUpdateViewHeader& clone);
public:
	u_int16_t   m_view;
	u_int16_t   m_seq;
	CIPAddress  m_prefix;
        u_int8_t    m_mask;
        u_int8_t    m_status;
	time_t      m_lastchange;
	CIPAddress  m_peer_ip;
	u_int16_t   m_peer_as;
	u_int16_t   m_attrlen;
public:
	char* toString();
};

class CBGPUpdateViewHeader6{
public:
	CBGPUpdateViewHeader6();
	CBGPUpdateViewHeader6(CBGPUpdateLog* bul);
	CBGPUpdateViewHeader6(CBGPUpdateViewHeader6& clone);
public:
	u_int16_t   m_view;
	u_int16_t   m_seq;
	CIPAddress6  m_prefix;
        u_int8_t    m_mask;
        u_int8_t    m_status;
	time_t      m_lastchange;
	CIPAddress6  m_peer_ip;
	u_int16_t   m_peer_as;
	u_int16_t   m_attrlen;
public:
	char* toString();
};

class CBGPUpdateStateBody{
public:
	CBGPUpdateStateBody();
	CBGPUpdateStateBody(CBGPUpdateLog* bul);
	CBGPUpdateStateBody(CBGPUpdateStateBody& clone);
public:
	CIPAddress m_src_ip;
	CIPAddress m_dest_ip;
	u_int16_t m_old_state;
	u_int16_t m_new_state;
};

class CBGPUpdateStateBody6{
public:
	CBGPUpdateStateBody6();
	CBGPUpdateStateBody6(CBGPUpdateLog* bul);
	CBGPUpdateStateBody6(CBGPUpdateStateBody6& clone);
public:
	CIPAddress6 m_src_ip;
	CIPAddress6 m_dest_ip;
	u_int16_t m_old_state;
	u_int16_t m_new_state;
};

/*
 */

class CBGPMessageHeader{
public:
	CBGPMessageHeader();
	CBGPMessageHeader(CBGPUpdateLog* bul);
	CBGPMessageHeader(CBGPMessageHeader& clone);
public:
	CIPAddress m_src_ip;
	CIPAddress m_dest_ip;
	u_int8_t m_marker[16];
	u_int16_t m_length;
	u_int8_t m_type;
public:
	char* toString();
};

class CBGPMessageHeader6{
public:
	CBGPMessageHeader6();
	CBGPMessageHeader6(CBGPUpdateLog* bul);
	CBGPMessageHeader6(CBGPMessageHeader6& clone);
public:
	CIPAddress6 m_src_ip;
	CIPAddress6 m_dest_ip;
	u_int8_t m_marker[16];
	u_int16_t m_length;
	u_int8_t m_type;
public:
	char* toString();
};

#define BGP_MESSAGE_OPEN 1
#define BGP_MESSAGE_UPDATE 2
#define BGP_MESSAGE_NOTIFICATION 3
#define BGP_MESSAGE_KEEPALIVE 4

class CBGPOpenMessageHeader{
public:
	CBGPOpenMessageHeader();
public:
	u_int8_t m_version;
	u_int16_t m_myas;
	u_int16_t m_holdtime;
	u_int32_t m_bgpid;
	u_int8_t m_opt_param_len;
public:
	char* toString();
};

class CBGPOpenMessage {
public:
	CBGPOpenMessage();
	~CBGPOpenMessage();
	CBGPOpenMessage(CBGPOpenMessage& clone);
	CBGPOpenMessage(CBGPUpdateLog* bul);
public:
	CBGPOpenMessageHeader m_header;
	unsigned char* mp_opt_param;
public:
	char* toString();
};

class CBGPPathAttribute {
public:
	CBGPPathAttribute();
	CBGPPathAttribute(CBGPPathAttribute &clone);
	CBGPPathAttribute(CBGPUpdateLog* dul);
public:
	u_int8_t m_attr_flag;
	u_int8_t m_attr_type;
	union{
		struct {
			u_int16_t m_length;
			u_int8_t m_hook;
		}m_ext_data;

		struct {
			u_int8_t m_length;
			u_int8_t m_hook;
		}m_data;
	}m_attr;
public:
	bool IsOptional();
	bool IsTransitive();
	bool IsPartial();
	bool IsExtend();
	u_int16_t GetLength();
	void* GetData();
	int  Compare(CBGPPathAttribute* attr);
};

class CBGPPathAttr_Origin: public CBGPPathAttribute {
public:
	char* toString();
	u_int8_t GetOrigin();
	char* toBriefString();
};

class CBGPPathAttr_NextHop: public CBGPPathAttribute {
public:
	char* toString();
	CIPAddress GetNextHop();
	char* toBriefString();
};

class CBGPPathAttr_MED: public CBGPPathAttribute {
public:
	char* toString();
	u_int32_t GetMed();
	char* toBriefString();
};

class CBGPPathAttr_LocalPref: public CBGPPathAttribute {
public:
	char* toString();
	u_int32_t GetLocalPref();
	char* toBriefString();
};

class CBGPPathAttr_Aggregator: public CBGPPathAttribute {
public:
	char* toString();
	CIPAddress GetAggregator();
	u_int16_t  GetAggreAS();
	char* toBriefString();
};

class CBGPASPath {
public:
	CBGPASPath();
public:
	u_int8_t m_type;
	u_int8_t m_length;
	u_int8_t m_hook;
public:
	char* toString();
	u_int16_t GetASN(int index);
	u_int16_t GetAbsoluteLength();
};

class CBGPPathAttr_ASPath: public CBGPPathAttribute {
public:
	unsigned int GetASPathNumber();
	CBGPASPath* GetASPath(unsigned int index);
	u_int16_t GetOriginAS();
	char* toBriefString();
};

class CBGPPathAttr_Community: public CBGPPathAttribute {
public:
	char *toString();
	u_int32_t GetCommunity(int idx);
	u_int16_t GetCommunityNumber();
	char* toBriefString();
};

//REFERENCE: http://www.iana.org/assignments/bgp-parameters
#define BGP_PATH_ATTR_ORIGIN 1
#define BGP_PATH_ATTR_AS_PATH 2
#define BGP_PATH_ATTR_NEXT_HOP 3
#define BGP_PATH_ATTR_MED 4
#define BGP_PATH_ATTR_LOCAL_PREF 5
#define BGP_PATH_ATTR_ATOMIC_AGGREGATE 6
#define BGP_PATH_ATTR_AGGREGATOR 7
#define BGP_PATH_ATTR_COMMUNITY 8          //   [RFC1997]
#define BGP_PATH_ATTR_ORIGINATOR_ID 9      //   [RFC1998]
#define BGP_PATH_ATTR_CLUSTER_LIST 10      //   [RFC1998]
#define BGP_PATH_ATTR_DPA 11               //   [Chen]
#define BGP_PATH_ATTR_ADVERTISER 12        //   [RFC1863]
#define BGP_PATH_ATTR_CLUSTER_ID  13       //   [RFC1863]
#define BGP_PATH_ATTR_MP_REACH_NLRI 14	   //   [RFC2283]	
#define BGP_PATH_ATTR_MP_UNREACH_NLRI 15   //   [RFC2283]	
#define BGP_PATH_ATTR_EXT_COMMUNITIES  16  //   [Rosen]
#define BGP_PATH_ATTR_NEW_AS_PATH	17	   //   [E.Chen]	
#define BGP_PATH_ATTR_NEW_AGGREGATOR 18	   //   [E.Chen] 

#define BGP_PATH_ATTR_ORIGIN_IGP 0
#define BGP_PATH_ATTR_ORIGIN_EGP 1
#define BGP_PATH_ATTR_ORIGIN_INCOMPLETE 2

#define BGP_PATH_ATTR_AS_PATH_SET 1
#define BGP_PATH_ATTR_AS_PATH_SEQUENCE 2
//#define BGP_PATH_ATTR_AS_PATH_SET 1
//#define BGP_PATH_ATTR_AS_PATH_SET 1


class CBGPPathAttributeCollection {
public:
	CBGPPathAttributeCollection();
	~CBGPPathAttributeCollection();
	CBGPPathAttributeCollection(CBGPPathAttributeCollection& clone);
	CBGPPathAttributeCollection(CBGPUpdateLog* dul, int len);
private:
	bool m_isSet2;
public:
	u_int16_t m_length;
	unsigned char* mp_attrdata;
public:
	unsigned int GetAttrNumber();
	CBGPPathAttribute* GetAttribute(unsigned int index);
	CBGPPathAttribute* GetAttributeByType(u_int8_t type);
	void Set(CBGPUpdateLog* dul, int max_len);
	void Set(u_int16_t len, void* data);
	void SetBuffer(void* buffer);
	void Set2(u_int16_t len, void* data);
	void Set2(const CBGPPathAttributeCollection* attrs);
	bool Equal(CBGPPathAttributeCollection* attrs);
	bool isValid();
};

class CBGPNLRI {
public:
	CBGPNLRI();
	CBGPNLRI(CBGPNLRI& clone);
	CBGPNLRI(CBGPUpdateLog* dul);
public:
	u_int8_t m_length;
	u_int8_t m_hook;
public:
	char* toString(int af);
	CIPAddress GetAddress();
	CIPAddress6 GetAddress6();
	u_int8_t GetPrefixLen();
};

class CBGPNLRICollection {
public:
	CBGPNLRICollection();
	~CBGPNLRICollection();
	CBGPNLRICollection(CBGPNLRICollection& clone);
	CBGPNLRICollection(CBGPUpdateLog* dul, int max_len);
public:
	u_int16_t m_length;
	unsigned char* mp_nlridata;
public:
	unsigned int GetNLRINumber();
	CBGPNLRI* GetNLRI(unsigned int index);
	void Set(CBGPUpdateLog* dul, int max_len);
	void SetLength(unsigned int length);
	void SetDataFromLog(CBGPUpdateLog* dul);
	void SetData(void* data);
};

class CBGPUpdateMessage{
public:
	CBGPUpdateMessage();
	CBGPUpdateMessage(CBGPUpdateMessage& clone);
	CBGPUpdateMessage(CBGPUpdateLog* dul,unsigned int length);
public:
	CBGPNLRICollection m_withdraw;
	CBGPPathAttributeCollection m_pathattr;
	CBGPNLRICollection m_valid;
};

class CBGPNotifyMessage {
public:
	CBGPNotifyMessage();
	~CBGPNotifyMessage();
	CBGPNotifyMessage(CBGPNotifyMessage& clone);
	CBGPNotifyMessage(CBGPUpdateLog* dul,unsigned int msglen);
public:
	u_int8_t m_error;
	u_int8_t m_sub_err;
	unsigned int m_datalength;
	unsigned char* mp_data;
public:
	char* toString();
};

class CBGPMPReachNLRI {
public:
	CBGPMPReachNLRI();
	~CBGPMPReachNLRI();
public:
	u_int16_t m_afi;
	u_int8_t m_safi;
	u_int8_t m_nh_len;
	u_int8_t m_nh_hook;
public:
	u_int8_t GetSNPANumber();
	void* GetSNPAData(int index);
	u_int16_t GetNLRILength(u_int16_t len);
	void* GetNLRIData();
	u_int16_t GetAFI();
};

class CBGPMPUnreachNLRI {
public:
	CBGPMPUnreachNLRI();
	~CBGPMPUnreachNLRI();
public:
	u_int16_t m_afi;
	u_int8_t m_safi;
	u_int8_t m_hook;
public:
	u_int16_t GetNLRILength(u_int16_t len);
	void* GetNLRIData();
	u_int16_t GetAFI();
};

#define BGP_ERROR_HEADER 1
#define BGP_ERROR_OPEN   2
#define BGP_ERROR_UPDATE 3
#define BGP_ERROR_HOLDTIME 4
#define BGP_ERROR_FSM            5
#define BGP_ERROR_CEASE          6

#define BGP_ERROR_HEADER_SYNC 1
#define BGP_ERROR_HEADER_LEN  2
#define BGP_ERROR_HEADER_TYPE 3

#define BGP_ERROR_OPEN_VERSION 1
#define BGP_ERROR_OPEN_PEER    2
#define BGP_ERROR_OPEN_BGPID   3
#define BGP_ERROR_OPEN_OPT     4
#define BGP_ERROR_OPEN_AUTH    5
#define BGP_ERROR_OPEN_HOLDTIME 6

#define BGP_ERROR_UPDATE_ATTRLIST 1
#define BGP_ERROR_UPDATE_UNKOWNATTR 2
#define BGP_ERROR_UPDATE_MISSATTR 3
#define BGP_ERROR_UPDATE_ATTRFLAG 4
#define BGP_ERROR_UPDATE_ATTRLEN  5
#define BGP_ERROR_UPDATE_ORIGIN   6
#define BGP_ERROR_UPDATE_LOOP     7
#define BGP_ERROR_UPDATE_NEXTHOP  8
#define BGP_ERROR_UPDATE_OPTATTR  9
#define BGP_ERROR_UPDATE_NET      10
#define BGP_ERROR_UPDATE_ASPATH   11

#pragma pack()

#define MRT_NULL         0
#define MRT_START        1
#define MRT_DIE          2
#define MRT_I_AM_DEAD    3
#define MRT_PEER_DOWN    4
#define MRT_BGP          5
#define MRT_RIP          6
#define MRT_IDRP         7
#define MRT_RIPNG        8
#define MRT_BGP4PLUS     9
#define MRT_BGP4PLUS_01  10
#define MRT_OSPF         11
#define MRT_TABLE_DUMP   12
#define MRT_BGP4MP       16
#define MRT_BGP4MP_ET    17
#define MRT_ISIS         32
#define MRT_ISIS_ET      33
#define MRT_OSPF_ET      64

#define AFI_IPV4         1
#define AFI_IPV6         2

#define BGP4MP_STATE_CHANGE 0
#define BGP4MP_MESSAGE      1
#define BGP4MP_ENTRY        2
#define BGP4MP_SNAPSHOT     3
#define BGP4MP_MESSAGE_32BIT_AS 4

#define UNKNOWN_LOG -1
#define BGP_UPDATE_MESSAGE_V4 1
#define BGP_UPDATE_MESSAGE_V6 2
#define BGP_VIEW_DUMP_V4 3
#define BGP_VIEW_DUMP_V6 4

int getBGPUpdateType(char* filename);

int readBGPUpdateLog(char* filename, int (*processUpdateMessage4)(time_t tm, CBGPUpdateMessage* msg, CIPAddress peer, void* param), int (*processUpdateMessage6)(time_t tm, CBGPUpdateMessage *msg, CIPAddress6 peer, void* param), int (*processViewMessage4)(time_t tm, CBGPUpdateViewHeader* head, CBGPPathAttributeCollection *attr, void* param), int (*processViewMessage6)(time_t tm, CBGPUpdateViewHeader6* head, CBGPPathAttributeCollection *attr, void* param), int (*processOtherMessage)(time_t tm, u_int16_t type, u_int16_t subtype, u_int32_t length, void* data, void* param), void* param);

int readBGPViewDumpV4(char* filename, int (*processViewMessage4)(time_t tm, CBGPUpdateViewHeader* head, CBGPPathAttributeCollection *attr, void* param), void* param);

int readBGPUpdateMessageV4(char* filename, int (*processUpdateMessage4)(time_t tm, CBGPUpdateMessage* msg, CIPAddress peer, void* param), void* param);

char* toTypeString(u_int16_t type, u_int16_t subtype);

}


#endif
