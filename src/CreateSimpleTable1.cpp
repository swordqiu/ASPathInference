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




/**
 * InsertSimpleRoutingTable.cpp
 * 
 * Author: Jian Qiu (jqiu@ecs.umass.edu)
 *
 * Date: 12/26/2004
 **/
 
#include "StdAfx.h"
#include "Address.h"
#include "VMEMstd.h"
#include "VMEM.h"
#include "BufFile.h"
#include "MyUtil.h"
#include "SimpleRoutingTable2.h"

using namespace bgplib;

struct PeerRec {
	CIPAddress addr;
	u_int32_t  asn;
};

int main(int argc, char* argv[]) {
	if(argc < 4) {
		printf("Usage: batchCrateSimpleTable3 <dbpath> <routingtable> <peerlist> [prepending]\n");
		return -1;
	}

	CDynaArray pathstr(sizeof(u_int32_t));
	u_int32_t* pathptr;
	u_int8_t pathlen;
	u_int32_t tempasn, lsttmpasn;
	CPrefix prefix;
	//CPrefixASNMap sureASprefix(argv[2], false);
	//CPrefixASNMap prefixASmap(argv[3], false);
	CDynaArray peerlist(sizeof(struct PeerRec));
	bool prepending = false;
	if(argc >= 5 && strcmp(argv[4], "true") == 0) {
		prepending = true;
	}

	BufFile bufile;
	char* buffer;
	char buftemp[64];
	struct PeerRec pr;
	unsigned int start = 0;
	if(!bufile.Open(argv[2], BUF_FILE_READ_MODE)) {
		printf("Error, cannot open file %s", argv[4]);
		return -1;
	}
	while((buffer = bufile.getnextstr()) != NULL) {
		if(MyUtil::isPathStatus(buffer)) {
			//printf("%s", buffer);
			if(buffer[PATH_DEST_BEGIN] != ' ') {
				buftemp[0] = '\0';
				getstring(buftemp,buffer,PATH_DEST_BEGIN);
				prefix.ParseStr(buftemp);
				//if(prefix.isPublic() && prefix.m_prefixlen < 30) {
				//	printf("%s\n", prefix.toString());
				//}
			}
			if(buffer[PATH_NEXT_BEGIN-1]!=' ') {
				buffer=bufile.getnextstr();
			}
			if(prefix.isPublic() && prefix.m_prefixlen < 30) {
				start=PATH_ASLIST_BEGIN;
				if(strchr(buffer + start, '{') != NULL) {
					continue;
				}
				printf("%s\n", prefix.toString());
				pathstr.RemoveAll();
				lsttmpasn = 0xFFFFFFFF;
				while(start < strlen(buffer)) {
					start = getstring(buftemp, buffer, start);
					if(buftemp[0] == 'i' || buftemp[0] == '?' || buftemp[0] == 'e') {
						break;
					}
					tempasn = asn_a2n(buftemp);
					if(is_valid_asn(tempasn) && (tempasn != lsttmpasn || prepending)) {
						pathstr.Add(&tempasn);
						lsttmpasn = tempasn;
					}
				}
				if(pathstr.GetLength() > 1) {
					pathptr = (u_int32_t*)pathstr.Get(0);
					pathlen = pathstr.GetLength();
					CSimpleRoutingTable srt(argv[1], pathptr[0], false);
					//MyUtil::ShowPath(&pathptr[0], pathlen);
					//MyUtil::ShowPath(&pathptr_rev[pathlen-i-1], i+1);
					srt.AddRecord2(&prefix, pathlen, &pathptr[0]);
					srt.Release();
					//sureASprefix.AddRecord(&prefix, pathptr[0]);
					//prefixASmap.AddRecord(&prefix, pathptr[pathlen - 1]);
					getstring(buftemp, buffer, PATH_NEXT_BEGIN);
					pr.addr.ParseStr(buftemp);
					pr.asn = pathptr[0];
					if(peerlist.Find(&pr) < 0) {
						peerlist.Add(&pr);
					}
				}
			}
		}
	}

	FILE* fp;
	if((fp = fopen(argv[3], "a")) == NULL) {
		printf("Error open peerlist file!\n");
		exit(-1);
	}
	for(int i = 0; i < peerlist.GetLength(); i ++) {
		struct PeerRec* ppr = (struct PeerRec*)peerlist.Get(i);
		fprintf(fp, "%s %d\n", ppr->addr.toString(), ppr->asn);
	}
	fclose(fp);
	//sureASprefix.Release();
	//prefixASmap.Release();
	bufile.Close();
	
	return 0;
}
