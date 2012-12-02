#!/usr/bin/env python

import sys;
import string;
import urllib;
import re;

def inferPath(src, prefix, algorithm='LUF', use_known=True):
    url = "http://127.0.0.1:61002/infer?%s" % \
            urllib.urlencode({'src_': src, 'prefix_': prefix,
                                'algorithm_': algorithm,
                                'use_known_': use_known})
    return urllib.urlopen(url).read()

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='AS Path inference')
    parser.add_argument('--src', metavar='<SRC>', help='Source AS Number', required=True)
    parser.add_argument('--prefix', metavar='<PREFIX>', help='PREFIX', required=True)
    parser.add_argument('--algorithm', metavar='<ALG>', choices=['LUF', 'SPF'], help='Algorithm', default='LUF')
    parser.add_argument('--use-known', action='store_true', help='Use known paths', default='LUF')
    args = parser.parse_args()
    print inferPath(args.src, args.prefix, args.algorithm, args.use_known)
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
