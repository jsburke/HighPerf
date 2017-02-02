# HighPerf
Private Repo for collaboration and work in EC527 labs and project

## Current Assignment -- Lab 1
|Taks|Subtask|status|notes|
|----|-------|------|-----|
|B&O 5.3||||
|B&O 6.5||||
|B&O 6.6||||
|B&O Blocking||||
|Part 1 |||run code, compute data, interpret on spreadsheet|
|       |Note||current loops in main need to free vecs|
|       |    ||need to make a vec_free function|
|Part 2 |||Optimize MMM using loop interchange|
|       ||||
|Part 3 |||MMM with blocking|
|Part 4 |||Matrix transpose with loop interchange and blocking
This whole assigment seems far more messy than the first unfortunately

proposed for vec_free(), will test

void vec_free(vec_ptr vec) //return 1 for good, 0 for fail
{
  free(vec->data);
  free(vec);
}


## Next Assignment -- Lab 2

Check list for where a given portion may be

| Task |Subtasks| Status |Notes|
|------|--------|--------|-----|
|B&0 Reading||J:Started||
|Part 1| | | use test_combine1-7.c|
|      |a| | compile, run, graph CPE, tinker, compare with book|
|      |b| | doubles & multiply, unroll loop by up to 10, graph CPE, explain|
|      |c| | same as b, but multiple accumulators and reassoc. transform|
|      |submit||modified code, tables, graphs, explanations|
|Part 2| | | create test_dot.c, seems to require part 1b|
|      |submit| | code with original dot & best versions, describe optimizations,  CPE and graphs for both|
|Part 3| | |  test_branch.c|
|      | | | finish coding each init_vectors to show data dependence|
|      | | | generate CPE graphs|
|      |submit| | modified code & description, CPE plots wrt different data, explanation of CPE; possibly assembly level|
|Part 4| | | make test_eval.c, problem 5.20 from B&O|
|      |submit| | decription of methods, code, plots and CPEs of original and best, justify performance maybe assembly|
|Part 5| | | complain somehow|
