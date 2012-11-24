#!/usr/bin/env python

import sys;
import string;
import bgplib;

R = 60;
L = 1;

g_asgraph = {};
g_transfreq = {};
g_prefixfreq = {};
g_maxfreq = 0;
g_asexit = {};

def addExit(src, exitas):
	global g_asexit;
        if not g_asexit.has_key(src):
		g_asexit[src] = {};
	g_asexit[src][exitas] = g_asexit[src].get(exitas, 0) + 1;


def isPopularPrefix(prefix):
	global g_prefixfreq, g_maxfreq;
	if g_prefixfreq.has_key(prefix) and g_prefixfreq[prefix] >= g_maxfreq*2/3:
		return True;
	else:
		return False;

def addEdge(as1, as2):
	if not g_asgraph.has_key(as1):
		g_asgraph[as1] = {};
	if not g_asgraph[as1].has_key(as2):
		g_asgraph[as1][as2] = [0, True];
	if not g_asgraph.has_key(as2):
		g_asgraph[as2] = {};
	if not g_asgraph[as2].has_key(as1):
		g_asgraph[as2][as1] = [0, True];

def addPrefix(prefix):
	global g_prefixfreq, g_maxfreq;
	g_prefixfreq[prefix] = g_prefixfreq.get(prefix, 0) + 1;
	if g_maxfreq < g_prefixfreq[prefix]:
		g_maxfreq = g_prefixfreq[prefix];

def constructGraph(prefix, nexthop, path):
	global g_transfreq;
	if not bgplib.validRoute(prefix, path):
		return;
	addPrefix(prefix);
	if len(path) >= 2:
		for i in range(len(path) - 1):
			addEdge(path[i], path[i+1]);
			if i != 0:
				g_transfreq[path[i]] = g_transfreq.get(path[i], 0) + 1;
		addExit(int(path[-1]), int(path[-2]));	

def getDegree(asn):
	global g_asgraph;
	if g_asgraph.has_key(asn):
		return len(g_asgraph[asn]);
	else:
		return 0;

def getTransfreq(asn):
	global g_transfreq;
	if g_transfreq.has_key(asn):
		return g_transfreq[asn];
	else:
		return 0;

def getMetric(asn):
	return getDegree(asn);
	#if getTransfreq(asn) == 0:
	#	return 0;
	#else:
	#	return getDegree(asn);

filternum = 0;
totalnum = 0;
def peakresearch(prefix, path):
	global filternum, totalnum;
	totalnum = totalnum + 1;
	if not isPopularPrefix(prefix):
		filternum = filternum + 1;
		return;
	isUp = True;
	peak = [];
	for i in range(len(path)-1):
		if getDegree(path[i]) < getDegree(path[i+1]):
			isUp = True;
		else:
			if isUp:
				peak.append(path[i]);
			isUp = False;
	if len(peak) > 1:
		tmpstr = "";
		for i in range(len(path)):
			tmpstr = tmpstr + str(path[i]) + "(" + str(getDegree(path[i])) + ") ";
		print tmpstr;

def findMaxDegree(path):
        maxidx = -1;
        maxdeg = 0;
        for i in range(len(path)):
                if maxdeg < getMetric(path[i]):
                        maxdeg = getMetric(path[i]);
                        maxidx = i;
        return maxidx;

def statisTransitNumber(prefix, nexthop, path):
	global g_asgraph;
	if not bgplib.validRoute(prefix, path):
		return;
	if not isPopularPrefix(prefix):
		return;
	#print path;
	if len(path) >= 2:
		topidx = findMaxDegree(path);
		if topidx > 0:
			for i in range(topidx):
				g_asgraph[path[i]][path[i+1]][0] = g_asgraph[path[i]][path[i+1]][0] + 1;
				#print path[i], path[i+1];
				#if path[i] == 32756 and path[i+1] == 26677:
				#	print path[i], path[i+1], g_asgraph[path[i]][path[i+1]];

		if topidx + 1 < len(path):
			for i in range(topidx+1, len(path)):
				g_asgraph[path[i]][path[i-1]][0] = g_asgraph[path[i]][path[i-1]][0] + 1;
				#if path[i] == 32756 and path[i-1] == 26677:
				#	print path[i], path[i-1], g_asgraph[path[i]][path[i-1]];

def isSibling(as1, as2):
	tr12 = g_asgraph[as1][as2][0];
	tr21 = g_asgraph[as2][as1][0];
	if (tr12 > L and tr21 > L) or (tr12 > 0 and tr12 <= L and tr21 > 0 and tr21 <= L):
		return True;
	else:
		return False;

def identifyNotPeerLink(prefix, nexthop, path):
	global g_asgraph;
	if not bgplib.validRoute(prefix, path):
		return;
	if not isPopularPrefix(prefix):
		return;
	if len(path) >= 2:
		topidx = findMaxDegree(path);
		if topidx >= 2:
			for i in range(topidx-1):
				g_asgraph[path[i]][path[i+1]][1] = False;
		if topidx + 2 < len(path):
			for i in range(topidx+1, len(path) - 1):
				g_asgraph[path[i]][path[i+1]][1] = False;
		if topidx > 0 and topidx < len(path) - 1 and not isSibling(path[topidx-1], path[topidx]) and not isSibling(path[topidx], path[topidx+1]):
			if getDegree(path[topidx-1]) > getDegree(path[topidx+1]):
				g_asgraph[path[topidx]][path[topidx+1]][1] = False;
			else:
				g_asgraph[path[topidx-1]][path[topidx]][1] = False;


