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
 * Create cumulative Prefix ASPath map and ASN-prefix map
 *
 * Author: Jian Qiu (swordqiu@gmail.com)
 *
 * Date: 1/2/2012
 **/
 
#include "StdAfx.h"
#include "Address.h"
#include "VMEMstd.h"
#include "VMEM.h"
#include "BufFile.h"
#include "MyUtil.h"
#include "SimpleRoutingTable2.h"

using namespace bgplib;

int main(int argc, char* argv[]) {
	if(argc < 3) {
		printf("Usage: batchCrateSimpleTable3 <dbpath> <routingtable> [number_of_rec] [type]\n");
        printf("       type: prefix_path, asn_prefix, all, none\n");
		return -1;
	}

	CDynaArray pathstr(sizeof(u_int32_t));
	u_int32_t* pathptr;
	u_int8_t pathlen;
	u_int32_t tempasn, lsttmpasn;
	CPrefix prefix;
	//CPrefixASNMap sureASprefix(argv[2], false);
	CPrefixASNMap prefixASmap(argv[1], false);

    long long max_count = 0;
    long long line_count = 0;

    if (argc > 3) {
        max_count = atoll(argv[3]);
    }

    bool rec_prefix_path = true;
    bool rec_asn_prefix = true;
    if (argc > 4) {
        if (strcmp(argv[4], "prefix_path") == 0) {
            rec_asn_prefix = false;
        }else if (strcmp(argv[4], "asn_prefix") == 0) {
            rec_prefix_path = false;
        }else if (strcmp(argv[4], "none") == 0) {
            rec_prefix_path = false;
            rec_asn_prefix = false;
        }
    }

	BufFile bufile;
	char* buffer;
	char buftemp[64];
	unsigned int start = 0;
	if(!bufile.Open(argv[2], BUF_FILE_READ_MODE)) {
		printf("Error, cannot open file %s", argv[4]);
		return -1;
	}
	while((buffer = bufile.getnextstr()) != NULL && (max_count <= 0 || line_count < max_count)) {
		if(MyUtil::isPathStatus(buffer)) {
            line_count ++;
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
				//printf("%s\n", prefix.toString());
				pathstr.RemoveAll();
				lsttmpasn = 0xFFFFFFFF;
				while(start < strlen(buffer)) {
					start = getstring(buftemp, buffer, start);
					if(buftemp[0] == 'i' || buftemp[0] == '?' || buftemp[0] == 'e') {
						break;
					}
					tempasn = asn_a2n(buftemp);
					if(is_valid_asn(tempasn) && (tempasn != lsttmpasn)) {
						pathstr.Add(&tempasn);
						lsttmpasn = tempasn;
					}
				}
				if(pathstr.GetLength() > 1) {
                    //printf("%s", prefix.toString());
					//MyUtil::ShowPath(&pathptr[0], pathlen);
					//MyUtil::ShowPath(&pathptr_rev[pathlen-i-1], i+1);
					pathptr = (u_int32_t*)pathstr.Get(0);
					pathlen = pathstr.GetLength();
                    if (rec_prefix_path) {
					    CSimpleRoutingTable srt(argv[1], pathptr[0], false);
					    srt.AddRecord2(&prefix, pathlen, &pathptr[0]);
					    srt.Release();
                    }
					//sureASprefix.AddRecord(&prefix, pathptr[0]);
                    if (rec_asn_prefix) {
					    prefixASmap.AddRecord(&prefix, pathptr[pathlen - 1]);
                    }
					/*getstring(buftemp, buffer, PATH_NEXT_BEGIN);
					pr.addr.ParseStr(buftemp);
					pr.asn = pathptr[0];
					if(peerlist.Find(&pr) < 0) {
						peerlist.Add(&pr);
					}*/
				}
			}
		}
	}

	//sureASprefix.Release();
	prefixASmap.Release();
	bufile.Close();
	
	return 0;
}
