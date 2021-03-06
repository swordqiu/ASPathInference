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
 * GetSurePath.cpp
 *
 * Author: Jian Qiu (swordqiu@gmail.com)
 *
 * Date: 12/1/2012
 **/

#include "StdAfx.h"
#include "VMEMstd.h"
#include "VMEM.h"
#include "Address.h"
#include "MyUtil.h"
#include "SimpleRoutingTable2.h"

using namespace bgplib;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: GetSurePath <dbpath> <asn> <prefix>\n");
        return -1;
    }

	char temp[4096];
	bzero(temp, sizeof(temp));

    int exit_code = 0;
    u_int32_t asn = asn_a2n(argv[2]);
    //printf("ASN: %d\n", asn);
    CPrefix prefix(argv[3]);
    //printf("Prefix: %s\n", prefix.toString());
    CSimpleRoutingTable crt(argv[1], asn, true);
    if (crt.GetAllPathStr(&prefix, 0, temp)) {
        printf("%s", temp);
    }else{
        exit_code = 1;
    }
    crt.Release();

    return exit_code;
}
