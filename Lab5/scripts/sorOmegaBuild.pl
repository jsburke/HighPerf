: # -*- perl -*-
  eval 'exec perl -S  $0 ${1+"$@"}' 
    if 0;  # if running under some shell
#
# Copied from /Lab3/scripts/combineBuild and modified

$myname = "sorOmegaBuild";

$cwd = `pwd`; chomp $cwd;
if (!($cwd =~ m|/code$|)) {
  die "$myname: Curdir should be the 'code' directory.\n";
}

$bin = "./sor_omega";
if (-e $bin) {
  unlink($bin);
}

$src = "test_SOR_OMEGA.c";
if(!(-f $src)) {
  die "$myname: Source file $src not found.\n";
}

print "compiling $src to $bin\n";
system("gcc -O1 -lrt $src -o $bin");
if (!(-x $bin)) {
  die "$myname: No binary $bin, compile error?\n";
}

# run a series of tests
# harvest data
system("$bin 2 8 0.50 10");
print "\n";

system("$bin 4 16 0.40 10");
print "\n";

system("$bin 5 25 0.30 10");
print "\n";

system("$bin 8 32 0.20 10");
print "\n";

system("$bin 9 41 0.10 10");
print "\n";

system("$bin 15 50 0.09 10");
print "\n";

system("$bin 15 75 0.08 10");
print "\n";

$ddir = "part1_data";
if(!(-d $ddir)) {
  print "making directory for data output files\n";
  system("mkdir $ddir");
}
$ncsv = (`ls *.csv | wc -l`) + 0;
if ($ncsv > 0) {
  system("mv *.csv $ddir");
} else {
  die "$myname: No .csv output files were produced.\n";
}

# system("rm $bin");

print "test_SOR_OMEGA tests complete\n";
