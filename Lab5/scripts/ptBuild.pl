: # -*- perl -*-
  eval 'exec perl -S  $0 ${1+"$@"}' 
    if 0;  # if running under some shell

$myname = "ptBuild";

$cwd = `pwd`; chomp $cwd;
if (!($cwd =~ m|/code$|)) {
  die "$myname: Curdir should be the 'code' directory.\n";
}

$bin = "./PT_go";
if (-e $bin) {
  unlink($bin);
}

$src = "test_pt.c";
if(!(-f $src)) {
  die "$myname: Source file $src not found.\n";
}

print "compiling $src to $bin\n";
system("gcc -pthread -lrt $src -o $bin -lm");
if (!(-x $bin)) {
  die "$myname: No binary $bin, compile error?\n";
}

system("$bin 10 5 78");  # up to length of 400
print "\n";


$ddir = "part3_data";
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

system("rm $bin");

print "test_pt tests complete\n";