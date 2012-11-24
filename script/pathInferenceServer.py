#!/usr/bin/env python

import sys;
import string;
import os;
import math;
import bgplib;
import httpserver;
import time;

pathtable = {};
border = [];
inborder = {};
fixed = {};

def getPathRelationship(path):
	global asgraph
	i = 0;
	while i < len(path) - 1 and asgraph.getRelation(path[i], path[i+1]) == bgplib.SIBLING_TO_SIBLING:
		i = i + 1;
	if i == len(path) - 1:
		return bgplib.SIBLING_TO_SIBLING;
	else:
		return asgraph[path[i]][path[i+1]];

def updatePath(pathset, path):
	if len(pathset) == 0:
		bestpath = [];
	else:
		bestpath = pathset[0];
	replace = False;
	for i in range(len(pathset)):
		if pathset[i][3] == path[3]:
			if pathset[i][2:-1] != path[2:-1]:
				pathset[i] = path;
				replace = True;
				break;
			else:
				return False;
	if not replace:
		pathset.append(path);
	pathset.sort(cmppath);
	if bestpath == pathset[0]:
		return False;
	else:
		return True;

def initPath(pathset, path):
	for p in pathset:
		if p[2:-1] == path:
			p[0] = p[0] + 1;
			return;
	tmp = [1, 0];
	tmp.extend(path);
	tmp.append(getPathRelationship(path));
	pathset.append(tmp);
	return

def getInitTable(prefix):
	global dbpath, isuseknown, pathtable, border, inborder, fixed, known_aslist;
	for viewas in known_aslist:
		pathlist = bgplib.getSurePath(dbpath, viewas, prefix);
		for path in pathlist:
			print path;
			for i in range(1, len(path)):
				subpath = [];
				subpath.extend(path[i:]);
				if isuseknown == "false" and len(subpath) > 1:
					continue;
				if not pathtable.has_key(subpath[0]):
					pathtable[subpath[0]] = [];
					border.append(subpath[0]);
					inborder[subpath[0]] = 1;
					fixed[subpath[0]] = 1;
				initPath(pathtable[subpath[0]], subpath);
	for asn in border:
		pathtable[asn].sort(cmppathInit);
		print pathtable[asn];

def cmppathInit(path1, path2):
	if path1[0] != path2[0]:
		return - path1[0] + path2[0];
	if len(path1) != len(path2):
		return len(path1) - len(path2);
	return path1[3] - path2[3];

def cmppath(path1, path2):
        global g_cmpfunc;
        if g_cmpfunc == 1:
                return cmppathfunc1(path1, path2);
        elif g_cmpfunc == 2:
                return cmppathfunc2(path1, path2);
	elif g_cmpfunc == 3:
		return cmppathfunc3(path1, path2);
	elif g_cmpfunc == 4:
		return cmppathfunc4(path1, path2);
        else:
                print "Unknown compare path function!";
                sys.exit(-1);

def cmppathfunc2(path1, path2):
	global firsthopHash, g_current_prefix, g_feedback;
	global aspref
	if path1[2:-1] == path2[2:-1]:
		return 0;
	if path1[1]*path2[1] == 0 and path1[1] != path2[1]:
		return path1[1] - path2[1];
	if path1[1] == 0 and path2[1] == 0 and path1[0] != path2[0]:
		return - path1[0] + path2[0];
	# normal compare begin
        if g_feedback and firsthopHash.has_key(g_current_prefix) and firsthopHash[g_current_prefix].has_key(path1[2]):
                if len(path1) >= 4 and path1[3] == firsthopHash[g_current_prefix][path1[2]]:
                        return -1;
                if len(path2) >= 4 and path2[3] == firsthopHash[g_current_prefix][path2[2]]:
                        return 1;
	if len(path1) != len(path2):
		return len(path1) - len(path2);
	if aspref.getExitPref(path1[2], path1[3]) != aspref.getExitPref(path2[2], path2[3]):
		return - aspref.getExitPref(path1[2], path1[3]) + aspref.getExitPref(path2[2], path2[3]);
	if path1[1] != path2[1]:
		return path1[1] - path2[1];
	if path1[0] != path2[0]:
		return - path1[0] + path2[0];
	return path1[3] - path2[3];

