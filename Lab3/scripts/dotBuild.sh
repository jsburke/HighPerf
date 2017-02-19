#!/usr/bin/env perl
#
# Copied from Lab1/scripts/tmbBuild.sh and modified

$cwd = `pwd`; chomp $cwd;
if (!($cwd =~ m|/code$|)) {
  die "dotBuild.sh: Curdir should be the 'code' directory.\n";
}

$bin = "./td";
if (-e $bin) {
  unlink($bin);
}

$src = "test_dot8.c";
if(!(-f $src)) {
  die "dotBuild.sh: Source file $src not found.\n";
}

system("gcc -o $bin $src -lrt -lm");

if (!(-x $bin)) {
  die "No binary $bin, compile error?\n";
}

for($DELTA=14; $DELTA<=17; $DELTA++) {
  system("$bin 0 $DELTA 10");
}

$ddir = "p1_data";
if(!(-d $ddir)) {
  system("mkdir $ddir");
}
$ncsv = (`ls *.csv | wc -l`) + 0;
if ($ncsv > 0) {
  system("mv *.csv $ddir");
} else {
  print STDERR "No .csv output files were produced.\n";
}

system("rm $bin");
