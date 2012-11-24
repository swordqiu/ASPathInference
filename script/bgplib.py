#!/usr/bin/env python

import string;
import sys;
import time;
import os;
import re;

sys.path.append('/home/jqiu/pythonlib');

import bgp;


def getMask(len):
	return (2**len - 1)*2**(32-len);

def sgn(x):
	if x > 0:
		return 1;
	elif x == 0:
		return 0;
	else:
		return -1;

def minhash(hs):
	mn = 4294967296.0;
	for v in hs.values():
		if mn > v:
			mn = v;
	return v;

def normalizehash(hs):
	mn = minhash(hs);
	newhs = {};
	for k in hs.keys():
		newhs[k] = hs[k]/mn;
	return newhs;

def sumhash(hs):
	sm = 0;
	for v in hs.values():
		sm = sm + v;
	return sm;

class CPrefix:
	val = 0;
	prefixlen = 0;

	def parseString(self, prefstr): #"xxx.xxx.xxx.xxx/xx"
		if not re.compile("^\d+\.\d+\.\d+\.\d+").match(prefstr):
			self.val = 0;
			self.prefixlen = 0;
			return;
		tmp = prefstr.split(".");
		a = int(tmp[0]);
		b = int(tmp[1]);
		c = int(tmp[2]);
		if tmp[3].find("/") > 0:
			self.prefixlen = int(tmp[3][tmp[3].find("/")+1:]);
			tmp[3] = tmp[3][:tmp[3].find("/")];
		d = int(tmp[3]);
		self.val = a*16777216 + b*65536 + c*256 + d;
		if prefstr.find("/") < 0:
			if a == 0:
				self.prefixlen = 0;
			elif a < 128:
				self.prefixlen = 8;
			elif a < 192:
				self.prefixlen = 16;
			else:
				self.prefixlen = 24;
			while (self.val > (self.val & getMask(self.prefixlen))):
				self.prefixlen = self.prefixlen + 1;
		else:
			self.val = self.val & getMask(self.prefixlen);

	def __init__(self, str):
		if str != "":
			self.parseString(str);

	def setVal(self, addr, len):
		self.prefixlen = len;
		self.val = addr & getMask(self.prefixlen);

	def __str__(self):
		a = self.val/2**24;
		b = (self.val - a*2**24)/2**16;
		c = (self.val - a*2**24 - b*2**16)/2**8;
		d = self.val - a*2**24 - b*2**16 - c*2**8;
		if (a < 128 and self.prefixlen == 8) or (a>=128 and a < 192 and self.prefixlen == 16) or (a >= 192 and self.prefixlen == 24):
			surfix = "";
		else:
			surfix = "/" + str(self.prefixlen);
		return str(a) + "." + str(b) + "." + str(c) + "." + str(d) + surfix;

	def mask(self, len):
		return self.val & getMask(len);

	def __cmp__(self, pre2):
		if pre2 is None:
			return 1;
		if self.val != pre2.val:
			return sgn(self.val - pre2.val);
		else:
			return sgn(self.prefixlen - pre2.prefixlen);

	def __hash__(self):
		return str(self).__hash__();

	def getMaskedPrefix(self, prelen):
		newpre = CPrefix(str(self));
		newpre.val = newpre.val & getMask(prelen);
		newpre.prefixlen = prelen;
		return newpre;

	def match(self, prefix):
		if self.prefixlen <= prefix.prefixlen and self.val == prefix.mask(self.prefixlen):
			return True;
		else:
			return False;

	def mergeable(self, prefix):
		if self.prefixlen == prefix.prefixlen and self.val != prefix.val and self.mask(self.prefixlen - 1) == prefix.mask(self.prefixlen - 1):
			return True;
		else:
			return False;

	def setPrefixlen(self, plen):
		self.prefixlen = plen;
		self.val = self.mask(plen);

	def size(self):
		return 2**(32 - self.prefixlen);

	def toBitString(self):
		bitstr = "";
		for i in range(0, self.prefixlen):
			if (self.val & (2**(31 - i))) != 0:
				bitstr = bitstr + "1";
			else:
				bitstr = bitstr + "0";
		return bitstr;

	def fromBitString(self, bitstr):
		self.prefixlen = len(bitstr);
		self.val = 0;
		for i in range(0, self.prefixlen):
			if bitstr[i] == '1':
				self.val = self.val + (2**(31 - i));

