#!/usr/bin/perl

use LWP::Simple;
use Cwd;

use File::Spec;
use File::Basename;

$prog_base = dirname(dirname(File::Spec->rel2abs(__FILE__)));

$home = shift;

if (!$home) {
	$home = getcwd();
}

#print $home."\n";

$table = "oix-snapshot.dat";

$command = "$prog_base/script/batchCreateDB.pl $home";
print $command."\n";
system($command);

#$command = "$home/inferASrelationship_degree.py $home/$table $home/tmp/oix_relation_degree $home/tmp/oix_relation_degree2 $home/tmp/oix_degree $home/tmp/oix_preference $home/tmp/known_aslist >> $home/access_log 2>&1";
$command = "$prog_base/script/inferRelation.py $home/tables/$table $home/tmp/oix_relation_degree $home/tmp/oix_preference >> $home/log/access_log 2>&1";
print $command."\n";
system($command);

$url = "http://127.0.0.1:61002/terminate";
get($url);

if(-e "$home/log/pid") {
  open(PID, "<$home/log/pid");
  $pid = <PID>;
  chomp($pid);
  close(PID);
  $command = "kill -9 $pid";
  print $command."\n";
  system($command);
}

$command = "/bin/rm -fr $home/data";
print $command."\n";
system($command);

$command = "/bin/mv $home/tmp $home/data";
print $command."\n";
system($command);

# pathInferenceServer.py dbpath isuseknown relationfile preferencefile prefixlist known_aslist nexthop [pid]
$command = "$prog_base/script/pathInferenceServer.py $home/data/oixdb true $home/data/oix_relation_degree $home/data/oix_preference $home/data/oix_prefixlist $home/data/knownlist $home/log/nexthopinfo $home/log/pid >> $home/log/access_log 2>&1 &";
print $command."\n";
system($command);

