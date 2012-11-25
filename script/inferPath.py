#!/usr/bin/env python

import sys
import os
import re
import bgplib
import time
import argparse
import socket


class InferASPath(object):

    def __init__(self, db, path, relation, known_count=0, infer_count=0):
        self.db = db
        self.known_count = known_count
        self.infer_count = infer_count
        self.path_relation = relation
        self.path = path

    def known(self):
        self.known_count += 1

    def compare_known_path(self, path2):
        if self.known_count != path2.known_count:
            return - self.known_count + path2.known_count;
        if len(self.path) != len(path2.path):
            return len(self.path) - len(path2.path);
        return self.path[0] - path2.path[0];

    def get_path_str(self):
        tmpstr = '%s' % self.path[0]
        for i in range(0, len(self.path) - 1):
            tmpstr += '%s%d' % (
                self.db.asgraph.getRelationSymbol(self.path[i], self.path[i+1]),
                self.path[i+1])
        return tmpstr

    def __str__(self):
        return '%d %d %s' % (self.known_count, self.infer_count,
                                self.get_path_str())

    def __repr__(self):
        return '<KnownASPath %s>' % str(self)

    def compare_infer_path(self, path2):
        return NotImplemented


class LUFInferASPath(InferASPath):
    """ Least Uncertainty First """

    def compare_infer_path(self, path2):
        if self.path == path2.path:
            return 0
        if self.infer_count != path2.infer_count:
            return self.infer_count - path2.infer_count
        if self.known_count != path2.known_count:
            return - self.known_count + path2.known_count;
        if len(self.path) != len(path2.path):
            return len(self.path) - len(path2.path);
        exit1 = self.get_aslink_preference(self.path[0], self.path[1])
        exit2 = self.get_aslink_preference(path2.path[0], path2.path[1])
        if exit1 != exit2:
            return - exit1 + exit2
        return self.path[1] - path2.path[1];

    def get_aslink_preference(self, as1, as2):
        return self.db.aspref.getExitPref(as1, as2)


class SPFInferASPath(InferASPath):
    """ Shortest Path First """

    def compare_infer_path(self, path2):
        if self.path == path2.path:
            return 0;
        if self.infer_count*path2.infer_count == 0 and \
                self.infer_count != path2.infer_count:
            return self.infer_count - path2.infer_count
        if self.infer_count == 0 and path2.infer_count == 0 and \
                self.known_count != path2.known_count:
            return - elf.known_count + path2.known_count;
        if len(self.path) != len(path2.path):
            return len(self.path) - len(path2.path);
        exit1 = self.get_aslink_preference(self.path[0], self.path[1])
        exit2 = self.get_aslink_preference(path2.path[0], path2.path[1])
        if exit1 != exit2:
            return - exit1 + exit2
        if self.infer_count != path2.infer_count:
            return self.infer_count - path2.infer_count
        if self.known_count != path2.known_count:
            return - self.known_count + path2.known_count
        return self.path[0] - path2.path[0]

    def get_aslink_preference(self, as1, as2):
        return self.db.aspref.getExitPref(as1, as2)


def LUFPolicyInferASPath(LUFInferASPath):
    """ LUF + AS policy """

    def get_aslink_preference(self, as1, as2):
        return bgplib.relationPriority(self.db.asgraph.getRelation(as1, as2))


def SPFPolicyInferASPath(SPFInferASPath):
    """ SPF + AS policy """

    def get_aslink_preference(self, as1, as2):
        return bgplib.relationPriority(self.db.asgraph.getRelation(as1, as2))


class InferASPathSet(object):

    def __init__(self, db, path_cls):
        self.db = db
        self.path_class = path_cls
        self.paths = []

    def add_known_path(self, path, relation):
        p = self.get_match_path(path)
        if p is None:
            p = self.path_class(self.db, path, relation)
            self.paths.append(p)
        p.known()

    def get_match_path(self, path):
        for p in self.paths:
            if p.path == path:
                return p
        return None

    def compare_known_paths(self, path1, path2):
        return path1.compare_known_path(path2)

    def compare_infer_paths(self, path1, path2):
        return path1.compare_infer_path(path2)

    def sort_known_paths(self):
        self.paths.sort(self.compare_known_paths)

    def sort_infer_paths(self):
        self.paths.sort(self.compare_infer_paths)

    def __str__(self):
        return '\n'.join(map(str, self.paths))

    def __repr__(self):
        return '<InferASPathSet %s>' % str(self)

    def update_path(self, newpath):
        if len(self.paths) == 0:
            bestpath = None
        else:
            bestpath = self.paths[0].path
        replace = False
        for i in range(len(self.paths)):
            if self.paths[i].path[1] == newpath.path[1]:
                if self.paths[i].path != newpath.path:
                    self.paths[i] = newpath
                    replace = True
                    break
                else:
                    return False
        if not replace:
            self.paths.append(newpath)
        self.sort_infer_paths()
        if bestpath == self.paths[0].path:
            return False;
        else:
            return True;