nonpublic = [CPrefix("10.0.0.0/8"), CPrefix("127.0.0.0/8"), CPrefix("172.16.0.0/12"), CPrefix("192.168.0.0/16")];

def isPrivate(pref):
	for net in nonpublic:
		if net.match(pref):
			return True;
	return False;

# AS112 Project home page: http://www.as112.net/
def validRoute(prefixstr, path):
	if validPrefix(prefixstr) and validPath(path):
		return True;
	else:
		return False;

# RFC 1918, RFC 3330
# RFC-1918, http://www.iana.org/assignments/ipv4-address-space
bogonprefix = [5, 7, 10, 23, 27, 31, 36, 37, 39, 42, 127, 197, 223];
#bogonprefix.extend(range(0, 3));
#bogonprefix.extend(range(77, 80));
#bogonprefix.extend(range(92, 121));
#bogonprefix.extend(range(173, 188));
#bogonprefix.extend(range(224, 256));

def isPrivate2(a, b):
	if a == 10 or a == 127 or (a == 172 and b in range(16, 32)) or (a==192 and b==168) or (a==169 and b == 254) or (a==198 and (b==18 or b==19)):
		return True;
	else:
		return False;

def isBogon(a):
	global bogonprefix;
	if a in bogonprefix or (a>=0 and a<3) or (a>=77 and a<80) or (a>=92 and a<121) or (a>=173 and a<188) or (a>=224 and a<256):
		return True;
	else:
		return False;

def validPrefix(pref):
	slash = pref.find("/");
	if slash > 0:
		prefixlen = int(pref[(slash+1):]);
	else:
		prefixlen = 24;
	dota = pref.find(".");
	if dota > 0:
		a = int(pref[0:dota]);
		dotb = pref.find(".", dota + 1);
		if dotb > 0:
			b = int(pref[(dota+1):dotb]);
		elif slash > 0:
			b = int(pref[(dota+1):slash]);
	elif slash > 0:
		a = int(pref[0:slash]);
		b = 0;
	if prefixlen < 30 and prefixlen >= 8 and not isPrivate2(a,b) and not isBogon(a) and pref.find("192.0.2.") != 0 and pref.find("192.168.2/") != 0:
		return True;
	else:
		return False;

def validPath(aspath):
	if len(aspath) > 0 and aspath[-1] != 112:
		return True;
	else:
		return False;

def emptystr(num):
	""" produce empty set of length num
	"""
	tmpstr = "";
	for i in range(num):
		tmpstr = tmpstr + ' ';
	return tmpstr;

class CPath:
	path = [];

	def __init__(self, str):
		self.path = [];
		self.parseString(str);

	def parseString(self, str):
		tmp = str.split(" ");
		for i in range(len(tmp)):
			self.path.append(int(tmp[i]));

	def __cmp__(self, p):
		if len(self.path) != len(p.path):
			return len(self.path) - len(p.path);
		else:
			for i in range(len(self.path)):
				if self.path[i] != p.path[i]:
					return self.path[i] - p.path[i];
			return 0;

	def __str__(self):
		tmpstr = str(self.path[0]);
		for asn in self.path[1:]:
			tmpstr = tmpstr + " " + str(asn);
		return tmpstr;

	def __hash__(self):
		return str(self).__hash__();

