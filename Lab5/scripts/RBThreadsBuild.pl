: # -*- perl -*-
  eval 'exec perl -S  $0 ${1+"$@"}' 
    if 0;  # if running under some shell
#
# Copied from /Lab3/scripts/combineBuild and modified

$myname = "RBThreadsBuild";

$cwd = `pwd`; chomp $cwd;
if (!($cwd =~ m|/code$|)) {
  die "$myname: Curdir should be the 'code' directory.\n";
}

$bin = "./sc";
if (-e $bin) {
  unlink($bin);
}

$src = "SOR_comp.c";
if(!(-f $src)) {
  die "$myname: Source file $src not found.\n";
}

print "compiling $src to $bin\n";
system("gcc -O2 $src -lrt -lm -o $bin");
if (!(-x $bin)) {
  die "$myname: No binary $bin, compile error?\n";
}

# run a series of tests
# harvest data
#            BASE  DELTA  ITERS
$cmd = "$bin  10    10     20";
print "$cmd\n";
system($cmd);

$ddir = "part4_data";
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

print "SOR_by_strips tests complete\n";
