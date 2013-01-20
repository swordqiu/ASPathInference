#!/usr/bin/env python

import json
import sys
import os
import re
import bgplib
import time
import socket
import httpserver
from  asinfodb import ASInfoBase


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
        tmpstr = '%d' % self.path[0]
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


class LUFPolicyInferASPath(LUFInferASPath):
    """ LUF + AS policy """

    def get_aslink_preference(self, as1, as2):
        return bgplib.relationPriority(self.db.asgraph.getRelation(as1, as2))


class SPFPolicyInferASPath(SPFInferASPath):
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


def resolve_dns(name):
    import dns.resolver
    try:
        rets = []
        answers = dns.resolver.query(name, 'A')
        for rdata in answers:
            rets.append('%s' % rdata)
        return rets
    except Exception as e:
        print e
    return None


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
            addr_list = resolve_dns(src)
            if addr_list is None or len(addr_list) == 0:
                return None
            elif len(addr_list) == 1:
                return self.get_paths(addr_list[0])
            else:
                oas_list = {}
                for addr in addr_list:
                    asns = self.db.get_origin_asns(addr)
                    for asn in asns:
                        oas_list[str(asn)] = 1
                rets = []
                for asn_str in oas_list.keys():
                    paths = self.get_paths(asn_str)
                    rets.append(paths)
                return rets
        return self.get_asn_paths(asn)

    def result_to_string(self, result):
        tmpstr = "*** %s %d\n" % (str(result['prefix']), result['asn'])
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
        pathlist = self.db.get_sure_paths(self.prefix_str)['paths'];
        for path in pathlist:
            #print path;
            for i in range(len(path)):
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
                self.pathtable[subpath[0]].add_known_path(subpath, path_relation)
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



def main():
    import argparse
    parser = argparse.ArgumentParser(description='AS Path Inference.')
    parser.add_argument('--db-path', metavar='<DB_PATH>',
                            default='/opt/data/data/oixdb',
                            help='AS Path DB path')
    parser.add_argument('--as-relationship', metavar='<AS_RELATIONSHIP>',
                            default='/opt/data/data/oix_relation_degree',
                            help='AS Relationship file')
    parser.add_argument('--link-preference', metavar='<LINK_PREFERENCE>',
                            default='/opt/data/data/oix_preference',
                            help='Link preference file')
    parser.add_argument('--prefix-list', metavar='<PREFIX_LIST>',
                            default='/opt/data/data/oix_prefixlist',
                            help='Prefix list file')
    parser.add_argument('--src', metavar='<SRC_ASN_IP>',
                            action='append', help='Source ASN or IP')
    parser.add_argument('--prefix', metavar='<DEST_PREFIX>',
                            help='Destination prefix or IP')
    parser.add_argument('--use-known', action='store_true',
                            help='Use known paths')
    parser.add_argument('--algorithm', metavar='<ALGORITHM>',
                            choices=['SPF', 'LUF', 'SPFPolicy', 'LUFPolict'],
                            help='Path selection algorithm')
    parser.add_argument('--pid', metavar='<PID_FILE>',
                            help='Path of pid file')

    args = parser.parse_args()

    db = ASInfoBase(args.db_path, args.as_relationship, args.link_preference,
                args.prefix_list)

    if args.prefix and args.src:
        if is_valid_address(args.prefix):
            inferer = ASPathInferer(db, args.prefix, args.use_known, args.algorithm)
            for src in args.src:
                if is_valid_address(src):
                    print inferer.get_paths_in_string(src)
                else:
                    print 'Invalid src %s' % src
        else:
            print "Invalid prefix %s" % (args.prefix)
    else:
        if args.pid:
            with open(args.pid, 'w') as f:
                f.write('%d' % os.getpid())
        print "Ready to serve"
        handlelist = {};
        handlelist["infer"] = httpserver.CHTTPApp(inferPath, ["src_",
                                    "prefix_", "use_known_", "algorithm_"], db);
        handlelist['aspeers'] = httpserver.CHTTPApp(get_aspeers, ['asn_'], db);
        handlelist['surepaths'] = httpserver.CHTTPApp(get_sure_paths, ['prefix_'], db);
        handlelist["terminate"] = httpserver.CHTTPApp(terminate, [], db);

        httpserver.httpServerRun(handlelist, 61002);


def is_valid_address(addr):
    if re.match(r'\d+(\.\d+){3}', addr):
        if bgplib.validPrefix(addr):
            return True
        else:
            return False
    else:
        return True


def inferPath(vals, db):
    if is_valid_address(vals['prefix_']):
        if not re.match(r'^\d{1,3}(\.\d{1,3}){3}(/\d{1,2})?', vals['prefix_']):
            addr_list = resolve_dns(vals['prefix_'])
            if addr_list is None or len(addr_list) == 0:
                return 'Invalid domain name %s' % vals['prefix_']
            else:
                prefix = addr_list[0]
            #else:
            #    return 'Domain name %s resolves to multiple address: %s' % (
            #            vals['prefix_'], ','.join(addr_list))
        else:
            prefix = vals['prefix_']
        inferer = ASPathInferer(db, prefix, bool(vals['use_known_']),
                                vals['algorithm_'])
        if is_valid_address(vals['src_']):
            ret = inferer.get_paths_in_string(vals['src_'])
            #print ret
            return ret
        else:
            return 'Invalid src %s' % vals['src_']
    else:
        return 'Invalid prefix %s' % vals['prefix_']


def get_aspeers(vals, db):
    asn = vals['asn_']
    if re.match(r'/^AS%d+/i', asn):
        asn = asn[2:]
    peers = db.get_peers(int(asn))
    rets = []
    for p in peers:
        rets.extend(p)
    #print rets
    return ' '.join(map(str, rets))


def get_sure_paths(vals, db):
    prefix = vals['prefix_']
    if not re.match(r'^\d{1,3}(\.\d{1,3}){3}(/\d{1,2})?$', prefix):
        addr_list = resolve_dns(prefix)
        if addr_list is None or len(addr_list) == 0:
            return 'Invalid domain name %s' % prefix
        else:
            prefix = addr_list[0]
    paths = db.get_sure_paths(prefix)
    if paths is not None:
        return json.dumps(paths)
    else:
        return 'No data'


def terminate(vals, db):
    sys.exit(0)


if __name__ == '__main__':
    main()