class COrderedList:
	data = [];

	def __init__(self):
		self.data = [];

	def add(self, o):
		start = 0;
		end = len(self.data)-1;
		while start <= end:
			j = (start + end)/2;
			if "__cmp__" in dir(o):
				result = o.__cmp__(self.data[j]);
				if result == 0:
					return;
				elif result > 0:
					start = j + 1;
				else:
					end = j - 1;
			else:
				if o == self.data[j]:
					return;
				elif o > self.data[j]:
					start = j + 1;
				else:
					end = j - 1;
		#print o;
		self.data.insert(start, o);

	def add2(self, o):
		self.data.append(o);

	def __getitem__(self, idx):
		return self.data[idx];

	def __len__(self):
		return len(self.data);

	def pop(self, idx):
		return self.data.pop(idx);

	def __cmp__(self, o):
		i = 0;
		while i < len(self.data) and i < len(o.data):
			if self.data[i] != o.data[i]:
				return self.data[i].__cmp__(o.data[i]);
			i = i + 1;
		return len(self.data) - len(o.data);

	def difference(self, olist):
		diff1 = 0;
		diff2 = 0;
		same = 0;
		i = 0;
		j = 0;
		while i < len(self.data) or j < len(olist):
			if i < len(self.data):
				oi = self.data[i];
			else:
				oi = None;
			if j < len(olist):
				oj = olist[j];
			else:
				oj = None;
			if oi is None and oj is not None:
				result = 1;
			elif oi is not None and oj is None:
				result = -1;
			elif "__cmp__" in dir(oi):
				result = oi.__cmp__(oj);
			else:
				if oi > oj:
					result = 1;
				elif oi == oj:
					result = 0;
				else:
					result = -1;
			if result == 0:
				i = i + 1;
				j = j + 1;
				same = same + 1;
			elif result > 0:
				j = j + 1;
				diff2 = diff2 + 1;
			else:
				i = i + 1;
				diff1 = diff1 + 1;
		return [diff1, same, diff2];

class CASLink:
        as1 = None;
        as2 = None;

        def __init__(self, a1, a2):
                if a1 < a2:
                        self.as1 = a1;
                        self.as2 = a2;
                elif a1 > a2:
                        self.as1 = a2;
                        self.as2 = a1;
                else:
                        print "Error: as1 and as2 are the same!";
                        sys.exit(-1);

        def __cmp__(self, link):
                if link is None:
                        return 1;
                else:
			if self.as1 != link.as1:
				return self.as1 - link.as1;
                        else:
				return self.as2 - link.as2;

        def __str__(self):
                return "AS" + str(self.as1) + "-AS" + str(self.as2);




NO_LINK = -1;

NO_BGP_SESSION = 0;

CUSTOMER_TO_PROVIDER = 1;
PEER_TO_PEER = 2;
PROVIDER_TO_CUSTOMER = 3;
SIBLING_TO_SIBLING = 4;

def reverseRelation(rel):
	if rel == CUSTOMER_TO_PROVIDER:
		return PROVIDER_TO_CUSTOMER;
	elif rel == PROVIDER_TO_CUSTOMER:
		return CUSTOMER_TO_PROVIDER;
	else:
		return rel;

def relationPriority(rel):
	if rel == PROVIDER_TO_CUSTOMER:
		return 4;
	elif rel == CUSTOMER_TO_PROVIDER:
		return 2;
	elif rel == PEER_TO_PEER:
		return 3;
	elif rel == SIBLING_TO_SIBLING:
		return 1;
	else:
		print "Unknown relationship! ", rel;
		sys.exit(-1);

def relationCode2Str(s):
	if s == PROVIDER_TO_CUSTOMER:
		return "+";
	elif s == CUSTOMER_TO_PROVIDER:
		return "-";
	elif s == PEER_TO_PEER:
		return "=";
	elif s == SIBLING_TO_SIBLING:
		return "*";
	else:
		return "?";

def appendable(rel1, rel2): # rel2 append to rel1
	if rel1 == SIBLING_TO_SIBLING or rel1 == CUSTOMER_TO_PROVIDER or rel2 == SIBLING_TO_SIBLING or ((rel1 == PEER_TO_PEER or rel1 == PROVIDER_TO_CUSTOMER) and rel2 == PROVIDER_TO_CUSTOMER):
		return True;
	else:
		return False;

def getPeerList(line):
	line = line[:-1];
	#print line;
	start = line.find("::");
	retarr = [];
	if start > 0:
		line = line[start + 2:];
		if len(line) > 0:
			retarr = line.split(":");
	#print len(retarr);
	return retarr;

def addNeighborList(nbhash, peerlist, type):
	for i in range(len(peerlist)):
		nbhash[int(peerlist[i])] = type;


