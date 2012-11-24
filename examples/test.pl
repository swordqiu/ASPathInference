#!/usr/bin/perl -w
#

require "../script/aspathinfer.pl";

$srcas = shift;
$dstip = shift;

if(!$dstip) {
  print "Infer.pl srcas dstip/dstprefix\n";
  exit(-1);
}

print &inferPath($srcas, $dstip, "LUF", "NO_FEEDBACK")."\n";
