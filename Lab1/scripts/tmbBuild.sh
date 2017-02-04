#!/usr/bin/env perl

$cwd = `pwd`; chomp $cwd;
if (!($cwd =~ m|/code$|)) {
  die "tmbBuild.sh: Curdir should be the 'code' directory.\n";
}

$bin = "./tmb";
if (-e $bin) {
  unlink($bin);
}

$src = "test_mmm_block.c";
if(!(-f $src)) {
  die "tmbBuild.sh: Source file $src not found.\n";
}

system("gcc -O1 $src -lrt -o $bin");

if (-x $bin) {
  system($bin);
} else {
  die "No binary $bin, compile error?\n";
}

$ddir = "part3_data";
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
