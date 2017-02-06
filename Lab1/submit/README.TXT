READ ME FOR LAB 1 -- John Burke and Robert Munafo

The Included

PDF of results with graphs

directory 'matlabUtil' contains Matlab scripts we used to hasten
graphing. Here they are listed by name (omitting '.m') and use

  graph_2d    : plots base 10 logarith versus number of elements for both
                manners of part 1
  graph_ratio : plots the ratio of reverse:forward vs elements for part 1,
                showing speed up
  graph_CPE   : plots cycles per element versus elements for both manners of
                part one
  graph_mmm   : plots cycles versus elements for each loop interchange
  graph_mblock  plots cycles vs. matrix size for the blocked optimisation
                of ijk matrix multiply (test_mmm_block.c)


Directory 'code' contains our C code

  combine2d.c      : given, but slightly modified for lab computers and rapid
                     testing, part 1
  mmm_inter.c      : same as above, part 2
  test_mmm_block.c : blocking matrix multiply for part 3
  test_transpose.c : transpose code for part four, tested interchange on it,
                     slower was commented out
  test_transpose_block.c : blocking version of above