class CASGraph:
	asgraph = {};

	def __init__(self, filename):
		self.asgraph = {};
		self.readASRelation(filename);

	def readASRelation(self, filename):
		try:
			f = open(filename, 'r');
			line = f.readline();
			while len(line) > 0:
				asn = int(line[2:-2]);
				nblist = {};
				providers = getPeerList(f.readline());
				addNeighborList(nblist, providers, CUSTOMER_TO_PROVIDER);
				customers = getPeerList(f.readline());
				addNeighborList(nblist, customers, PROVIDER_TO_CUSTOMER);
				peers = getPeerList(f.readline());
				addNeighborList(nblist, peers, PEER_TO_PEER);
				siblings = getPeerList(f.readline());
				addNeighborList(nblist, siblings, SIBLING_TO_SIBLING);
				self.asgraph[asn] = nblist;
				#print str(asn) + " " + str(len(providers) + len(customers) + len(peers) + len(siblings));
				line = f.readline();
		except IOError, (errno, strerror):
			print "I/O error(%s): %s" % (errno, strerror)
		except:
			print "Unexpected error:", sys.exc_info()[0]
			raise;

	def removeStubNodes(self):
		isdel = True;
		#print len(self.asgraph);
		while isdel:
			isdel = False;
			asnlist = self.asgraph.keys();
			for asn in asnlist:
				if len(self.asgraph[asn]) == 1:
					peer = self.asgraph[asn].keys()[0];
					del self.asgraph[asn][peer];
					del self.asgraph[peer][asn];
				if len(self.asgraph[asn]) == 0:
					del self.asgraph[asn];
					isdel = True;
		#print len(self.asgraph);

	def removeBridges(self):
		isdel = True;
		print len(self.asgraph);
		while isdel:
			isdel = False;
			asnlist = self.asgraph.keys();
			for asn in asnlist:
				if len(self.asgraph[asn]) == 2:
					nbs = self.asgraph[asn].keys();
					rels = [self.asgraph[asn][nbs[0]], self.asgraph[asn][nbs[1]]];
					remove = False;
					if rels[0] == reverseRelation(rels[1]):
						remove = True;
						rel0 = reverseRelation(rels[0]);
						rel1 = reverseRelation(rels[1]);
					elif rels[0] == SIBLING_TO_SIBLING or rels[0] == PEER_TO_PEER:
						remove = True;
						rel0 = rels[1];
						rel1 = reverseRelation(rels[1]);
					elif rels[1] == SIBLING_TO_SIBLING or rels[1] == PEER_TO_PEER:
						remove = True;
						rel0 = reverseRelation(rels[0]);
						rel1 = rels[0];
					if remove:
						del self.asgraph[nbs[0]][asn];
						del self.asgraph[nbs[1]][asn];
						self.asgraph[nbs[0]][nbs[1]] = rel0;
						self.asgraph[nbs[1]][nbs[0]] = rel1;
						#print "remove", str(nbs[0]), relationCode2Str(reverseRelation(self.asgraph[asn][nbs[0]])), str(asn), relationCode2Str(self.asgraph[asn][nbs[1]]), str(nbs[1]);
						del self.asgraph[asn][nbs[0]];
						del self.asgraph[asn][nbs[1]];
						del self.asgraph[asn];
						isdel = True;
		print len(self.asgraph);

	def __getitem__(self, asn):
		if self.asgraph.has_key(asn):
			return self.asgraph[asn];
		else:
			return None;

	def getRelation(self, asn1, asn2):
		if self.asgraph.has_key(asn1) and self.asgraph[asn1].has_key(asn2):
			return self.asgraph[asn1][asn2];
		else:
			return SIBLING_TO_SIBLING;

	def getRelationSymbol(self, asn1, asn2):
		return relationCode2Str(self.getRelation(asn1, asn2));

	def __len__(self):
		return len(self.asgraph);

	def getASList(self):
		return self.asgraph.keys();

	def hasLink(self, asn1, asn2):
		if self.asgraph.has_key(asn1) and self.asgraph[asn1].has_key(asn2):
			return True;
		else:
			return False;

	def getNeighbor(self, asn, rel):
		nblist = [];
		for nb in self.asgraph[asn].keys():
			if self.asgraph[asn][nb] == rel:
				nblist.append(nb);
		return nblist;

	def getDistance(self, asn1, asn2):
		queue = [asn1];
		processed = {};
		processed[asn1] = [0, SIBLING_TO_SIBLING];
		while len(queue) > 0:
			asn = queue.pop(0);
			for nb in self.asgraph[asn].keys():
				rel = self.asgraph[asn][nb];
				if appendable(processed[asn][1], rel):
					if nb == asn2:
						return processed[asn][0] + 1;
					if not processed.has_key(nb):
						if rel == SIBLING_TO_SIBLING:
							rel = processed[asn][1];
						processed[nb] = [processed[asn][0] + 1, rel];
						queue.append(nb);
		return -1;


