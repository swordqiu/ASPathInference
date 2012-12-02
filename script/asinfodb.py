#!/usr/bin/env python

import sys
import os
import re
import bgplib
import time
import subprocess


class ASInfoBase(object):

    def __init__(self, db_path, relationfile, preferfile, prefixfile):
        self.begin_time = int(time.time()/8/3600)*8*3600;
        self.db_path = db_path
        self.asgraph = bgplib.CASGraph(relationfile);
        self.aspref = bgplib.CExitPreference(preferfile);
        self.prefixtree = bgplib.CPrefixTree2(prefixfile);
        self.prefixtree.rehash();
        self.find_known_aslist()

    def find_known_aslist(self):
        self.known_aslist = []
        fp = re.compile(r'^(?P<asn>\d+)_prefix$')
        for f in os.listdir(self.db_path):
            m = fp.search(f)
            if m is not None:
                self.known_aslist.append(int(m.group('asn')))
        print self.known_aslist, len(self.known_aslist)

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

    def get_sure_paths(self, prefix):
        prefix = str(self.get_match_prefix(prefix))
        paths = []
        for viewas in self.known_aslist:
            pathlist = self.get_sure_path(viewas, prefix)
            paths.extend(self.get_sure_path(viewas, prefix))
        return paths

    def get_origin_asns(self, prefix):
        paths = self.get_sure_paths(prefix)
        asns = {}
        for path in paths:
            asns[path[-1]] = asns.get(path[-1], 0) + 1
        rets = []
        for asn, ct in sorted(asns.items(), key=lambda x: x[1], reverse=True):
            rets.append(asn)
        return rets

    def get_lib_bin_path(self):
        if getattr(self, 'lib_bin_path', None) is None:
            path = os.path.join(os.path.dirname(bgplib.__file__), '../bin')
            self.lib_bin_path = os.path.abspath(path)
        return self.lib_bin_path

    def get_sure_path(self, asn, prefixstr):
        bin_path = os.path.join(self.get_lib_bin_path(), 'GetSurePath')
        if os.path.exists(bin_path):
            try:
                cmds = [bin_path, self.db_path, str(asn), prefixstr]
                p = subprocess.Popen(cmds, stdout=subprocess.PIPE)
                output = p.communicate()[0]
                lines = output.split('\n')
                paths = []
                for l in lines:
                    l = l.strip()
                    if len(l) > 0:
                        p = []
                        for asn in re.split(r'\s+', l):
                            if len(asn) > 0:
                                p.append(int(asn))
                        paths.append(p[1:])
                return paths
            except Exception as e:
                print e
        return []


def main():
    import argparse
    parser = argparse.ArgumentParser(description='AS Path DB.')
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

    args = parser.parse_args()

    db = ASInfoBase(args.db_path, args.as_relationship, args.link_preference,
                args.prefix_list)

    print db.get_origin_asns('166.111.64.1')


if __name__ == '__main__':
    main()
