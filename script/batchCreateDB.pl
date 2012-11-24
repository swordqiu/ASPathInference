#!/usr/bin/perl -w

use File::Spec;
use File::Basename;

$prog_base = dirname(dirname(File::Spec->rel2abs(__FILE__)));

$home = shift;

if(-e "$home/tmp") {
  $command = "rm -fr $home/tmp";
  #print $command;
  system($command);
}

$command = "mkdir $home/tmp";
#print $command."\n";
system($command);

$command = "mkdir $home/tmp/oixdb";
#print $command."\n";
system($command);

$table = "oix-snapshot.dat";

$time = time();
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdat) = gmtime($time);
$hour = int($hour/8)*8;

$url = sprintf "http://archive.routeviews.org/oix-route-views/%04d.%02d/oix-full-snapshot-%04d-%02d-%02d-%02d00.bz2",$year+1900,$mon+1, $year + 1900, $mon + 1, $mday, $hour;

print $url;

system("mkdir -p $home/tables");

if(-e "$home/tables/$table") {
  unlink("$home/tables/$table");
}

$command = "wget -q $url -O $home/tables/$table.bz2";
#print $command."\n"; $ret = -1;
$ret = system($command);

if($ret == 0) {

  $command = "bunzip2 $home/tables/$table.bz2";
  #print $command."\n";
  system($command);

  $command = "$prog_base/bin/CreateSimpleTable1 $home/tmp/oixdb $home/tables/$table $home/tmp/peerlist > $home/tmp/oix_prefixlist";
  #print $command."\n";
  system($command);

}

$command = "cat $home/tmp/peerlist | awk \'{print \$2}\' | sort -n | uniq  > $home/tmp/knownlist";
system($command);