class ASPathInferer(object):

    def __init__(self, db, prefix, use_known, algorithm):
        if not re.match(r'^\d+(\.\d+){3}$', prefix):
            prefix = socket.gethostbyname(prefix)
        self.db = db
        self.prefix = self.db.get_match_prefix(prefix)
        self.prefix_str = str(self.prefix)
        self.use_known = use_known
        self.infer_path_class = self.get_infer_path_class(algorithm)
        self.pathtable = {}
        self.border = []
        self.inborder = {}
        self.fixed = {}
        self.infer()

    def get_infer_path_class(self, algorithm):
        if algorithm == 'LUF':
            return LUFInferASPath
        elif algorithm == 'SPF':
            return SPFInferASPath
        elif algorithm == 'LUFPolicy':
            return LUFPolicyInferASPath
        elif algorithm == 'SPFPolicy':
            return SPFPolicyInferASPath
        else:
            return LUFInferASPath

    def infer(self):
        tick = bgplib.CTicker()
        self.init_path_table()
        self.bellmanford();
        self.infer_time = tick.report()

    def get_asn_paths(self, asn):
        if asn in self.pathtable:
            return {'db_time': self.db.begin_time,
                    'cost': self.infer_time,
                    'prefix': self.prefix,
                    'asn': asn,
                    'paths': self.pathtable[asn]}
        else:
            return None

    def get_paths(self, src):
        if re.match(r'^\d+$', src):
            asn = int(src)
        elif re.match(r'^AS\d+$', src):
            asn = int(src[2:])
        elif re.match(r'^\d+(\.\d+){3}$', src):
            asns = self.db.get_origin_asns(src)
            print 'Origin AS for %s: %s' % (src, asns)
            rets = []
            for asn in asns:
                rets.append(self.get_paths(str(asn)))
            return rets
        else:
            addr = socket.gethostbyname(src)
            if addr is not None:
                return self.get_paths(addr)
        return self.get_asn_paths(asn)

    def result_to_string(self, result):
        tmpstr = "*** %d-%s\n" % (result['asn'], str(result['prefix']))
        tmpstr += str(result['paths']) + "\n"
        tmpstr += ">>> " + str(result['db_time']) + "\n"
        tmpstr += "^^^ " + str(result['cost']) + "\n"
        return tmpstr

    def get_paths_in_string(self, asn):
        rets = self.get_paths(asn)
        tmpstr = ''
        if rets is not None:
            if isinstance(rets, list):
                for ret in rets:
                    tmpstr += self.result_to_string(ret)
            else:
                tmpstr += self.result_to_string(rets)
        return tmpstr;

    def init_path_table(self):
        for viewas in self.db.known_aslist:
            pathlist = bgplib.getSurePath(self.db.db_path, viewas,
                                            self.prefix_str);
            for path in pathlist:
                #print path;
                for i in range(1, len(path)):
                    subpath = path[i:]
                    if not self.use_known and len(subpath) > 1:
                        continue;
                    if not self.pathtable.has_key(subpath[0]):
                        self.pathtable[subpath[0]] = InferASPathSet(self.db,
                                                        self.infer_path_class);
                        self.border.append(subpath[0]);
                        self.inborder[subpath[0]] = 1;
                        self.fixed[subpath[0]] = 1;
                    path_relation = self.db.get_path_relationship(subpath)
                    self.pathtable[subpath[0]].add_known_path(subpath,
                                                            path_relation)
        for asn in self.border:
            self.pathtable[asn].sort_known_paths();
            print self.pathtable[asn];

    def bellmanford(self):
        while len(self.border) > 0:
            asn = self.border.pop(0);
            del self.inborder[asn];
            propath = self.pathtable[asn].paths[0];
            for nb in self.db.asgraph[asn].keys():
                if not self.fixed.has_key(nb) and \
                        nb not in propath.path and \
                        bgplib.appendable(self.db.asgraph[nb][asn], \
                                            propath.path_relation):
                    if not self.pathtable.has_key(nb):
                        self.pathtable[nb] = InferASPathSet(self.db,
                                                    self.infer_path_class);
                    known_count = propath.known_count
                    infer_count = propath.infer_count + 1
                    path = [nb] + propath.path
                    if self.db.asgraph[nb][asn] != bgplib.SIBLING_TO_SIBLING:
                        path_relation = self.db.asgraph[nb][asn]
                    else:
                        path_relation = propath.path_relation
                    newpath = self.infer_path_class(self.db, path,
                                            path_relation,
                                            known_count=known_count,
                                            infer_count=infer_count)
                    if self.pathtable[nb].update_path(newpath) and \
                            not self.inborder.has_key(nb):
                        self.border.append(nb)
                        self.inborder[nb] = 1


