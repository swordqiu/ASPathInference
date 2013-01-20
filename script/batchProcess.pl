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

print $home."\n";

$table = "oix-snapshot.dat";

$command = "$prog_base/script/batchCreateDB.pl $home";
print $command."\n";
system($command);

#$command = "$home/inferASrelationship_degree.py $home/$table $home/tmp/oix_relation_degree $home/tmp/oix_relation_degree2 $home/tmp/oix_degree $home/tmp/oix_preference $home/tmp/known_aslist >> $home/access_log 2>&1";
$command = "$prog_base/script/inferRelation.py --table $home/tables/$table --as-relationship $home/tmp/oix_relation_degree --link-preference $home/tmp/oix_preference >> $home/log/access_log 2>&1";
print $command."\n";
system($command);

$command = "/bin/rm -fr $home/data";
print $command."\n";
system($command);

$command = "/bin/mv $home/tmp $home/data";
print $command."\n";
system($command);

$command = "$prog_base/script/startInfer.sh $home";
print $command."\n";
system($command);