class CExitPreference:
	asexit = {};

	def __init__(self, filename):
		self.asexit = {};
		self.readPreference(filename);

	def addExit(self, rec):
		if not self.asexit.has_key(int(rec[0])):
			self.asexit[int(rec[0])] = {}
		self.asexit[int(rec[0])][int(rec[1])] = int(rec[2]);

	def readPreference(self, filename):
		try:
			f = open(filename, 'r');
			line = f.readline();
			while len(line) > 0:
				record = line[:-1].split(":");
				self.addExit(record);
				line = f.readline();
		except IOError, (errno, strerror):
			print "I/O error(%s): %s" % (errno, strerror)
		except:
			print "Unexpected error:", sys.exc_info()[0]
			raise

	def getExitPref(self, asn1, asn2):
		if self.asexit.has_key(asn1) and self.asexit[asn1].has_key(asn2):
			return self.asexit[asn1][asn2];
		else:
			return 0;

	def __getitem__(self, asn):
		if self.asexit.has_key(asn):
			return self.asexit[asn];
		else:
			return {};

# IANA: http://www.iana.org/assignments/as-numbers
def is_valid_asn(asn):
	if asn == 0 or \
			asn == 23456 or \
			(asn >= 61440 and asn <= 65535) or \
			(asn >= 65536 and asn <= 131071) or \
			(asn >= 133120 and asn <= 196607) or \
			(asn >= 199680 and asn <= 262143) or \
			(asn >= 263168 and asn <= 327679) or \
			(asn >= 328704 and asn <= 393215) or \
			asn > 394240:
		return False
	else:
		return True

def readpathstr(line, allow_dup=False):
	tmpstr = "";
	path = [];
	for i in range(len(line) + 1):
		if i < len(line) and "0123456789.".find(line[i]) >= 0:
			tmpstr = tmpstr + line[i];
		else:
			if len(tmpstr) > 0:
				if tmpstr.find('.') > 0:
					print line, tmpstr
					tmpasns = tmpstr.split('.')
					print tmpasns
					tmpasn = (int(tmpasns[0]) << 16) + int(tmpasns[1])
					print tmpstr, tmpasn
				else:
					tmpasn = int(tmpstr)
				if is_valid_asn(tmpasn) and (allow_dup or len(path) == 0 or path[-1] != tmpasn):
					path.append(int(tmpstr));
			tmpstr = "";
	return path;

def readpathstr2(line):
	return readpathstr(line, allow_dup=True)

def splitstr2num(line):
	tmpstr = "";
	path = [];
	for i in range(len(line) + 1):
		if i < len(line) and "0123456789.".find(line[i]) >= 0:
			tmpstr = tmpstr + line[i];
		else:
			if len(tmpstr) > 0:
				path.append(tmpstr);
			tmpstr = "";
	return path;

def str2intarray(line, sep):
	tmppath = line.split(sep);
	path = [];
	for i in range(len(tmppath)):
		path.append(int(tmppath[i]));
	return path;

def array2str(path, sep):
	if len(path) == 0:
		return "";
	else:
		tmpstr = str(path[0]);
		for i in range(1, len(path)):
			tmpstr = tmpstr + sep + str(path[i]);
		return tmpstr;

