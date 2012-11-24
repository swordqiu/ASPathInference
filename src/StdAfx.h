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




#ifndef _STDAFX_H
#define _STDAFX_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
//#include <arpa/inet.h>
#include <time.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
using namespace std;

#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define INVALID_CHILDNODE_INDEX -1

#define PI 3.1415926535

#define LH +1
#define EH 0
#define RH -1

#define BIGGER_AND_CONTAIN		0
#define SMALLER_AND_CONTAIN		1
#define DONT_CONTAIN			2

#define AS_STACK_DEPTH 1024

#define VMEM_FILE_BUFFER_SIZE (2048)

#define MAX_AS_DEPTH 255
#define MAX_AS_NUMBER 65535
#define AS_NUMBER_IN_USE 0xde

#define VSEEK_SET 0
#define VSEEK_CUR 1
#define VSEEK_END 2

#define LEFT_SIDE_CHILD		0
#define RIGHT_SIDE_CHILD	1
#define UNKNOWN_SIDE_CHILD	2

#define MAX_INTEGER 2147483647
#define MAX_SHORT_INTEGER 32767
#define MAX_UNSIGNED_INTEGER 65535
#define INVALID_DOUBLE 3.4E38

#define DYNA_ARRAY_INIT_SIZE 50
#define DYNA_ARRAY_STEP_SIZE 10

#define MAX_PATH_LEN         256
#define MAX_LEN              1024


#define BGP_UPDATE_TYPE_WITHDRAW 1
#define BGP_UPDATE_TYPE_ANNOUNCE 0

#define PATH_STATUS	"sdh*>i"
#define PATH_SUPPRESSED      's'
#define PATH_DAMPED          'd'
#define PATH_HISTORY         'h'
#define PATH_VALID           '*'
#define PATH_BEST            '>'
#define PATH_INTERNAL        'i'

#define PATH_ORIGIN          "ie?"
#define PATH_ORIGIN_IGP      'i'
#define PATH_ORIGIN_EGP      'e'
#define PATH_ORIGIN_INCOMPLETE    '?'

#define AS_SORT_BYOUTDEGREE   0
#define AS_SORT_BYPATHNUM     1
#define AS_SORT_BYADDRSPACE   2
#define AS_SORT_BYCOUNTRY	  3

#ifndef IPV6
	#define PATH_STATUS_BEGIN 0
	#define PATH_STATUS_LEN	  3
	#define PATH_DEST_BEGIN   3
	#define PATH_NEXT_BEGIN   20
	#define PATH_METRIX_BEGIN 40
	#define PATH_METRIX_LEN   6
	#define PATH_LOCPRF_BEGIN 47
	#define PATH_LOCPRF_LEN   6
	#define PATH_WEIGHT_BEGIN 54
	#define PATH_WEIGHT_LEN   6
	#define PATH_ASLIST_BEGIN 61
#else
	#define PATH_STATUS_BEGIN 0
	#define PATH_STATUS_LEN   3
	#define PATH_DEST_BEGIN   3
	#define PATH_NEXT_BEGIN   5
	#define PATH_METRIX_BEGIN 45
	#define PATH_METRIX_LEN   6
	#define PATH_LOCPRF_BEGIN 51
	#define PATH_LOCPRF_LEN   6
	#define PATH_WEIGHT_BEGIN 58
	#define PATH_WEIGHT_LEN   6
	#define PATH_ASLIST_BEGIN 65
#endif

#define AVERAGE_PATH_LEN   "average_path.data"
#define MAX_ASPATH_LEN     "max_path.data"
#define AVERAGE_OUTDEGREE  "average_out.data"
#define MAX_OUTDEGREE      "max_out.data"
#define ROUTE_NUM          "route_num.data"
#define AS_PEER_NUM        "as_peer_num.data"
#define AS_NUM             "as_num.data"
#define ADDR_PREFIX_LEN    "prefix_len.data"
#define AVERAGE_PREFIX_LEN "average_prefix_len.data"
#define MIN_PREFIX_LEN_FILE    "min_prefix_len.data"
#define MAX_PREFIX_LEN_FILE     "max_prefix_len.data"
#define AVERAGE_ADDRESS    "average_address.data"

#define MORE_SPECIFIC_NUM  "more_specific.data"
#define MORE_SPECIFIC_PER  "more_specific_percentage.data"
#define ADDRESS_TIERS      "address_tier.data"
#define ADDRESS_NUMBER	   "address_num.data"
#define MORE_SPEC_ADDR_PER "more_specific_address_percentage.data"	
#define USABLE_ADDR_PER	   "address_util_percentage.data"
#define AGGREGATABLE_ROUTE "aggregatable_route.data"
#define AGGREGATABLE_PER   "aggregatable_route_percentage.data"

#ifndef IPV6
	#define MINIMUM_PREFIX		8
#else
	#define MINIMUM_PREFIX		24
#endif

#ifndef IPV6
	#define ERROR_ROUTE_THRES  30
	#define STATICS_HTML       "/www/htdocs/bgp-view/statics_ipv4.html"
#else
	#define ERROR_ROUTE_THRES  64
	#define STATICS_HTML       "/www/htdocs/bgp-view/statics_ipv6.html"
#endif

#define AXIS_CYLINDER 0
#define AXIS_CUBE	  1

#define SORT_DESCENDENT 0 //descendent
#define SORT_ASCENDENT  1 //ascendent

typedef u_int64_t UINT_64;
typedef int64_t INT_64;

typedef u_int16_t _asnum_t;

#define ASPATH_TYPE_SEQUENCE 0
#define ASPATH_TYPE_SET		 1
#define ADD_OTHER_AS_PAIR  1

#define AS_RELATION_UNKNOWN		-1
#define AS_RELATION_SIBLING_SIBLING	 0
#define AS_RELATION_PROVIDER_CUSTOMER	 1
#define AS_RELATION_CUSTOMER_PROVIDER	 2
#define AS_RELATION_PEER_PEER		 3

#define AS_RELATION_THRESHOLD_L		 1
#define AS_RELATION_THRESHOLD_R		 60
#define AS_RELATION_THRESHOLD_O		 5
#define AS_RELATION_THRESHOLD_RATIO	 2
#define AS_RELATION_THRESHOLD_ADDR	 2048

#define UNKNOWN_AS_TYPE 2
#define PROVIDER_AS 0
#define CUSTOMER_AS 1

#define MAX_ADDRESS_LEN 40 //5*8
#define MAX_PREFIX_LEN  45 // 40 + 3 /128

/*#define ASSERT(f, str) \
	if(!(f)) {\
		printf("[ASSERT ERROR]: %s", (const char*)(str));\
		exit(-1);\
	}
*/

#define max(x, y) ((x)>=(y)? (x):(y))
#define min(x, y) ((x)<=(y)? (x):(y))

#define isTimeStr(x) (strlen((x)) == strlen("YYYY-mm-dd hh:ii:ss") && (x)[4] == '-')

#define MALLOC_CHECK_ 0

#endif
