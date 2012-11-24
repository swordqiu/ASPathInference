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
 * Jian Qiu
 *
 * BatchCreateSimpleRoutingTable5.cpp
 *
 * April 16, 2007
 *
 */

#include "StdAfx.h"
#include "MyUtil.h"
#include "VMEMstd.h"
#include "VMEM.h"
#include "Address.h"
#include "BufFile.h"
#include "BGPUpdate.h"
#include "SimpleRoutingTable2.h"

using namespace bgplib;

bool isVerbose = false;

bool prepending = false;

struct PeerRec{
	CIPAddress addr;
	u_int32_t  asn;
};

void ProcessASPath2(CBGPPathAttr_ASPath* attr, CDynaArray* path) {
	u_int32_t lastasn = 0;
	u_int32_t tempasn = 0;
	for(unsigned int i = 0; i < attr->GetASPathNumber(); i ++) {
		CBGPASPath* tmppath = attr->GetASPath(i);
		for(int j = 0; j < tmppath->m_length; j ++) {
			tempasn = tmppath->GetASN(j);
			if(tempasn > 0 && tempasn < 34816 && (lastasn != tempasn || prepending)) {
				lastasn = tempasn;
				path->Add(&lastasn);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	//CMySQL mysql;
	if(argc<4)
	{
		printf("usage: UpdateParser <dbpath> <view> <peerlist> [prepending]\n");
		exit(1);
	}
	//char* logname="bgp.update.021009.1102.log";
	CBGPUpdateLog bgplog(argv[2]);
	//unsigned int messagenum=0;
	struct PeerRec pr;
	CPrefix prefix;
	CDynaArray path(sizeof(u_int32_t));
	CDynaArray peerlist(sizeof(struct PeerRec));

	if(argc >= 5 && strcmp(argv[4], "true") == 0) {
		prepending = true;
	}

	//if(strcmp(argv[2],"true")==0)
	//	isVerbose=true;

	while(!bgplog.isEnd())
	{
		//messagenum++;
		CBGPUpdateHeader header(&bgplog);
		if(isVerbose) printf("%s\n",header.toString());
		if(header.m_type==12 && header.m_subtype==1) {
			CBGPUpdateViewHeader viewhead(&bgplog);

			CBGPPathAttributeCollection pathattrs;
			pathattrs.Set(viewhead.m_attrlen, bgplog.GetValue(viewhead.m_attrlen));

			CBGPPathAttribute* attr = pathattrs.GetAttributeByType(BGP_PATH_ATTR_AS_PATH);
			if(attr != NULL) {
				ProcessASPath2((CBGPPathAttr_ASPath*)attr, &path);
	
				u_int8_t pathlen = path.GetLength();
				if(pathlen > 0) {
					pr.addr = viewhead.m_peer_ip;
					pr.asn  = viewhead.m_peer_as;

					if(peerlist.Find(&pr) < 0) {
						peerlist.Add(&pr);
					}
	
					prefix.Set(viewhead.m_prefix, viewhead.m_mask);
					u_int32_t* pathptr = (u_int32_t*)path.Get(0);
					//printf("%s %d %d ", prefix.toString(), pathlen, pathptr[0]);
					//MyUtil::ShowPath(pathptr, pathlen);

					CSimpleRoutingTable srt(argv[1], pathptr[0], false);
					srt.AddRecord2(&prefix, pathlen, pathptr);
					srt.Release();
					path.RemoveAll();
				}
			}
		}else
		{
			if(header.m_type == 12 || header.m_type == 16) {
				bgplog.GetValue(header.m_length); // skip
			}else {
				if(isVerbose) printf("########## Unknown Type ##########\n");
			}
		}
	}

	FILE* fp = fopen(argv[3], "a");
	for(int i = 0; i < peerlist.GetLength(); i ++) {
		struct PeerRec* ppr = (struct PeerRec*)peerlist.Get(i);
		fprintf(fp, "%s %d\n", ppr->addr.toString(), ppr->asn);
	}
	fclose(fp);

	//if(isVerbose) printf("Message Number: %d\n UPDATE: %ld", messagenum, bgplog.m_updatenum);
	return 0;
}