class CPrefixTree:
	prefixhash = {};

	def __init__(self, filename):
		self.prefixhash = {};
		self.readPrefixAtom(filename);

	def readPrefixAtom(self, filename):
		try:
			f = open(filename, 'r');
			line = f.readline();
			while len(line) > 0:
				tmps = line[:-1].split(":");
				masterprefix = CPrefix(tmps[0]);
				self.prefixhash[masterprefix] = masterprefix;
				for prefstrs in tmps[1:]:
					self.prefixhash[CPrefix(prefstrs)] = masterprefix;
				line = f.readline();
		except:
			print "Exception: ", sys.exc_info()[0];
			raise;

	def getMatchPrefix(self, pref):
		len = pref.prefixlen;
		while len >= 8:
			tmppref = pref.getMaskedPrefix(len);
			if self.prefixhash.has_key(tmppref):
				return self.prefixhash[tmppref];
			len = len - 1;
		return None;

class CPrefixTree2:
	prefixhash = {};
	maxfreq = 0;

	def __init__(self, filename):
		self.prefixhash = {};
		self.maxfreq = 0;
		self.readPrefixAtom(filename);

	def readPrefixAtom(self, filename):
		try:
			f = open(filename, 'r');
			line = f.readline();
			while len(line) > 0:
				prefix = line[:-1];
				self.prefixhash[prefix] = self.prefixhash.get(prefix, 0) + 1;
				if self.maxfreq < self.prefixhash[prefix]:
					self.maxfreq = self.prefixhash[prefix];
					#print prefix + " " + str(self.maxfreq);
				line = f.readline();
			#print self.maxfreq;
			#print len(self.prefixhash);
		except:
			print "Exception: ", sys.exc_info()[0];
			raise;

	def rehash(self):
		newhash = {};
		for prefstr in self.prefixhash.keys():
			pref = CPrefix(prefstr);
			newhash[pref] = newhash.get(pref, 0) + self.prefixhash[prefstr];
		self.prefixhash = {};
		self.prefixhash = newhash;

	def getMatchPrefix(self, pref):
		len = pref.prefixlen;
		while len >= 8:
			tmppref = pref.getMaskedPrefix(len);
			#print tmppref;
			if self.prefixhash.has_key(tmppref) and self.prefixhash[tmppref] >= 2:
				return tmppref;
			len = len - 1;
		return None;

class CPrefixAtom:
	prefixlist = [];

	def __init__(self, filename):
		self.prefixlist = [];
		self.readPrefixAtom(filename);

	def readPrefixAtom(self, filename):
		try:
			f = open(filename, 'r');
			line = f.readline();
			while len(line) > 0:
				tmps = line[:-1].split(":");
				self.prefixlist.append(CPrefix(tmps[0]));
				line = f.readline();
			self.prefixlist.sort();
		except:
			print "Exception: ", sys.exc_info()[0];
			raise;

	def __len__(self):
		return len(self.prefixlist);

	def __getitem__(self, idx):
		return self.prefixlist[idx];

class CTicker:
	tick = 0;

	def __init__(self):
		self.init();

	def init(self):
		self.tick = time.time();

	def report(self):
		return time.time() - self.tick;

def isPathStatus(line):
	if len(line) >= 3 and (line[0] == '*' or line[0] == ' ') and "sdh>iS ".find(line[1]) >= 0 and "sdh>iS ".find(line[2]) >= 0 and line[0:3] != "   ":
		return True;
	else:
		return False;

def incompleteLine(line):
	if len(line) < 61:
		return True;
	else:
		return False;

def getPathStr(line):
	return line[61:];

def countLine(filename):
	try:
		num = 0;
		f = open(filename, 'r');
		line = f.readline();
		while len(line) > 0:
			num = num + 1;
			line = f.readline();
		return num;
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def readDir(dirname, pattern, proc):
        try:
                files = os.listdir(dirname);
                for filename in files:
                        if re.compile(pattern).match(filename):
				proc(dirname + "/" + filename);
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def readArray(filename):
	try:
		arr = [];
		f = open(filename, 'r');
		line = f.readline();
		while len(line) > 0:
			arr.append(line[:-1]);
			line = f.readline();
		return arr;
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def getSurePath(dbpath, asn, prefix):
	str = bgp.getSurePath(dbpath, asn, prefix);
	paths = str.split("\n");
	pathlist = [];
	for p in paths:
		if len(p) > 0:
			pathlist.append(str2intarray(p, " "));
	return pathlist;

