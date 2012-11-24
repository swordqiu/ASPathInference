#!/usr/bin/env python

import sys;
import string;
sys.path.append('../script/');
import aspathinfer;

srcas = int(sys.argv[1]);
dstip = sys.argv[2];

if len(sys.argv) < 3:
	print "Usage: test.py srcas dstip/dstprefix";
	sys.exit(-1);

print aspathinfer.inferPath(srcas, dstip, "LUF", "NO_FEEDBACK");