def cmppathfunc4(path1, path2):
	global asgraph
	if path1[2:-1] == path2[2:-1]:
		return 0;
	if path1[1]*path2[1] == 0 and path1[1] != path2[1]:
		return path1[1] - path2[1];
	if path1[1] == 0 and path2[1] == 0 and path1[0] != path2[0]:
		return - path1[0] + path2[0];
	# normal compare begin
	if len(path1) != len(path2):
		return len(path1) - len(path2);
	if bgplib.relationPriority(asgraph.getRelation(path1[2], path1[3])) != bgplib.relationPriority(asgraph.getRelation(path2[2], path2[3])):
		return bgplib.relationPriority(asgraph.getRelation(path1[2], path1[3])) - bgplib.relationPriority(asgraph.getRelation(path2[2], path2[3]));
	if path1[1] != path2[1]:
		return path1[1] - path2[1];
	if path1[0] != path2[0]:
		return - path1[0] + path2[0];
	return path1[3] - path2[3];

def cmppathfunc1(path1, path2):
	global firsthopHash, g_current_prefix, g_feedback;
	global aspref
	if path1[2:-1] == path2[2:-1]:
		return 0;
        if g_feedback and firsthopHash.has_key(g_current_prefix) and firsthopHash[g_current_prefix].has_key(path1[2]):
                if len(path1) >= 4 and path1[3] == firsthopHash[g_current_prefix][path1[2]]:
                        return -1;
                if len(path2) >= 4 and path2[3] == firsthopHash[g_current_prefix][path2[2]]:
                        return 1;
	if path1[1] != path2[1]:
		return path1[1] - path2[1];
	if path1[0] != path2[0]:
		return - path1[0] + path2[0];
	if len(path1) != len(path2):
		return len(path1) - len(path2);
	if aspref.getExitPref(path1[2], path1[3]) != aspref.getExitPref(path2[2], path2[3]):
		return - aspref.getExitPref(path1[2], path1[3]) + aspref.getExitPref(path2[2], path2[3]);
	return path1[3] - path2[3];

def cmppathfunc3(path1, path2):
	global asgraph
	if path1[1] != path2[1]:
		return path1[1] - path2[1];
	if path1[0] != path2[0]:
		return - path1[0] + path2[0];
	if len(path1) != len(path2):
		return len(path1) - len(path2);
	if bgplib.relationPriority(asgraph.getRelation(path1[2], path1[3])) != bgplib.relationPriority(asgraph.getRelation(path2[2], path2[3])):
		return bgplib.relationPriority(asgraph.getRelation(path1[2], path1[3])) - bgplib.relationPriority(asgraph.getRelation(path2[2], path2[3]));
	return path1[3] - path2[3];

def bellmanford():
	global border, inborder, asgraph, pathtable, fixed;
	while len(border) > 0:
		asn = border.pop(0);
		del inborder[asn];
		propath = pathtable[asn][0];
		for nb in asgraph[asn].keys():
			if not fixed.has_key(nb) and nb not in propath[2:-1] and bgplib.appendable(asgraph[nb][asn], propath[-1]):
				if not pathtable.has_key(nb):
					pathtable[nb] = [];
				newpath = [propath[0], propath[1] + 1, nb];
				#newpath = [int(propath[0]*math.exp(-propath[1]-1)), propath[1] + 1, nb];
				newpath.extend(propath[2:-1]);
				if asgraph[nb][asn] != bgplib.SIBLING_TO_SIBLING:
					newpath.append(asgraph[nb][asn]);
				else:
					newpath.append(propath[-1]);
				if updatePath(pathtable[nb], newpath) and not inborder.has_key(nb):
					border.append(nb);
					inborder[nb] = 1;


prefixlist = [];

def readPrefixFile(filename):
	try:
		f = open(filename, "r");
		line = f.readline();
		while len(line) > 0:
			if line.find(":") > 0:
				tmp = line[:-1].split(":");
			else:
				tmp = [line[:-1]];
			prefixlist.append(bgplib.CPrefix(tmp[0]));
			line = f.readline();
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;


def outputPath(outputdir, asn, prefix, pathset):
	try:
		f = open(outputdir + "/" + str(asn), "a");
		f.write("*** " + prefix + "\n");
		for i in range(len(pathset)):
			f.write(bgplib.array2str(pathset[i], " ") + "\n");
		f.close();
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def printPathSet(prefix, pathset, cost):
	global begintimestr, asgraph
	tmpstr = "*** " + prefix + "\n";
	for path in pathset:
		tmpstr = tmpstr + str(path[0]) + " " + str(path[1]) + " " + str(path[2]);
		for i in range(2, len(path)-2):
			tmpstr = tmpstr + asgraph.getRelationSymbol(path[i], path[i+1]) + str(path[i+1]);
		tmpstr = tmpstr + "\n";
	tmpstr = tmpstr + ">>> " + begintimestr + "\n";
	tmpstr = tmpstr + "^^^ " + str(cost);
	return tmpstr;