def readBGPTable(filename, handler):
	try:
		f = open(filename, 'r');
		line = f.readline();
		while len(line) > 0:
			if isPathStatus(line):
				if line[3] != ' ':
					prefix = line[3:];
					i = 0;
					while i < len(prefix) and "1234567890/.".find(prefix[i]) >= 0:
						i = i + 1;
					if i > 18:
						slash_idx = prefix.find('/')
						if slash_idx > 0:
							i = slash_idx+3
							print slash_idx, i, prefix[:i]
					prefix = prefix[:i];
				if incompleteLine(line):
					line = f.readline();
				nexthop = line[20:line.find(" ", 20)];
				#print nexthop;
				path = readpathstr(getPathStr(line[:-1]));
				# if valideRoute(prefix, path):
				handler(prefix, nexthop, path);
			line = f.readline();
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def readBGPDump(filename, handler):
        try:
                f = open(filename, 'r');
                line = f.readline();
                while len(line) > 0:
                        if line.find("PREFIX: ") == 0:
                                prefix = line[8:-1];
                        if line.find("FROM: ") == 0:
                                tmppath = string.split(line[6:-1], " ");
                                nexthop = tmppath[0];
                        if line.find("ASPATH: ") == 0:
                                path = readpathstr(line[8:-1]);
				#if valideRoute(prefix, path):
				handler(prefix, nexthop, path);
                        line = f.readline();
        except IOError, (errno, strerror):
                print "I/O error(%s): %s" % (errno, strerror)
        except:
                print "Unexpected error:", sys.exc_info()[0]
                raise

def comparePathIgnoreDup(path1, path2):
	i = 0;
	j = 0;
	while i < len(path1) and j < len(path2) and path1[i] == path2[j]:
		now = i;
		while i < len(path1) and path1[i] == path1[now]:
			i = i + 1;
		now = j;
		while j < len(path2) and path2[j] == path2[now]:
			j = j + 1;
	if i >= len(path1) and j >= len(path2):
		return 0;
	elif i >= len(path1):
		return -1;
	elif j >= len(path2):
		return 1;
	else:
		return path1[i] - path2[j];

def removeDupPath(path):
	newpath = [];
	for i in range(len(path)):
		if len(newpath) <= 0 or newpath[-1] != path[i]:
			newpath.append(path[i]);
	return newpath;

class COwnership:
        prefix = None;
        asn = None;

        def __init__(self, pref, asn):
                self.prefix = pref;
                self.asn = asn;

        def __cmp__(self, o):
                if o is None:
                        return 1;
                else:
                        re1 = self.prefix.__cmp__(o.prefix);
                        if re1 != 0:
                                return re1;
                        else:
                                return 0; #self.asn - o.asn;

        def __str__(self):
                return str(self.prefix) + " " + str(self.asn);

        def contain(self, o):
                if self.asn == o.asn and self.prefix.match(o.prefix):
                        return True;
                else:
                        return False;

        def mergeable(self, o):
                if self.asn == o.asn and self.prefix.mergeable(o.prefix):
                        return True;
                else:
                        return False;

        def merge(self):
                self.prefix.setPrefixlen(self.prefix.prefixlen - 1);


def zeros(num):
	i = 0;
	tmparr = [];
	while i < num:
		tmparr.append(0);
		i = i + 1;
	return tmparr;

def printStr(filename, line):
	try:
		f = open(filename, "a");
		f.write(line + "\n");
		f.close();
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;


def reducePrefixList(olist):
	i = 1;
	while i < len(olist):
		if i > 0 and olist[i-1].match(olist[i]):
			olist.pop(i);
		elif i > 0 and olist[i-1].mergeable(olist[i]):
			olist[i-1].setPrefixlen(olist[i-1].prefixlen - 1);
			olist.pop(i);
			i = i - 1;
		else:
			i = i + 1;
	return olist;


def gmmktime(tmstr, fmt):
	return time.mktime(time.strptime(tmstr, fmt)) - time.mktime(time.strptime("1970-01-01 00:00:00", "%Y-%m-%d %H:%M:%S"));

def gmmktime2(tmstr):
	return time.mktime(time.strptime(tmstr, "%Y-%m-%d %H:%M:%S")) - time.mktime(time.strptime("1970-01-01 00:00:00", "%Y-%m-%d %H:%M:%S"));
