: # -*- perl -*-
  eval 'exec perl -S  $0 ${1+"$@"}' 
    if 0;  # if running under some shell

$myname = "sorMultiBuild";

$cwd = `pwd`; chomp $cwd;
if (!($cwd =~ m|/code$|)) {
  die "$myname: Curdir should be the 'code' directory.\n";
}

$bin = "./sor_multi";
if (-e $bin) {
  unlink($bin);
}

$src = "test_SOR.c";
if(!(-f $src)) {
  die "$myname: Source file $src not found.\n";
}

print "compiling $src to $bin\n";
system("gcc -O1 -lrt $src -o $bin");
if (!(-x $bin)) {
  die "$myname: No binary $bin, compile error?\n";
}

system("$bin 8 2 1.626466 5");
print "\n";
system("$bin 8 2 1.626466 8");
print "\n";
system("$bin 8 2 1.626466 16");
print "\n";
system("$bin 8 2 1.626466 25");
print "\n";

system("$bin 11 4 1.675212 5");
print "\n";
system("$bin 11 4 1.675212 8");
print "\n";
system("$bin 11 4 1.675212 16");
print "\n";
system("$bin 11 4 1.675212 25");
print "\n";

system("$bin 15 5 1.709798 5");
print "\n";
system("$bin 15 5 1.709798 8");
print "\n";
system("$bin 15 5 1.709798 16");
print "\n";
system("$bin 15 5 1.709798 25");
print "\n";

system("$bin 35 15 1.819958 5");
print "\n";
system("$bin 35 15 1.819958 8");
print "\n";
system("$bin 35 15 1.819958 16");
print "\n";
system("$bin 35 15 1.819958 25");
print "\n";

# system("$bin 50 25 1.868704 5");
# print "\n";

system("$bin 95 35 1.934833 5");
print "\n";
system("$bin 95 35 1.934833 8");
print "\n";
system("$bin 95 35 1.934833 16");
print "\n";
system("$bin 95 35 1.934833 25");
print "\n";

system("$bin 125 50 1.97057 5");
print "\n";
system("$bin 125 50 1.97057 8");
print "\n";
system("$bin 125 50 1.97057 16");
print "\n";
system("$bin 125 50 1.97057 25");
print "\n";

system("$bin 130 70 1.986623 5");
print "\n";
system("$bin 130 70 1.986623 8");
print "\n";
system("$bin 130 70 1.986623 16");
print "\n";
system("$bin 130 70 1.986623 25");
print "\n";

system("$bin 135 75 1.992489 5");
print "\n";
system("$bin 135 75 1.992489 8");
print "\n";
system("$bin 135 75 1.992489 16");
print "\n";
system("$bin 135 75 1.992489 25");
print "\n";

system("$bin 140 80 1.998082 5");
print "\n";
system("$bin 140 80 1.998082 8");
print "\n";
system("$bin 140 80 1.998082 16");
print "\n";
system("$bin 140 80 1.998082 25");
print "\n";

$ddir = "part2_data";
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

print "test_SOR tests complete\n";