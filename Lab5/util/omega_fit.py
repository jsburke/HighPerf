#!/usr/bin/env python
#------------------------------------------------------
# omega_fit [-h] [-l|-e]
#------------------------------------------------------
#
#	-h --help		Display this message
#
#	option
#
#	-l --length		Calculate omega based on length
#	-e --elements		Calculate omega based on number of array elements (default)
#
#	responds with Omega fitted by the following curve
#		omega = 1.33 + 0.062 ln x
#

import os, sys, argparse
from math import log

class parse_defined_error(argparse.ArgumentParser):
	def error(self, msg = ""):
		if(msg): print("\nERROR: %s\n" % msg)
		file = open(sys.argv[0])
		for (lineNum, line) in enumerate(file):
			if(line[0] != "#"): sys.exit(msg != "")
			if((lineNum == 2) or (lineNum >= 4)): print(line[1:].rstrip("\n"))

def parse_cmd():
	parser = parse_defined_error(add_help = False)

	# Normal Args

	parser.add_argument("-h", "--help", action = "store_true")

	# Additional
	one_of = parser.add_mutually_exclusive_group()

	one_of.add_argument("-l", "--length",   type = int)
	one_of.add_argument("-e", "--elements", type = int)

	# General handler

	opts = parser.parse_args()
	if opts.help:
		parser.error()
		sys.exit()
	else:
		return opts
#------------------------------------------------------
# Math
#------------------------------------------------------

def omega_fit(elems):
	fit = (1.33 + 0.062 * log(elems))
	if (fit >= 2.0):
		fit = 1.9999
	return fit

#------------------------------------------------------
# Main
#------------------------------------------------------

def main():
	opts = parse_cmd()

	if opts.length:
		print("%f" % omega_fit(opts.length ** 2))

	if opts.elements:
		print("%f" % omega_fit(opts.elements))

	sys.exit()

main()		