def printLinkPref(filename):
	with open(filename, 'w') as f:
		for i in g_asexit.keys():
			for j in g_asexit[i].keys():
				f.write(str(i) + ":" + str(j) + ":" + str(g_asexit[i][j]) + "\n");


def getRelationship(as1, as2):
        tr12 = g_asgraph[as1][as2][0];
        p12  = g_asgraph[as1][as2][1];
        tr21 = g_asgraph[as2][as1][0];
        p21  = g_asgraph[as2][as1][1];
        d1   = getDegree(as1);
        d2   = getDegree(as2);
        if p12 and p21 and d1*1.0/d2 < R and d2*1.0/d1 > 1.0/R and tr12+tr21 > 0:
                return bgplib.PEER_TO_PEER;
        elif (tr21 > L and tr12 > 0 and tr12 <= L) or (tr21 > 0 and tr12 == 0):
                return bgplib.PROVIDER_TO_CUSTOMER;
        elif (tr12 > L and tr21 > 0 and tr21 <= L) or (tr12 > 0 and tr21 == 0):
                return bgplib.CUSTOMER_TO_PROVIDER;
        #elif (tr12 > L and tr21 > L) or (tr12 > 0 and tr12 <= L and tr21 > 0 and tr21 <= L):
        #        return bgplib.SIBLING_TO_SIBLING;
	else:
		return bgplib.SIBLING_TO_SIBLING;


def printRelationship(filename):
	try:
		p2c = 0;
		p2p = 0;
		s2s = 0;
		with open(filename, 'w') as f:
			for asn in g_asgraph.keys():
				nblist = g_asgraph[asn];
				provider = [];
				customer = [];
				peer = [];
				sibling = [];
				for asn2 in nblist.keys():
					rel = getRelationship(asn, asn2);
					if rel == bgplib.CUSTOMER_TO_PROVIDER:
						provider.append(asn2);
					elif rel == bgplib.PROVIDER_TO_CUSTOMER:
						customer.append(asn2);
					elif rel == bgplib.PEER_TO_PEER:
						peer.append(asn2);
					else:
						sibling.append(asn2);
				f.write("AS" + str(asn) + ":" + "\n");
				f.write("Providers:#" + str(len(provider)) + "::" + bgplib.array2str(provider, ":") + "\n");
				f.write("Customers:#" + str(len(customer)) + "::" + bgplib.array2str(customer, ":") + "\n");
				f.write("Peers:#" + str(len(peer)) + "::" + bgplib.array2str(peer, ":") + "\n");
				f.write("Siblings:#" + str(len(sibling)) + "::" + bgplib.array2str(sibling, ":") + "\n");
				p2c = p2c + len(provider) + len(customer);
				p2p = p2p + len(peer);
				s2s = s2s + len(sibling);
		print "P2C: " + str(p2c/2);
		print "P2P: " + str(p2p/2);
		print "S2S: " + str(s2s/2);
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def printRelationship2(filename):
        try:
                f = open(filename, 'w');
                for asn in g_asgraph.keys():
                        nblist = g_asgraph[asn];
                        for asn2 in nblist.keys():
                                if asn < asn2:
                                        rel = getRelationship(asn, asn2);
                                        #if g_asgraph[asn][asn2][0]*g_asgraph[asn2][asn][0] != 0:
                                        #        mark = "*";
                                        #        if g_asgraph[asn][asn2][3]*g_asgraph[asn2][asn][3] != 0:
                                        #                mark = mark + "&";
                                        #else:
                                        #        mark = "";
                                        #f.write(str(asn) + " " + str(asn2) + " " + relationCode2Str(rel) + " " + str(g_asgraph[asn][asn2]) + " " + str(g_asgraph[asn2][asn]) + " " + str(getPeerStatistics(asn)) + " " + str(getPeerStatistics(asn2)) + " " + str(asaccess.get(asn, 0)) + " " + str(asaccess.get(asn2, 0)) + " " + mark + "\n");
                                        f.write(str(asn) + " " + str(asn2) + " " + str(rel) + " " + str(g_asgraph[asn][asn2]) + " " + str(g_asgraph[asn2][asn]) + "\n");
                f.close();
        except:
                print "Exception: ", sys.exc_info()[0];
                raise;


def readTableList(filename):
	try:
		f = open(filename, "r");
		line = f.readline();
		tlt = [];
		while len(line) > 0:
			if line[0] != '#':
				tlt.append(line[:-1].split(" "));
			line = f.readline();
		f.close();
		return tlt;
	except:
		print "Exception", sys.exc_info()[0];
		raise;



if sys.argv[1].find("tablelist") >= 0:
	tablelist = readTableList(sys.argv[1]);
else:
	tablelist = [[sys.argv[1], "TABLE"]];

for i in range(3):
	for table in tablelist:
		if table[1] == "TABLE":
			if i == 0:
				bgplib.readBGPTable(table[0], constructGraph);
			elif i == 1:
				bgplib.readBGPTable(table[0], statisTransitNumber);
			else:
				bgplib.readBGPTable(table[0], identifyNotPeerLink);
		elif table[1] == "DUMP":
			if i == 0:
				bgplib.readBGPDump(table[0], constructGraph);
			elif i == 1:
				bgplib.readBGPDump(table[0], statisTransitNumber);
			else:
				bgplib.readBGPDump(table[0], identifyNotPeerLink);
		else:
			print "unsupported format";
			sys.exit(-1);

printRelationship(sys.argv[2]);
printLinkPref(sys.argv[3]);
#printRelationship2(sys.argv[4]);