class ASInfoBase(object):

    def __init__(self, db_path, relationfile, preferfile, prefixfile, knownas):
        self.begin_time = int(time.time()/8/3600)*8*3600;
        self.db_path = db_path
        self.asgraph = bgplib.CASGraph(relationfile);
        self.aspref = bgplib.CExitPreference(preferfile);
        self.prefixtree = bgplib.CPrefixTree2(prefixfile);
        self.prefixtree.rehash();
        self.known_aslist = self.read_aslist(knownas);
        #print self.known_aslist;

    @classmethod
    def read_aslist(cls, filename):
        with open(filename, "r") as f:
            line = f.readline()
            tmplist = []
            while len(line) > 0:
                tmplist.append(line[:-1])
                line = f.readline()
            return tmplist

    def get_path_relationship(self, path):
        i = 0;
        while i < len(path) - 1 and \
            self.asgraph.getRelation(path[i], path[i+1]) == \
                bgplib.SIBLING_TO_SIBLING:
            i = i + 1;
        if i == len(path) - 1:
            return bgplib.SIBLING_TO_SIBLING;
        else:
            return self.asgraph[path[i]][path[i+1]];

    def get_match_prefix(self, prefix):
        return self.prefixtree.getMatchPrefix(bgplib.CPrefix(prefix))

    def get_origin_asns(self, prefix):
        prefix = str(self.get_match_prefix(prefix))
        asns = {}
        for viewas in self.known_aslist:
            pathlist = bgplib.getSurePath(self.db_path, viewas, prefix)
            for path in pathlist:
                if path[-1] in asns:
                    asns[path[-1]] += 1
                else:
                    asns[path[-1]] = 1
        rets = []
        for asn, ct in sorted(asns.items(), key=lambda x: x[1], reverse=True):
            rets.append(asn)
        return rets


def main():
    parser = argparse.ArgumentParser(description='AS Path Inference.')
    parser.add_argument('--db-path', metavar='<DB_PATH>', required=True,
                            help='AS Path DB path')
    parser.add_argument('--as-relationship', metavar='<AS_RELATIONSHIP>',
                            required=True, help='AS Relationship file')
    parser.add_argument('--link-preference', metavar='<LINK_PREFERENCE>',
                            required=True, help='Link preference file')
    parser.add_argument('--prefix-list', metavar='<PREFIX_LIST>',
                            required=True, help='Prefix list file')
    parser.add_argument('--known-aslist', metavar='<KNOWN_ASN>',
                            required=True, help='Known AS list')
    parser.add_argument('--src', metavar='<SRC_ASN_IP>', required=True,
                            action='append', help='Source ASN or IP')
    parser.add_argument('--prefix', metavar='<DEST_PREFIX>', required=True,
                            help='Destination prefix or IP')
    parser.add_argument('--use-known', action='store_true',
                            help='Use known paths')
    parser.add_argument('--algorithm', metavar='<ALGORITHM>',
                            choices=['SPF', 'LUF', 'SPFPolicy', 'LUFPolict'],
                            help='Path selection algorithm')

    args = parser.parse_args()

    db = ASInfoBase(args.db_path, args.as_relationship, args.link_preference,
                args.prefix_list, args.known_aslist)

    inferer = ASPathInferer(db, args.prefix, args.use_known, args.algorithm)

    for src in args.src:
        print inferer.get_paths_in_string(src)


if __name__ == '__main__':
    print main()
