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
 *
 * Jian Qiu, jqiu@ecs.umass.edu
 *
 *
 * BGPUpdate.cpp
 */

#include "StdAfx.h"
#include "MyUtil.h"
#include "Address.h"
#include "BufFile.h"
#include "DateTime.h"
#include "BGPUpdate.h"

//#define _UPDATE_DEBUG_ 1
//

namespace bgplib {

CBGPUpdateLog::CBGPUpdateLog()
{
	m_updatenum=0;
	m_opennum=0;
	m_keepalive=0;
	m_notifynum=0;
}

CBGPUpdateLog::CBGPUpdateLog(char* logfile)
{
	Open(logfile,BUF_FILE_READ_BINARY_MODE);
	m_updatenum=0;
	m_opennum=0;
	m_keepalive=0;
	m_notifynum=0;
}

CBGPUpdateLog::CBGPUpdateLog(CBGPUpdateLog& bul)
{
	m_updatenum=bul.m_updatenum;
	m_opennum=bul.m_opennum;
	m_keepalive=bul.m_keepalive;
	m_notifynum=bul.m_notifynum;
}

CBGPUpdateLog::~CBGPUpdateLog()
{
	Close();
}

void* CBGPUpdateLog::GetValue(size_t size)
{
	return read(size);
}

CBGPUpdateHeader::CBGPUpdateHeader()
{
	m_length = 0;
	m_time = 0;
	m_type = 0;
	m_subtype = 0;
}

CBGPUpdateHeader::CBGPUpdateHeader(CBGPUpdateLog* bul)
{
	void* buf = bul->GetValue(sizeof(CBGPUpdateHeader));
	if(buf != NULL) {
		memcpy(this, buf, sizeof(CBGPUpdateHeader));
		MyUtil::ConvType(&m_length,sizeof(u_int32_t));
		MyUtil::ConvType(&m_time,sizeof(time_t));
		MyUtil::ConvType(&m_type,sizeof(u_int16_t));
		MyUtil::ConvType(&m_subtype,sizeof(u_int16_t));
	}else {
		m_length = 0;
		m_time = 0;
		m_type = 0;
		m_subtype = 0;
	}
}

CBGPUpdateHeader::CBGPUpdateHeader(CBGPUpdateHeader& clone)
{
	this->m_length=clone.m_length;
	this->m_subtype=clone.m_subtype;
	this->m_time=clone.m_time;
	this->m_type=clone.m_type;
}

char* CBGPUpdateHeader::toString()
{
	static char temp[1024];
	DateTime dt(m_time);
	sprintf(temp,"Time: %s TYPE:%d SUBTYPE:%d Length:%d", dt.tostr(), m_type, m_subtype, m_length);
	return temp;
}

CBGPUpdateASHeader::CBGPUpdateASHeader()
{
	m_src_as = 0;
	m_dest_as = 0;
	m_interface = 0;
	m_af = 0;
}

CBGPUpdateASHeader::CBGPUpdateASHeader(CBGPUpdateLog* bul)
{
	void* buf = bul->GetValue(sizeof(CBGPUpdateASHeader));
	if(buf != NULL) {
		memcpy(this, buf, sizeof(CBGPUpdateASHeader));
		MyUtil::ConvType(&m_src_as,sizeof(u_int16_t));
		MyUtil::ConvType(&m_dest_as,sizeof(u_int16_t));
		MyUtil::ConvType(&m_interface,sizeof(u_int16_t));
		MyUtil::ConvType(&m_af,sizeof(u_int16_t));
	}else {
		m_src_as = 0;
		m_dest_as = 0;
		m_interface = 0;
		m_af = 0;
	}
}

CBGPUpdateASHeader::CBGPUpdateASHeader(CBGPUpdateASHeader& clone)
{
	this->m_af=clone.m_af;
	this->m_dest_as=clone.m_dest_as;
	this->m_interface=clone.m_interface;
	this->m_src_as=clone.m_src_as;
}

char* CBGPUpdateASHeader::toString()
{
	static char temp[256];

	sprintf(temp,"SOURCE: AS%d DEST: AS%d IF: %d AF: %d",m_src_as,m_dest_as,m_interface,m_af);
	return temp;
}

CBGPUpdateASHeader::~CBGPUpdateASHeader()
{
}

CBGPUpdateViewHeader::CBGPUpdateViewHeader()
{
	m_view = 0;
	m_seq = 0;
	m_lastchange = 0;
	m_peer_as = 0;
	m_attrlen = 0;
}

CBGPUpdateViewHeader::CBGPUpdateViewHeader(CBGPUpdateLog* bul)
{
	void* buf = bul->GetValue(sizeof(CBGPUpdateViewHeader));
	if(buf != NULL) {
		memcpy(this, buf, sizeof(CBGPUpdateViewHeader));
		MyUtil::ConvType(&m_view, sizeof(u_int16_t));
		MyUtil::ConvType(&m_seq, sizeof(u_int16_t));
		m_prefix.ConvType();
		MyUtil::ConvType(&m_lastchange, sizeof(time_t));
		m_peer_ip.ConvType();
		MyUtil::ConvType(&m_peer_as, sizeof(u_int16_t));
		MyUtil::ConvType(&m_attrlen,sizeof(u_int16_t));
	}else {
		m_view = 0;
		m_seq = 0;
		m_lastchange = 0;
		m_peer_as = 0;
		m_attrlen = 0;
	}
}

CBGPUpdateViewHeader::CBGPUpdateViewHeader(CBGPUpdateViewHeader& clone)
{
	this->m_attrlen    = clone.m_attrlen;
	this->m_peer_as    = clone.m_peer_as;
	this->m_peer_ip    = clone.m_peer_ip;
	this->m_lastchange = clone.m_lastchange;
	this->m_status     = clone.m_status;
	this->m_mask       = clone.m_mask;
	this->m_prefix     = clone.m_prefix;
	this->m_view       = clone.m_view;
}

char *CBGPUpdateViewHeader::toString() {
	static char temp[128];
	sprintf(temp, "VIEW: #%d SEQ: %d Prefix: %s/%d Status: %d Len: %d\n", m_view, m_seq, m_prefix.toString(), m_mask, m_status, m_attrlen);
	sprintf(temp, "%sLastchange: %sPeer: %s AS: %d ", temp, ctime(&m_lastchange), m_peer_ip.toString(), m_peer_as);
	return temp;
}

CBGPUpdateViewHeader6::CBGPUpdateViewHeader6()
{
	m_peer_as = 0;
	m_attrlen = 0;
}

CBGPUpdateViewHeader6::CBGPUpdateViewHeader6(CBGPUpdateLog* bul)
{
	void* buf = bul->GetValue(sizeof(CBGPUpdateViewHeader6));
	if(buf != NULL) {
		memcpy(this, buf, sizeof(CBGPUpdateViewHeader6));
		MyUtil::ConvType(&m_view, sizeof(u_int16_t));
		MyUtil::ConvType(&m_seq, sizeof(u_int16_t));
		m_prefix.ConvType();
		MyUtil::ConvType(&m_lastchange, sizeof(time_t));
		m_peer_ip.ConvType();
		MyUtil::ConvType(&m_peer_as, sizeof(u_int16_t));
		MyUtil::ConvType(&m_attrlen,sizeof(u_int16_t));
	}else {
		m_peer_as = 0;
		m_attrlen = 0;
	}
}

CBGPUpdateViewHeader6::CBGPUpdateViewHeader6(CBGPUpdateViewHeader6& clone)
{
	this->m_attrlen    = clone.m_attrlen;
	this->m_peer_as    = clone.m_peer_as;
	this->m_peer_ip    = clone.m_peer_ip;
	this->m_lastchange = clone.m_lastchange;
	this->m_status     = clone.m_status;
	this->m_mask       = clone.m_mask;
	this->m_prefix     = clone.m_prefix;
	this->m_view       = clone.m_view;
}

char *CBGPUpdateViewHeader6::toString() {
	static char temp[128];
	sprintf(temp, "VIEW: #%d SEQ: %d Prefix: %s/%d Status: %d Len: %d\n", m_view, m_seq, m_prefix.toString(), m_mask, m_status, m_attrlen);
	sprintf(temp, "%sLastchange: %sPeer: %s AS: %d ", temp, ctime(&m_lastchange), m_peer_ip.toString(), m_peer_as);
	return temp;
}

CBGPUpdateStateBody::CBGPUpdateStateBody()
{
}

CBGPUpdateStateBody::CBGPUpdateStateBody(CBGPUpdateLog* bul)
{
	memcpy(this,bul->GetValue(sizeof(CBGPUpdateStateBody)),sizeof(CBGPUpdateStateBody));
	MyUtil::ConvType(&m_src_ip,sizeof(CIPAddress));
	MyUtil::ConvType(&m_dest_ip,sizeof(CIPAddress));
	MyUtil::ConvType(&m_old_state,sizeof(u_int16_t));
	MyUtil::ConvType(&m_new_state,sizeof(u_int16_t));
}

CBGPUpdateStateBody::CBGPUpdateStateBody(CBGPUpdateStateBody& clone)
{
	this->m_dest_ip=clone.m_dest_ip;
	this->m_new_state=clone.m_new_state;
	this->m_old_state=clone.m_old_state;
	this->m_src_ip=clone.m_src_ip;
}

CBGPUpdateStateBody6::CBGPUpdateStateBody6()
{
}

CBGPUpdateStateBody6::CBGPUpdateStateBody6(CBGPUpdateLog* bul)
{
	memcpy(this,bul->GetValue(sizeof(CBGPUpdateStateBody6)),sizeof(CBGPUpdateStateBody6));
	MyUtil::ConvType(&m_src_ip,sizeof(CIPAddress6));
	MyUtil::ConvType(&m_dest_ip,sizeof(CIPAddress6));
	MyUtil::ConvType(&m_old_state,sizeof(u_int16_t));
	MyUtil::ConvType(&m_new_state,sizeof(u_int16_t));
}

CBGPUpdateStateBody6::CBGPUpdateStateBody6(CBGPUpdateStateBody6& clone)
{
	m_src_ip=clone.m_src_ip;
	m_dest_ip=clone.m_dest_ip;
	m_old_state=clone.m_new_state;
	m_new_state=clone.m_new_state;
}

CBGPMessageHeader::CBGPMessageHeader()
{
	m_length = 0;
	m_type = 0;
}

CBGPMessageHeader::CBGPMessageHeader(CBGPUpdateLog* bul)
{
	void* buf = bul->GetValue(sizeof(CBGPMessageHeader));
	if(buf != NULL) {
		memcpy(this, buf, sizeof(CBGPMessageHeader));
		MyUtil::ConvType(&m_src_ip,sizeof(CIPAddress));
		MyUtil::ConvType(&m_dest_ip,sizeof(CIPAddress));
		//MyUtil::ConvType(m_marker,sizeof(u_int8_t)*16);
		MyUtil::ConvType(&m_length,sizeof(u_int16_t));
		MyUtil::ConvType(&m_type,sizeof(u_int8_t));
	}else {
		m_length = 0;
		m_type = 0;
	}
}

CBGPMessageHeader::CBGPMessageHeader(CBGPMessageHeader& clone)
{
	m_src_ip=clone.m_src_ip;
	m_dest_ip=clone.m_dest_ip;
	memcpy(m_marker,clone.m_marker,sizeof(u_int8_t)*16);
	m_length=clone.m_length;
	m_type=clone.m_type;
}

char* CBGPMessageHeader::toString()
{
	static char temp[256];

	sprintf(temp,"src: %s", m_src_ip.toString());
	sprintf(temp,"%s dest: %s length: %d type: %d",temp,m_dest_ip.toString(),m_length,m_type);

	return temp;
}


CBGPMessageHeader6::CBGPMessageHeader6()
{
}

CBGPMessageHeader6::CBGPMessageHeader6(CBGPUpdateLog* bul)
{
	//int x=sizeof(CBGPMessageHeader6);
	void* buf = bul->GetValue(sizeof(CBGPMessageHeader6));
	if(buf != NULL) {
		memcpy(this, buf, sizeof(CBGPMessageHeader6));
		//MyUtil::ConvType(&m_src_ip,sizeof(CIPAddress6));
		m_src_ip.ConvType();
		//MyUtil::ConvType(&m_dest_ip,sizeof(CIPAddress6));
		m_dest_ip.ConvType();
		//MyUtil::ConvType(&m_marker,sizeof(u_int8_t)*16);
		MyUtil::ConvType(&m_length,sizeof(u_int16_t));
		MyUtil::ConvType(&m_type,sizeof(u_int8_t));
	}else {
		m_length = 0;
		m_type = 0;
	}
}

CBGPMessageHeader6::CBGPMessageHeader6(CBGPMessageHeader6& clone)
{
	m_src_ip=clone.m_src_ip;
	m_dest_ip=clone.m_dest_ip;
	memcpy(m_marker,clone.m_marker,sizeof(u_int8_t)*16);
	m_length=clone.m_length;
	m_type=clone.m_type;
}

char* CBGPMessageHeader6::toString()
{
	static char temp[256];

	sprintf(temp,"src: %s", m_src_ip.toString());
	sprintf(temp,"%s dest: %s length: %d type: %d",temp,m_dest_ip.toString(),m_length,m_type);

	return temp;
}

CBGPOpenMessageHeader::CBGPOpenMessageHeader()
{
}

char* CBGPOpenMessageHeader::toString()
{
	static char temp[256];

	sprintf(temp,"VER: %d AS%d HOLDTIME: %d ID: %s OPT_LEN: %d",m_version,m_myas,m_holdtime,MyUtil::INET_N2A(m_bgpid),m_opt_param_len);
	return temp;
}

CBGPOpenMessage::CBGPOpenMessage()
{
	m_header.m_opt_param_len=0;
}

CBGPOpenMessage::~CBGPOpenMessage()
{
	if(m_header.m_opt_param_len>0)
		delete mp_opt_param;
}

CBGPOpenMessage::CBGPOpenMessage(CBGPOpenMessage& clone)
{
	m_header.m_version=clone.m_header.m_version;
	m_header.m_myas=clone.m_header.m_myas;
	m_header.m_holdtime=clone.m_header.m_holdtime;
	m_header.m_bgpid=clone.m_header.m_bgpid;
	m_header.m_opt_param_len=clone.m_header.m_opt_param_len;
	if(m_header.m_opt_param_len>0)
	{
		mp_opt_param=new unsigned char[m_header.m_opt_param_len];
		memcpy(mp_opt_param,clone.mp_opt_param,m_header.m_opt_param_len);
	}
}

CBGPOpenMessage::CBGPOpenMessage(CBGPUpdateLog* bul)
{
	memcpy(&m_header,bul->GetValue(sizeof(CBGPOpenMessageHeader)),sizeof(CBGPOpenMessageHeader));
	MyUtil::ConvType(&m_header.m_version, sizeof(u_int8_t));
	MyUtil::ConvType(&m_header.m_myas,sizeof(u_int16_t));
	MyUtil::ConvType(&m_header.m_holdtime, sizeof(u_int16_t));
	MyUtil::ConvType(&m_header.m_bgpid, sizeof(u_int32_t));
	MyUtil::ConvType(&m_header.m_opt_param_len, sizeof(u_int8_t));
	if(m_header.m_opt_param_len>0)
	{
		mp_opt_param=new unsigned char[m_header.m_opt_param_len];
		memcpy(mp_opt_param,bul->GetValue(m_header.m_opt_param_len),m_header.m_opt_param_len);
	}
}

char* CBGPOpenMessage::toString()
{
	return m_header.toString();
}

CBGPNLRICollection::CBGPNLRICollection()
{
	m_length=0;
}

CBGPNLRICollection::~CBGPNLRICollection()
{
	if(m_length>0)
	{
		delete mp_nlridata;
	}
}

CBGPNLRICollection::CBGPNLRICollection(CBGPNLRICollection& clone)
{
	m_length=clone.m_length;
	if(m_length>0)
	{
		mp_nlridata=new unsigned char[m_length];
		memcpy(mp_nlridata,clone.mp_nlridata,m_length);
	}
}

CBGPNLRICollection::CBGPNLRICollection(CBGPUpdateLog* dul, int max_len)
{
	Set(dul, max_len);
}

void CBGPNLRICollection::Set(CBGPUpdateLog* dul, int max_len)
{
	void* buf = dul->GetValue(sizeof(u_int16_t));
	if(buf != NULL) {
		memcpy(&m_length, buf, sizeof(u_int16_t));
		MyUtil::ConvType(&m_length, sizeof(u_int16_t));
#ifdef _UPDATE_DEBUG_
		printf("CBGPNLRICollection::Set: m_length: %d max_len: %d - 2\n", m_length, max_len);
#endif
		if(m_length + 2 > max_len) {
			m_length = 0;
		}
		SetDataFromLog(dul);
	}
}

void CBGPNLRICollection::SetLength(unsigned int length)
{
	m_length=length;
}

void CBGPNLRICollection::SetDataFromLog(CBGPUpdateLog* dul)
{
	if(m_length>0)
	{
		mp_nlridata=new unsigned char[m_length];
		//printf("[CBGPNLRICollection::SetData] m_length = %ld\n", m_length);fflush(stdout);
		if(mp_nlridata != NULL) {
			void* buf = dul->GetValue(m_length);
			if(buf != NULL) {
				memcpy(mp_nlridata, buf, m_length);
			}
		}else {
			printf("Error: fail to allocate memory for CBGPNLRICollection::SetData!\n");
			exit(-1);
		}
	}
}

void CBGPNLRICollection::SetData(void* data)
{
	if(m_length>0)
	{
		mp_nlridata=new unsigned char[m_length];
		memcpy(mp_nlridata, data, m_length);
	}
}

unsigned int CBGPNLRICollection::GetNLRINumber()
{
	unsigned int offset=0;
	unsigned int num=0;
	unsigned char length;

	while(offset<m_length)
	{
		length=*(mp_nlridata+offset);
		offset+=1+length/8;
		if(length%8!=0)
		offset+=1;
		if(offset > m_length) break;
		num++;
	}

	return num;
}

CBGPNLRI* CBGPNLRICollection::GetNLRI(unsigned int index)
{
	unsigned int offset=0;
	unsigned int num=0;
	unsigned char length;

	while(offset < m_length && num < index)
	{
		length=*(mp_nlridata+offset);
		offset+=1+length/8;
		if(length%8!=0)
			offset+=1;
		num++;
	}

	return (CBGPNLRI*)(mp_nlridata+offset);
}

CBGPPathAttributeCollection::CBGPPathAttributeCollection()
{
	m_length=0;
	m_isSet2 = false;
	mp_attrdata = NULL;
}

CBGPPathAttributeCollection::~CBGPPathAttributeCollection()
{
	if(m_length>0 && !m_isSet2)
	{
		delete mp_attrdata;
	}
}

CBGPPathAttributeCollection::CBGPPathAttributeCollection(CBGPPathAttributeCollection& clone)
{
	m_length=clone.m_length;
	if(m_length>0)
	{
		mp_attrdata=new unsigned char[m_length];
		memcpy(mp_attrdata,clone.mp_attrdata,m_length);
	}
}

CBGPPathAttributeCollection::CBGPPathAttributeCollection(CBGPUpdateLog* dul, int len)
{
	Set(dul, len);
}

void CBGPPathAttributeCollection::Set(CBGPUpdateLog* dul, int max_len)
{
	void* buf = dul->GetValue(sizeof(u_int16_t));
	if(buf != NULL) {
		memcpy(&m_length, buf, sizeof(u_int16_t));
		MyUtil::ConvType(&m_length,sizeof(u_int16_t));
#ifdef _UPDATE_DEBUG_
		printf("CBGPPathAttributeCollection::Set: m_length: %d max_len: %d - 2\n", m_length, max_len);
#endif
		if(m_length + 2 > max_len) {
			m_length = 0;
		}
		if(m_length>0)
		{
			mp_attrdata=new unsigned char[m_length];
			if(mp_attrdata != NULL) {
				buf = dul->GetValue(m_length);
				if(buf != NULL) {
					memcpy(mp_attrdata, buf, m_length);
				}
			}else {
				printf("Error: fail to allocate memory for CBGPPathAttributeCollection::Set(CBGPUpdateLog* dul, int len).\n");
				exit(-1);
			}
		}
	}
}

void CBGPPathAttributeCollection::Set(u_int16_t len, void *data) {
	m_length = len;
	if(len > 0) 
	{
		mp_attrdata = new unsigned char[m_length];
		memcpy(mp_attrdata, data, m_length);
	}
}

void CBGPPathAttributeCollection::SetBuffer(void* buf)
{
	m_isSet2 = true;
	mp_attrdata = (unsigned char*)buf;
}

void CBGPPathAttributeCollection::Set2(const CBGPPathAttributeCollection *attrs)
{
	ASSERT(m_isSet2, "Fatal error, call set2 after setbuffer");
	m_length = attrs->m_length;
	if(m_length > 0) 
	{
		memcpy(mp_attrdata, attrs->mp_attrdata, m_length);
	}
}

void CBGPPathAttributeCollection::Set2(u_int16_t len, void* data)
{
	ASSERT(m_isSet2, "Fatal error, call set2 after setbuffer");
	m_length = len;
	if(len > 0)
	{
		memcpy(mp_attrdata, data, m_length);
	}
}

unsigned int CBGPPathAttributeCollection::GetAttrNumber()
{
	unsigned int num=0;
	unsigned int offset=0;
	unsigned char flag;
	
	while(offset<m_length)
	{
		flag=*(mp_attrdata+offset);
		offset+=2;
		if((flag & 0x10)!=0)
		{
			u_int16_t len16=*((u_int16_t*)(mp_attrdata+offset));
			offset+=2+len16;
		}else
		{
			u_int8_t len8=*((u_int8_t*)(mp_attrdata+offset));
			offset+=1+len8;
		}
		num++;
	}
	return num;
}

CBGPPathAttribute* CBGPPathAttributeCollection::GetAttribute(unsigned int index)
{
        unsigned int num=0;
        unsigned int offset=0;
        unsigned char flag;

        while(offset<m_length && num<index)
        {
                flag=*(mp_attrdata+offset);
                offset+=2;
                if((flag & 0x10)!=0)
                {
                        u_int16_t len16=*((u_int16_t*)(mp_attrdata+offset));
                        offset+=2+len16;
                }else
                {
                        u_int8_t len8=*((u_int8_t*)(mp_attrdata+offset));
                        offset+=1+len8;
                }
                num++;
        }
        return (CBGPPathAttribute*)(mp_attrdata+offset);
}


CBGPPathAttribute* CBGPPathAttributeCollection::GetAttributeByType(u_int8_t type)
{
        unsigned int offset=0;
        unsigned char flag;
        CBGPPathAttribute* attr = NULL;

        while(offset<m_length)
        {
        	attr = (CBGPPathAttribute*)(mp_attrdata + offset);
		//printf("attr type: %d\n", attr->m_attr_type);
		if(attr->m_attr_type == type) {
			break;
		} else {
			attr = NULL;
		}
                flag=*(mp_attrdata+offset);
                offset+=2;
                if((flag & 0x10)!=0)
                {
                        u_int16_t len16=*((u_int16_t*)(mp_attrdata+offset));
                        offset += 2+len16;
                }else
                {
                        u_int8_t len8=*((u_int8_t*)(mp_attrdata+offset));
                        offset += 1+len8;
                }
	}
	return attr;
}

bool CBGPPathAttributeCollection::isValid() {
	if(GetAttributeByType(BGP_PATH_ATTR_ORIGIN) == NULL || GetAttributeByType(BGP_PATH_ATTR_AS_PATH) == NULL || GetAttributeByType(BGP_PATH_ATTR_NEXT_HOP) == NULL) {
		return false;
	}else {
		return true;
	}
}

bool CBGPPathAttributeCollection::Equal(CBGPPathAttributeCollection *attrs)
{
	unsigned int attrnum = GetAttrNumber();
	if(attrnum != attrs->GetAttrNumber())
		return false;
	for(unsigned int i = 0; i < attrnum; i++) {
		CBGPPathAttribute *attr = GetAttribute(i);
		if(attr->Compare(attrs->GetAttributeByType(attr->m_attr_type)) != 0) {
			return false;
		}
	}
	return true;
}

CBGPUpdateMessage::CBGPUpdateMessage()
{
}

CBGPUpdateMessage::CBGPUpdateMessage(CBGPUpdateMessage& clone)
{
	m_withdraw=clone.m_withdraw;
	m_pathattr=clone.m_pathattr;
	m_valid=clone.m_valid;
}

CBGPUpdateMessage::CBGPUpdateMessage(CBGPUpdateLog* dul,unsigned int msglen)
{
	m_withdraw.Set(dul, msglen - 19 - 2);
	if(msglen - 19 - 2 - m_withdraw.m_length > 0) {
		m_pathattr.Set(dul, msglen - 19 - 2 - m_withdraw.m_length);
	}
	//printf("[CBGPUpdateMessage] widthlen = %d, pathattrlen = %d\n", m_withdraw.m_length, m_pathattr.m_length);
	if(m_pathattr.m_length > 0 && msglen - 19 - 4 - m_withdraw.m_length - m_pathattr.m_length > 0) {
		if(m_pathattr.isValid()) {
			m_valid.SetLength(msglen - 19 - 4 - m_withdraw.m_length - m_pathattr.m_length);
			m_valid.SetDataFromLog(dul);
		}else {
			dul->rewind(2 + m_pathattr.m_length);
		}
	}
}

CBGPPathAttribute::CBGPPathAttribute()
{
}

CBGPPathAttribute::CBGPPathAttribute(CBGPPathAttribute& clone)
{
	m_attr_flag = clone.m_attr_flag;
	m_attr_type = clone.m_attr_type;
	if(IsExtend())
	{
		m_attr.m_ext_data.m_length = clone.m_attr.m_ext_data.m_length;
		memcpy(&m_attr.m_ext_data.m_hook, &clone.m_attr.m_ext_data.m_hook, GetLength());
	}else
	{
		m_attr.m_data.m_length = clone.m_attr.m_data.m_length;
		memcpy(&m_attr.m_data.m_hook, &clone.m_attr.m_data.m_hook, GetLength());
	}
}

CBGPPathAttribute::CBGPPathAttribute(CBGPUpdateLog* dul)
{
}

bool CBGPPathAttribute::IsOptional()
{
	if((m_attr_flag&0x80)!=0)
		return true;
	else
		return false;
}

bool CBGPPathAttribute::IsTransitive()
{
	if((this->m_attr_flag&0x40)!=0)
		return true;
	else
		return false;
}

bool CBGPPathAttribute::IsPartial()
{
	if((m_attr_flag&0x20)!=0)
		return true;
	else
		return false;
}

bool CBGPPathAttribute::IsExtend()
{
	if((m_attr_flag&0x10)!=0)
		return true;
	else
		return false;
}

u_int16_t CBGPPathAttribute::GetLength()
{
	u_int16_t len;
	if(IsExtend())
	{
		len=m_attr.m_ext_data.m_length;
		MyUtil::ConvType(&len,sizeof(u_int16_t));
	}else
	{
		len=0x00ff & m_attr.m_data.m_length;
	}
	return len;
}

void* CBGPPathAttribute::GetData()
{
	if(IsExtend())
	{
		return &m_attr.m_ext_data.m_hook;
	}else
	{
		return &m_attr.m_data.m_hook;
	}
}

int CBGPPathAttribute::Compare(CBGPPathAttribute* attr)
{
	if(attr == NULL || GetLength() > attr->GetLength()) return 1;
        else if(GetLength() < attr->GetLength()) return -1;
        else {
		if(GetLength() == 0) return 0;

		return memcmp(GetData(), attr->GetData(), GetLength());
        }
}

CBGPNLRI::CBGPNLRI()
{
}

CBGPNLRI::CBGPNLRI(CBGPNLRI& clone)
{
}

CBGPNLRI::CBGPNLRI(CBGPUpdateLog* dul)
{

}

char* CBGPNLRI::toString(int af)
{
	static char temp[MAX_PREFIX_LEN];
	u_int8_t prelen=m_length;
	unsigned int bytes=prelen/8;
	if(prelen%8!=0)
		bytes++;
	if(af==AF_INET)
	{
		if(bytes > 4) bytes = 4;
		CIPAddress addr;
		memcpy(&addr,&m_hook,bytes);
		addr.ConvType();
		sprintf(temp,"%s/%d",addr.toString(),prelen);
	}else if(af==AF_INET6)
	{
		if(bytes > 16) bytes = 16;
		CIPAddress6 addr6;
		memcpy(&addr6,&m_hook,bytes);
		addr6.ConvType();
		sprintf(temp,"%s/%d",addr6.toString(),prelen);
	}
	return temp;
}

CIPAddress CBGPNLRI::GetAddress()
{
	unsigned int bytes=m_length/8;
	if(m_length%8!=0)
        	bytes++;
	if(bytes > 4) bytes = 4;
	CIPAddress addr;
	memcpy(&addr,&m_hook,bytes);
	addr.ConvType();
	return addr;
}

CIPAddress6 CBGPNLRI::GetAddress6()
{
	unsigned int bytes=m_length/8;
	if(m_length%8!=0)
        	bytes++;
	if(bytes > 16) bytes = 16;
	CIPAddress6 addr6;
	memcpy(&addr6,&m_hook,bytes);
	addr6.ConvType();
	return addr6;
}

u_int8_t CBGPNLRI::GetPrefixLen()
{
	return m_length;
}

CBGPNotifyMessage::CBGPNotifyMessage()
{
	m_datalength=0;
}

CBGPNotifyMessage::~CBGPNotifyMessage()
{
	if(m_datalength>0)
	{
		delete mp_data;
	}
}

CBGPNotifyMessage::CBGPNotifyMessage(CBGPNotifyMessage& clone)
{
	m_error=clone.m_error;
	m_sub_err=clone.m_sub_err;
	m_datalength=clone.m_datalength;
	if(m_datalength>0)
	{
		mp_data=new unsigned char[m_datalength];
		memcpy(mp_data,clone.mp_data,m_datalength);
	}
}

CBGPNotifyMessage::CBGPNotifyMessage(CBGPUpdateLog* dul,unsigned int msglen)
{
	memcpy(&m_error,dul->GetValue(sizeof(u_int8_t)),sizeof(u_int8_t));
	MyUtil::ConvType(&m_error,sizeof(u_int8_t));
	memcpy(&m_sub_err,dul->GetValue(sizeof(u_int8_t)),sizeof(u_int8_t));
	MyUtil::ConvType(&m_sub_err,sizeof(u_int8_t));
	m_datalength=msglen-21;
	if(m_datalength>0)
	{
		mp_data=new unsigned char[m_datalength];
		memcpy(mp_data,dul->GetValue(m_datalength),m_datalength);
	}
}

char* CBGPNotifyMessage::toString()
{
	static char temp[256];
	sprintf(temp,"ErrCode: %d SubCode: %d Length: %d",m_error,m_sub_err,m_datalength);
	return temp;
}

CBGPMPReachNLRI::CBGPMPReachNLRI()
{
}

CBGPMPReachNLRI::~CBGPMPReachNLRI()
{
}

u_int8_t CBGPMPReachNLRI::GetSNPANumber()
{
	u_int8_t len=*((&m_nh_hook)+m_nh_len);
	return len;
}

void* CBGPMPReachNLRI::GetSNPAData(int index)
{
	u_int8_t* data=(&m_nh_hook)+m_nh_len+1;
	unsigned int offset=0;
	//if(index>=GetSNPANumber()) return NULL;
	for(int i=0;i<index;i++)
	{
		u_int8_t len=*(data+offset);
		offset+=1+len/2;
		if(len%2!=0)
			offset+=1;
	}
	return data+offset;
}

u_int16_t CBGPMPReachNLRI::GetNLRILength(u_int16_t len)
{
	u_int8_t* data=(&m_nh_hook)+m_nh_len+1;
	u_int8_t snpanum=GetSNPANumber();
	unsigned int offset=0;

	for(u_int8_t i=0;i<snpanum;i++)
	{
		u_int8_t snpalen=*(data+offset);
		offset+=1+snpalen/2;
		if(snpalen%2!=0)
			offset+=1;
	}

	return len-4-m_nh_len-1-offset;
}

void* CBGPMPReachNLRI::GetNLRIData()
{
	return GetSNPAData(GetSNPANumber());
}

u_int16_t CBGPMPReachNLRI::GetAFI()
{
	u_int16_t len=m_afi;
	MyUtil::ConvType(&len,sizeof(u_int16_t));
	return len;
}

CBGPMPUnreachNLRI::CBGPMPUnreachNLRI()
{
}

CBGPMPUnreachNLRI::~CBGPMPUnreachNLRI()
{
}

u_int16_t CBGPMPUnreachNLRI::GetNLRILength(u_int16_t len)
{
	return len-3;
}

void* CBGPMPUnreachNLRI::GetNLRIData()
{
	return &m_hook;
}

u_int16_t CBGPMPUnreachNLRI::GetAFI()
{
	u_int16_t len=m_afi;
	MyUtil::ConvType(&len,sizeof(u_int16_t));
	return len;
}

char* CBGPPathAttr_Origin::toString()
{
	static char temp[256];

	switch(*((u_int8_t*)GetData()))
	{
		case BGP_PATH_ATTR_ORIGIN_IGP:
			sprintf(temp, "origin: IGP");
			break;
		case BGP_PATH_ATTR_ORIGIN_EGP:
			sprintf(temp, "origin: EGP");
			break;
		case BGP_PATH_ATTR_ORIGIN_INCOMPLETE:
			sprintf(temp, "origin: INCOMPLETE");
			break;
	}
	return temp;
}

char* CBGPPathAttr_Origin::toBriefString()
{
	static char temp[256];

	switch(*((u_int8_t*)GetData()))
	{
		case BGP_PATH_ATTR_ORIGIN_IGP:
			sprintf(temp, "ORG: i");
			break;
		case BGP_PATH_ATTR_ORIGIN_EGP:
			sprintf(temp, "ORG: e");
			break;
		case BGP_PATH_ATTR_ORIGIN_INCOMPLETE:
			sprintf(temp, "ORG: ?");
			break;
	}
	return temp;
}

u_int8_t CBGPPathAttr_Origin::GetOrigin()
{
	return *((u_int8_t*)GetData());
}

char* CBGPPathAttr_NextHop::toString()
{
	static char temp[256];
	CIPAddress addr(GetData());
	addr.ConvType();
	sprintf(temp, "Next hop: %s",addr.toString());
	return temp;
}

char* CBGPPathAttr_NextHop::toBriefString()
{
	static char temp[256];
	CIPAddress addr(GetData());
	addr.ConvType();
	sprintf(temp, "NH: %s",addr.toString());
	return temp;
}


CIPAddress CBGPPathAttr_NextHop::GetNextHop()
{
	CIPAddress addr(GetData());
	addr.ConvType();
	return addr;
}

char* CBGPPathAttr_MED::toString()
{
	static char temp[256];
	u_int32_t med=*((u_int32_t*)GetData());
	MyUtil::ConvType(&med,sizeof(u_int32_t));
	sprintf(temp, "Med: %u", med);
	return temp;
}

char* CBGPPathAttr_MED::toBriefString()
{
	static char temp[256];
	u_int32_t med=*((u_int32_t*)GetData());
	MyUtil::ConvType(&med,sizeof(u_int32_t));
	sprintf(temp, "MED:%u", med);
	return temp;
}


u_int32_t CBGPPathAttr_MED::GetMed()
{
	u_int32_t med=*((u_int32_t*)GetData());
	MyUtil::ConvType(&med, sizeof(u_int32_t));
	return med;
}

char* CBGPPathAttr_LocalPref::toString()
{
	static char temp[256];
	u_int32_t localpref=*((u_int32_t*)GetData());
	MyUtil::ConvType(&localpref,sizeof(u_int32_t));
	sprintf(temp,"Local-pref: %u", localpref);
	return temp;
}

char* CBGPPathAttr_LocalPref::toBriefString()
{
	static char temp[256];
	u_int32_t localpref=*((u_int32_t*)GetData());
	MyUtil::ConvType(&localpref,sizeof(u_int32_t));
	sprintf(temp,"LOCP:%u", localpref);
	return temp;
}

u_int32_t CBGPPathAttr_LocalPref::GetLocalPref()
{
	u_int32_t localpref=*((u_int32_t*)GetData());
	MyUtil::ConvType(&localpref,sizeof(u_int32_t));
	return localpref;
}

char* CBGPPathAttr_Aggregator::toString()
{
	static char temp[256];
	sprintf(temp,"Aggregator: AS%d: %s", GetAggreAS(), GetAggregator().toString());
	return temp;
}

char* CBGPPathAttr_Aggregator::toBriefString()
{
	static char temp[256];
	sprintf(temp,"AGGR: %d:%s", GetAggreAS(), GetAggregator().toString());
	return temp;
}

u_int16_t CBGPPathAttr_Aggregator::GetAggreAS()
{
	u_int16_t as = *((u_int16_t*)GetData());
	MyUtil::ConvType(&as, sizeof(u_int16_t));
	return as;
}

CIPAddress CBGPPathAttr_Aggregator::GetAggregator()
{
	u_int32_t aggre = *((u_int32_t*)((u_int16_t*)GetData() + 1));
	CIPAddress aggregator(&aggre);
	aggregator.ConvType();
	return aggregator;
}

unsigned int CBGPPathAttr_ASPath::GetASPathNumber()
{
	u_int16_t offset=0;
	u_int8_t* data=(u_int8_t*)GetData();
	unsigned int num=0;

	while(offset<GetLength())
	{
		CBGPASPath* path=(CBGPASPath*)(data+offset);
		offset+=2+path->m_length*2;
		num++;
	}

	return num;
}

CBGPASPath* CBGPPathAttr_ASPath::GetASPath(unsigned int index)
{
	u_int16_t offset=0;
	u_int8_t* data=(u_int8_t*)GetData();
	unsigned int num=0;

	while(offset<GetLength()&& num<index)
	{
		CBGPASPath* path=(CBGPASPath*)(data+offset);
		offset+=2+path->m_length*2;
		num++;
	}

	return (CBGPASPath*)(data+offset);
}

u_int16_t CBGPPathAttr_ASPath::GetOriginAS()
{
 	CBGPASPath *path =  NULL;
	unsigned int i = 0;
	while(i < GetASPathNumber()) {
		path = GetASPath(i);
		if(path->m_type == BGP_PATH_ATTR_AS_PATH_SEQUENCE) {
			return path->GetASN(path->m_length - 1);
		}
		i++;
	}
	return 0;
}

char* CBGPPathAttr_ASPath::toBriefString()
{
	static char temp[1024];
	strcpy(temp, "PATH:");
	for(unsigned int i = 0; i < GetASPathNumber(); i ++) {
		CBGPASPath* path = GetASPath(i);
		if(path->m_type == BGP_PATH_ATTR_AS_PATH_SET) {
			strcat(temp, "{");
		}
		for(int j = 0; j < path->m_length; j ++) {
			sprintf(temp, "%s %u", temp, path->GetASN(j));
		}
		if(path->m_type == BGP_PATH_ATTR_AS_PATH_SET) {
			strcat(temp, "}");
		}
	}
	return temp;
}

CBGPASPath::CBGPASPath()
{
}

char* CBGPASPath::toString()
{
	static char temp[1024];
	switch(m_type)
	{
	case BGP_PATH_ATTR_AS_PATH_SET:
		sprintf(temp,"AS_SET:");
		break;
	case BGP_PATH_ATTR_AS_PATH_SEQUENCE:
		sprintf(temp,"AS_SEQUENCE:");
		break;
	}
	for(int i=0;i<m_length;i++)
	{
		sprintf(temp,"%s %u",temp,GetASN(i));
	}
	return temp;
}

u_int16_t CBGPASPath::GetASN(int index)
{
	u_int16_t asnum=*(((u_int16_t*)&m_hook)+index);
	MyUtil::ConvType(&asnum,sizeof(u_int16_t));
	return asnum;
}

u_int16_t CBGPASPath::GetAbsoluteLength()
{
	u_int16_t len=1;

	for (int i=1;i<m_length;i++)
	{
		if (GetASN(i-1) != GetASN(i))
		{
			len++;
		}
	}
	return len;
}

u_int16_t CBGPPathAttr_Community::GetCommunityNumber() 
{
	return (u_int16_t)(GetLength()/4);
}

u_int32_t CBGPPathAttr_Community::GetCommunity(int idx) 
{
	u_int32_t *commus = (u_int32_t*)GetData();
	u_int32_t com = commus[idx];
	MyUtil::ConvType(&com, sizeof(u_int32_t));
	return com;
}

char* CBGPPathAttr_Community::toString() 
{
	static char temp[256];

	strcpy(temp, "Community: ");

	for(int i = 0; i < GetCommunityNumber(); i ++) 
	{
		sprintf(temp, "%s[%08X]", temp, GetCommunity(i));
	}
	return temp;
}

char* CBGPPathAttr_Community::toBriefString()
{
	static char temp[256];
	strcpy(temp, "COMM: ");
	for(int i = 0; i < GetCommunityNumber(); i ++) 
	{
		sprintf(temp, "%s[%08X]", temp, GetCommunity(i));
	}
	return temp;
}

int getBGPUpdateType(char* filename) {
	CBGPUpdateLog bgplog(filename);
	CBGPUpdateHeader header(&bgplog);
	if(header.m_type == MRT_BGP4MP && header.m_subtype == BGP4MP_MESSAGE) {
		CBGPUpdateASHeader asheader(&bgplog);
		if(asheader.m_af == AFI_IPV4) {
			return BGP_UPDATE_MESSAGE_V4;
		}else if(asheader.m_af == AFI_IPV6) {
			return BGP_UPDATE_MESSAGE_V6;
		}
	}else if(header.m_type == MRT_TABLE_DUMP && header.m_subtype == AFI_IPV4) {
		return BGP_VIEW_DUMP_V4;
	}else if(header.m_type == MRT_TABLE_DUMP && header.m_subtype == AFI_IPV6) {
		return BGP_VIEW_DUMP_V6;
	}
	return UNKNOWN_LOG;
}

int readBGPUpdateMessageV4(char* filename, int (*processUpdateMessage4)(time_t tm, CBGPUpdateMessage* msg, CIPAddress peer, void* param), void* param) {
	CBGPUpdateLog bgplog(filename);
	if(processUpdateMessage4 == NULL) {
		return 0;
	}
	
	while(!bgplog.isEnd())
	{
		CBGPUpdateHeader header(&bgplog);
#ifdef _UPDATE_DEBUG_
		printf("%s\n", header.toString());
#endif
		if(header.m_type == MRT_BGP4MP && header.m_subtype == BGP4MP_MESSAGE) {
			CBGPUpdateASHeader asheader(&bgplog);
#ifdef _UPDATE_DEBUG_
			printf("%s\n", asheader.toString());
#endif
			if(asheader.m_af == AFI_IPV4) {
				CBGPMessageHeader msgheader(&bgplog);
#ifdef _UPDATE_DEBUG_
				printf("%s\n", msgheader.toString());
#endif
				if(msgheader.m_length >= 19 && msgheader.m_length <= 4096) {
					if(msgheader.m_type == BGP_MESSAGE_UPDATE && msgheader.m_length > 19 + 2 + 2) {
						CBGPUpdateMessage message(&bgplog, msgheader.m_length);
						if((*processUpdateMessage4)(header.m_time, &message, msgheader.m_src_ip, param) != 0) {
							return -1;
						}
					}else {
						bgplog.GetValue(msgheader.m_length - BGP_UPDATE_MESSAGE_HEADER_LENGTH); // skip
					}
				}else {
					if(!bgplog.rewind(sizeof(CBGPUpdateHeader) + sizeof(CBGPUpdateASHeader) + sizeof(CBGPMessageHeader) - 1)) {
#ifdef _UPDATE_DEBUG_
						printf("Rewind fails at CBGPMessageHeader\n");
#endif
					}
				}
			}else {
				if(!bgplog.rewind(sizeof(CBGPUpdateHeader) + sizeof(CBGPUpdateASHeader) - 1)) {
#ifdef _UPDATE_DEBUG_
					printf("Rewind fails at CBGPUpdateASHeader\n");
#endif
				}
			}
		}else {
			if(!bgplog.rewind(sizeof(CBGPUpdateHeader)-1)) {
#ifdef _UPDATE_DEBUG_
				printf("Rewin fails at CBGPUpdateHeader\n");
#endif
			}
		}
	}

	return 0;
}

int readBGPViewDumpV4(char* filename, int (*processViewMessage4)(time_t tm, CBGPUpdateViewHeader* head, CBGPPathAttributeCollection *attr, void* param), void* param)
{
	CBGPUpdateLog bgplog(filename);
	if(processViewMessage4 == NULL) {
		return 0;
	}
	
	while(!bgplog.isEnd())
	{
		CBGPUpdateHeader header(&bgplog);
#ifdef _UPDATE_DEBUG_
		printf("%s\n", header.toString());
#endif
		if(header.m_type == MRT_TABLE_DUMP && header.m_subtype == AFI_IPV4 && processViewMessage4 != NULL) {
			CBGPUpdateViewHeader viewhead(&bgplog);
#ifdef _UPDATE_DEBUG_
			printf("%s\n", viewhead.toString());
#endif
			CBGPPathAttributeCollection pathattrs;
			void* buf = bgplog.GetValue(viewhead.m_attrlen);
			if(buf != NULL) {
				pathattrs.Set(viewhead.m_attrlen, buf);
				if((*processViewMessage4)(header.m_time, &viewhead, &pathattrs, param) != 0) {
					return -1;
				}
			}
		}
	}

	return 0;
}

int readBGPUpdateLog(char* filename, int (*processUpdateMessage4)(time_t tm, CBGPUpdateMessage* msg, CIPAddress peer, void* param), int (*processUpdateMessage6)(time_t tm, CBGPUpdateMessage *msg, CIPAddress6 peer, void* param), int (*processViewMessage4)(time_t tm, CBGPUpdateViewHeader* head, CBGPPathAttributeCollection *attr, void* param), int (*processViewMessage6)(time_t tm, CBGPUpdateViewHeader6* head, CBGPPathAttributeCollection *attr, void* param), int (*processOtherMessage)(time_t tm, u_int16_t type, u_int16_t subtype, u_int32_t length, void* data, void* param), void* param)
{
	CBGPUpdateLog bgplog(filename);
	
	while(!bgplog.isEnd())
	{
		//messagenum++;
		CBGPUpdateHeader header(&bgplog);
#ifdef _UPDATE_DEBUG_
		printf("%s\n", header.toString());
#endif
		if(header.m_type == MRT_BGP4MP && header.m_subtype == BGP4MP_MESSAGE && (processUpdateMessage4 != NULL || processUpdateMessage6 != NULL)) {
			CBGPUpdateASHeader asheader(&bgplog);
#ifdef _UPDATE_DEBUG_
			printf("%s\n", asheader.toString());
#endif

			if(asheader.m_af == AFI_IPV4)
			{
				CBGPMessageHeader msgheader(&bgplog);
#ifdef _UPDATE_DEBUG_
				printf("%s\n", msgheader.toString());
#endif
				if(msgheader.m_type == BGP_MESSAGE_UPDATE && msgheader.m_length > 0 && msgheader.m_length <= 4096 && processUpdateMessage4 != NULL) {
					CBGPUpdateMessage message(&bgplog, msgheader.m_length);
					if((*processUpdateMessage4)(header.m_time, &message, msgheader.m_src_ip, param) != 0) {
						return -1;
					}
				}else {
					bgplog.GetValue(msgheader.m_length - BGP_UPDATE_MESSAGE_HEADER_LENGTH); // skip
				}
			}else if(asheader.m_af == AFI_IPV6)
			{
				CBGPMessageHeader6 msgheader6(&bgplog);
#ifdef _UPDATE_DEBUG_
				printf("%s\n", msgheader6.toString());
#endif
				if(msgheader6.m_type == BGP_MESSAGE_UPDATE && msgheader6.m_length > 0 && msgheader6.m_length <= 4096 && processUpdateMessage6 != NULL) {
					CBGPUpdateMessage message(&bgplog, msgheader6.m_length);
					if((*processUpdateMessage6)(header.m_time, &message, msgheader6.m_src_ip, param) != 0) {
						return -1;
					}
				}else {
					bgplog.GetValue(msgheader6.m_length - BGP_UPDATE_MESSAGE_HEADER_LENGTH); // skip
				}
			}
		}else if(header.m_type == MRT_TABLE_DUMP && header.m_subtype == AFI_IPV4 && processViewMessage4 != NULL) {
			CBGPUpdateViewHeader viewhead(&bgplog);
#ifdef _UPDATE_DEBUG_
			printf("%s\n", viewhead.toString());
#endif
			CBGPPathAttributeCollection pathattrs;
			pathattrs.Set(viewhead.m_attrlen, bgplog.GetValue(viewhead.m_attrlen));
			if((*processViewMessage4)(header.m_time, &viewhead, &pathattrs, param) != 0) {
				return -1;
			}
		}else if(header.m_type == MRT_TABLE_DUMP && header.m_subtype == AFI_IPV6 && processViewMessage6 != NULL) {
			CBGPUpdateViewHeader6 viewhead(&bgplog);
#ifdef _UPDATE_DEBUG_
			printf("%s\n", viewhead.toString());
#endif
			CBGPPathAttributeCollection pathattrs;
			pathattrs.Set(viewhead.m_attrlen, bgplog.GetValue(viewhead.m_attrlen));
			if((*processViewMessage6)(header.m_time, &viewhead, &pathattrs, param) != 0) {
				return -1;
			}
		}else
		{
			if((header.m_type <= 12 || header.m_type == 16 || header.m_type == 17 || header.m_type == 32 || header.m_type == 33 || header.m_type == 64) && (header.m_length <= 4096 + 256)) {
				void* data = bgplog.GetValue(header.m_length); // skip
				if(processOtherMessage != NULL) {
					if((*processOtherMessage)(header.m_time, header.m_type, header.m_subtype, header.m_length, data, param) != 0) {
						return -1;
					}
				}
			}else {
				//if(isVerbose) printf("########## Unknown Type ##########\n");
			}
		}
	}

	return 0;
}

char* toTypeString(u_int16_t type, u_int16_t subtype) {
	static char tempstr[256];
	switch(type) {
		case MRT_NULL:
			strcpy(tempstr, "MRT_NULL");
			break;
		case MRT_START:
			strcpy(tempstr, "MRT_START");
			break;
		case MRT_DIE:
			strcpy(tempstr, "MRT_DIE");
			break;
		case MRT_I_AM_DEAD:
			strcpy(tempstr, "MRT_I_AM_DEAD");
			break;
		case MRT_PEER_DOWN:
			strcpy(tempstr, "MRT_PEER_DOWN");
			break;
		case MRT_BGP:
			strcpy(tempstr, "MRT_BGP");
			break;
		case MRT_RIP:
			strcpy(tempstr, "MRT_RIP");
			break;
		case MRT_IDRP:
			strcpy(tempstr, "MRT_IDRP");
			break;
		case MRT_RIPNG:
			strcpy(tempstr, "MRT_RIPNG");
			break;
		case MRT_BGP4PLUS:
			strcpy(tempstr, "MRT_BGP4PLUS");
			break;
		case MRT_BGP4PLUS_01:
			strcpy(tempstr, "MRT_BGP4PLUS_01");
			break;
		case MRT_OSPF:
			strcpy(tempstr, "MRT_OSPF");
			break;
		case MRT_TABLE_DUMP:
			strcpy(tempstr, "MRT_TABLE_DUMP");
			break;
		case MRT_BGP4MP:
			strcpy(tempstr, "MRT_BGP4MP");
			switch(subtype) {
				case BGP4MP_STATE_CHANGE: strcat(tempstr, "/BGP4MP_STATE_CHANGE"); break;
				case BGP4MP_MESSAGE: strcat(tempstr, "/BGP4MP_MESSAGE"); break;
				case BGP4MP_ENTRY: strcat(tempstr, "/BGP4MP_ENTRY"); break;
				case BGP4MP_SNAPSHOT: strcat(tempstr, "/BGP4MP_SNAPSHOT"); break;
				case BGP4MP_MESSAGE_32BIT_AS: strcat(tempstr, "/BGP4MP_MESSAGE_32BIT_AS"); break;
				default: strcat(tempstr, "/UNKNOWN");
			}
			break;
		case MRT_BGP4MP_ET:
			strcpy(tempstr, "MRT_BGP4MP_ET");
			break;
		case MRT_ISIS:
			strcpy(tempstr, "MRT_ISIS");
			break;
		case MRT_ISIS_ET:
		  	strcpy(tempstr, "MRT_ISIS_ET");
			break;
		case MRT_OSPF_ET:
			strcpy(tempstr, "MRT_OSPF_ET");
			break;
		default:
			strcpy(tempstr, "UNKNOWN");
	}
	return tempstr;
}

}
