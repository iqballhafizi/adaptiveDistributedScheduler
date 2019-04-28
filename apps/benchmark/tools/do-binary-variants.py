#!/usr/bin/python3

# Creates custom binaries: reads config/testconfig.py, modifies config files and compiles to a binary.

#
# Copyright (c) 2018, Torsten Hueter, VIRTENIO GmbH.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the VIRTENIO GmbH nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE VIRTENIO GMBH BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import subprocess
import os
import sys
import pathlib
import time

# Check for word in string 
# compare: https://stackoverflow.com/questions/5319922/python-check-if-word-is-in-a-string
def contains_word(s, w):
    return (' ' + w + ' ') in (' ' + s + ' ')

# Add config path and import the test config
sys.path.append(str(pathlib.Path().absolute()) + '/config')
from testconfig import configs

# Create binary folder
pathlib.Path('./preon32binaries').mkdir(parents=True, exist_ok=True)

# Get arch from the shell
if len(sys.argv) == 1:
    arch = "preon32"
else:
    arch = sys.argv[1]

# Cleaning up
print("Cleanup ..")
subprocess.run(["make", "TARGET=" + arch, "clean"])

# Open up the contiki config and set the parameters
# contiki_conf = open("../../")
for config in configs:
    modified = []
    print()
    print("Modifying contiki-conf.h for config:", config["name"])
    
    # Open up the contiki-conf.h and modify the flags
    contiki_conf = open("../../contiki-conf.h", 'r')
    # Get longest name and use this for spacing
    l = 0
    for flag in config["contiki-conf"]:
        if len(flag[0]) > l:
            l = len(flag[0])
    lines = contiki_conf.readlines()
    for line in lines:
        if "#define" in line:
            for flag in config["contiki-conf"]:
                if contains_word(line, flag[0]):
                    print("Modify flag: ", flag)
                    # Add some spaces for better formatting
                    spaces = l - len(flag[0])
                    line = "#define " + flag[0] + " ".ljust(spaces) + str(flag[1]) + "\n"
            
        modified.append(line)
    contiki_conf.close()
    
    contiki_conf = open("../../contiki-conf.h", 'w')
    contiki_conf.writelines(modified)
    contiki_conf.close()
    
    modified = []
    print()
    print("Modifying benchmark_app.h for config:", config["name"])
    
    # Open up the benchmark-app.h and modify the flags
    if "benchmark-conf" in config.keys():
        benchmark_conf = open("./benchmark_app.h", 'r')
        # Get longest name and use this for spacing
        l = 0
        for flag in config["benchmark-conf"]:
            if len(flag[0]) > l:
                l = len(flag[0])
        lines = benchmark_conf.readlines()
        for line in lines:
            if "#define" in line:
                for flag in config["benchmark-conf"]:
                    if contains_word(line, flag[0]):
                        print("Modify flag: ", flag)
                        # Add some spaces for better formatting
                        spaces = l - len(flag[0])
                        line = "#define " + flag[0] + " ".ljust(spaces) + str(flag[1]) + "\n"
                
            modified.append(line)
        benchmark_conf.close()
    
        benchmark_conf = open("./benchmark_app.h", 'w')
        benchmark_conf.writelines(modified)
        benchmark_conf.close()

    # Activate / Deactivate TSCH in the Makefile
    en_tsch = config["Makefile"]["ENABLE_TSCH"]
    mfile = open("./Makefile", 'r')
    mlines = mfile.readlines()
    mlinesmod = []
    for mline in mlines:
        if "MODULES" in mline and "core/net/mac/tsch" in mline:
            mline = "MODULES += core/net/mac/tsch\n"
            if not en_tsch:
                mline = "# " + mline
        if "APPS" in mline and "orchestra" in mline:
            mline = "APPS += orchestra\n"
            if not en_tsch:
                mline = "# " + mline
        mlinesmod.append(mline)
    mfile.close()
    mfile = open("./Makefile", 'w')
    mfile.writelines(mlinesmod)
    mfile.close()

    # Compile the benchmark app
    print()
    print("Building the binary ..")

    # Build binary
    subprocess.run(["make", "TARGET=" + arch, '-j4'])

    # Rename the binary
    binary_name =   "preon32binaries/benchmark_app_" + config["name"] + "_" + \
                    time.strftime("%Y%m%d_%H_%M_%S") + "." + arch + "bin"
    subprocess.run(["arm-none-eabi-objcopy", '-O', 'binary', 'benchmark_app.' + arch, binary_name])
    print("Created binary: ", binary_name)
