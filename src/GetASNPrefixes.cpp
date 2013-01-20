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
    if (argc < 3) {
        printf("Usage: GetASNPrefixes <dbpath> <asn>\n");
        return -1;
    }

    int exit_code = 0;
    u_int32_t asn = asn_a2n(argv[2]);
    if (!is_valid_asn(asn)) {
        printf("Invalid ASN: %d\n", asn);
        return -1;
    }
    CPrefixASNMap pam(argv[1], true);
    if (!pam.PrintAllPrefixOfASN(asn)) {
        exit_code = 1;
    }
    pam.Release();

    return exit_code;
}
