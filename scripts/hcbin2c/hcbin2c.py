#!/usr/bin/python

# HcBin file conversion utility
#
# Copyright 2015-16 Hillcrest Laboratories, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License and 
# any applicable agreements you may have with Hillcrest Laboratories, Inc.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#
# hcbin2c.py
#
# This script converts a Hillcrest .hcbin formatted file that contains code
# for a sensorhub into a C HcBint_t data structure that can be used with the
# BNO070 MCU driver.
#

import sys, getopt
from HcBin import *
from HcBinGen import *
import re

def usage():
    print("hcbin2c.py [options] <input file> <output file>")
    print("  -z use compression [not yet supported]")

def baseFilename(s):
    filenameRE = re.compile("(.*)(\.[^.]*)")

    m = filenameRE.search(s)
    if (m):
        return m.group(1)
    else:
        return s

def testBaseFilename():
    for s in ["foobar", "foobar.c", "foobar.one.two"]:
        print("baseFilename('%s') = %s" % (s, baseFilename(s)))

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hz", ["help", "compress"])
    except getopt.GetoptError as err:
        print(err)
        usage()
        sys.exit(1)

    compress = True
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-z", "--compress"):
            compress = True
        else:
            assert False, "unhandled option"

    if len(args) != 2:
        print("Need to specify input and output file")
        usage()
        sys.exit(1)

    infile = args[0]
    c_outfile = baseFilename(args[1]) + ".c"
    h_outfile = baseFilename(args[1]) + ".h"
    print("Reading " + infile + "...")

    hcbin = HcBin()
    hcbin.read(infile)

    gen = HcBinGen()
    gen.setData(hcbin)
    gen.write(c_outfile, h_outfile)

if __name__ == "__main__":
    main()


