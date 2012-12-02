#!/usr/bin/perl

use Cwd;
use File::Spec;
use File::Basename;

$prog_base = dirname(dirname(File::Spec->rel2abs(__FILE__)));

$home = shift;

if (!$home) {
	$home = getcwd();
}

print $home."\n";

$command = "$prog_base/script/inferPath.py --db-path $home/data/oixdb --as-relationship $home/data/oix_relation_degree --link-preference $home/data/oix_preference --prefix-list $home/data/oix_prefixlist --pid $home/log/pid";
print $command."\n";
system($command);