def readASList(filename):
	try:
		f = open(filename, "r");
		line = f.readline();
		tmplist = [];
		while len(line) > 0:
			tmplist.append(line[:-1]);
			line = f.readline();
		return tmplist;
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def inferPath(val):
	global g_cmpfunc, border, inborder, fixed, pathtable, g_current_prefix, g_feedback;
	global prefixtree
	asn = int(val["asn_"]);
	pref = prefixtree.getMatchPrefix(bgplib.CPrefix(val["prefix_"]));
	g_current_prefix = pref;
	prefix = str(pref);
	#print val["prefix_"] + " " + str(prefix);
	g_cmpfunc = int(val["cmpfunc_"]);
	if val["feedback_"] == "1":
		g_feedback = True;
	else:
		g_feedback = False;
	pathtable = {};
	border = [];
	inborder = {};
	fixed = {};
	tick = bgplib.CTicker();
	getInitTable(prefix);
	bellmanford();
	if pathtable.has_key(asn):
		retstr = printPathSet(prefix, pathtable[asn], tick.report());
	else:
		retstr = "";
	del pathtable;
	del border;
	del inborder;
	del fixed;
	return retstr;

def readNexthopInfo(filename):
	try:
		f = open(filename, 'r');
		line = f.readline();
		while len(line) > 0:
			tmparr = line[:-1].split(" ");
			setNexthopInfo(bgplib.CPrefix(tmparr[0]), int(tmparr[1]), int(tmparr[2]));
			line = f.readline();
		f.close();
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;


def setNexthopHttp(val):
	prefix = bgplib.CPrefix(val["prefix_"]);
	src = int(val["src_"]);
	next = int(val["next_"]);
	setNexthopInfo(prefix, src, next);
	return "";

def setNexthopInfo(prefix, src, next):
	global firsthopHash, prefixtree;
	pref = prefixtree.getMatchPrefix(prefix);
	if pref is not None:
		if not firsthopHash.has_key(pref):
			firsthopHash[pref] = {};
		firsthopHash[pref][src] = next;

def saveNexthopInfo(filename):
	global firsthopHash;
	try:
		f = open(filename, 'w');
		for prefix in firsthopHash.keys():
			for asn in firsthopHash[prefix].keys():
				f.write(str(prefix) + " " + str(asn) + " " + str(firsthopHash[prefix][asn]) + "\n");
		f.close();
	except:
		print "Exception: ", sys.exc_info()[0];
		raise;

def terminate(val):
	global nexthopfile;
	saveNexthopInfo(nexthopfile);
	print "[" + time.strftime("%d/%b/%Y %H:%M:%S") + "] Ending service ...";
	return "";

if len(sys.argv) < 9:
	print "Usage: pathInferenceServer.py dbpath isuseknown relationfile preferencefile prefixlist known_aslist nexthop [pid]";
	sys.exit(-1);

dbpath = sys.argv[1];
isuseknown = sys.argv[2];
relationfile = sys.argv[3];
preferfile = sys.argv[4];
prefixfile = sys.argv[5];
knownasfile = sys.argv[6];
nexthopfile = sys.argv[7];
 
if len(sys.argv) > 8:
	pidfile = sys.argv[8];
	with open(pidfile, 'w') as f:
		f.write(os.getpid())

asgraph = bgplib.CASGraph(relationfile);

aspref = bgplib.CExitPreference(preferfile);

prefixtree = bgplib.CPrefixTree2(prefixfile);
prefixtree.rehash();

known_aslist = readASList(knownasfile);
print known_aslist;
#cpref = bgplib.CPrefix("192.100.178.0/24");

firsthopHash = {};
if os.path.exists(nexthopfile):
	readNexthopInfo(nexthopfile);

begintimestr = str(int(time.time()/8/3600)*8*3600);

print "[" + time.strftime("%d/%b/%Y %H:%M:%S") + "] Starting service ...";

handlelist = {};
handlelist["infer"] = httpserver.CHTTPApp(inferPath, ["asn_", "prefix_", "cmpfunc_", "feedback_"]);
handlelist["setnexthop"] = httpserver.CHTTPApp(setNexthopHttp, ["prefix_", "src_", "next_"]);
handlelist["terminate"] = httpserver.CHTTPApp(terminate, []);

httpserver.httpServerRun(handlelist, 61002);
