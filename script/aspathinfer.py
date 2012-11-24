#!/usr/bin/env python

import sys;
import string;
import urllib;
import re;

def inferPath(srcasn_, dstprefix_, cmpfunc_, feedback_):
	if dstprefix_.find("/") < 0:
		dstprefix_ = dstprefix_ + "/32";
  	if srcasn_ != 0 and re.compile("\\d+\\.\\d+\\.\\d+\\.\\d+").match(dstprefix_) and (cmpfunc_ == "SPF" or cmpfunc_ == "LUF") and (feedback_ == "NO_FEEDBACK" or feedback_ == "WITH_FEEDBACK"):
		if cmpfunc_ == "SPF":
			cmpcode = 2;
		else:
			cmpcode = 1;
		if feedback_ == "NO_FEEDBACK":
			feedcode = 2;
		else:
			feedcode = 1;
     		url = "http://127.0.0.1:61002/infer?asn_=" + str(srcasn_) + "&prefix_=" + dstprefix_ + "&cmpfunc_=" + str(cmpcode) + "&feedback_=" + str(feedcode);
		return urllib.urlopen(url).read();
	else:
		return "Error: inferPath([asn], [prefix], [LUF/SPF], [NO_FEEDBACK/WITH_FEEDBACK])";


if __name__ == '__main__':
	import argparse
	parser = argparse.ArgumentParser(description='AS Path inference')
	parser.add_argument('--asn', metavar='<AS_NUM>', help='AS Number', required=True)
	parser.add_argument('--prefix', metavar='<PREFIX>', help='PREFIX', required=True)
	parser.add_argument('--alg', metavar='<ALG>', choices=['LUF', 'SPF'], help='Algorithm', default='LUF')
	args = parser.parse_args(sys.argv[1:])
	print args
	print inferPath(args.asn, args.prefix, args.alg, "NO_FEEDBACK");
# Returned information:
#################################################################################################################
#                                               #
#   *** 128.119.0.0                             #   the prefix containing the destination address
#   6 1 4538-1239+7911+1249                     #   ordered according to LUF/SPF
#   1 1 4538*7660*22388*11537+10578+1249        #   format: frequency_index unsure_length path
#   37 2 4538*4134=7911+1249                    #   path format: * SIBLING_TO_SIBLING = PEER_TO_PEER
#   6 2 4538-9929-1239+7911+1249                #                - CUSTOMER_TO_PROVIDER + PROVIDER_TO_CUSTOMER
#   6 2 4538*9405-1239+7911+1249                #
#   5 2 4538-9264-11537+10578+1249              #
#   1 2 4538-9270*7660*22388*11537+10578+1249   #
#   >>> 1115921388.67                           #    the time when the base BGP tables are collected
#   ^^^ 11.7617368698                           #    time consumption
#                                               #
##################################################################################################################
