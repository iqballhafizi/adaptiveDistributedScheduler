#!/usr/bin/python3

# Calls analyzePRR.py for all root logs.

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

#! /usr/bin/python3

import os
import fnmatch
import subprocess

addresses = [
    "1631:5635:3053:d630",
    "1631:5335:3053:d930",
    "334:4834:3852:db39",
    "643:3033:3256:d631",
    "646:6433:3256:d431",
    "1634:2735:3053:db30",
    "1631:5035:3053:db30",
    "647:5133:3256:d331",
    "a38:5139:314d:d633",
    "638:5733:3256:d931",
    "1631:4335:3053:d830"
]
 
filters = [ 
    "ContikiMAC-16_20180926",
    "ContikiMAC-8_20180926",
    "ContikiMAC-4_20180926",
    "ContikiMAC-2_20180926",
    "TSCH-minimal-3_20180926",
    "TSCH-minimal-6_20180926",
    "TSCH-minimal-12_20180802",
    "TSCH-minimal-25_20180926",
    "java_TSCH-minimal-3_20180926",
    "java_TSCH-minimal-6_20180926",
    "java_TSCH-minimal-12_20180926",
    "java_TSCH-minimal-25_20180926",
    "TSCH-orchestra-RB-7_20180926",
    "TSCH-orchestra-SB-7_20180926",
    "TSCH-SFSB_20181023_11_00_00_20181023_11_10"
]
 
description = [
    "ContikiMAC, 16 Hz Duty Cycle",
    "ContikiMAC, 8 Hz Duty Cycle",
    "ContikiMAC, 4 Hz Duty Cycle",
    "ContikiMAC, 2 Hz Duty Cycle",
    "TSCH, minimal schedule, schedule length 3 slots",
    "TSCH, minimal schedule, schedule length 6 slots",
    "TSCH, minimal schedule, schedule length 12 slots",
    "TSCH, minimal schedule, schedule length 25 slots",
    "TSCH (Java), minimal schedule, schedule length 3 slots",
    "TSCH (Java), minimal schedule, schedule length 6 slots",
    "TSCH (Java), minimal schedule, schedule length 12 slots",
    "TSCH (Java), minimal schedule, schedule length 25 slots",
    "TSCH, Orchestra scheduler, RB-7",
    "TSCH, Orchestra scheduler, SB-7",
    "TSCH, TUHH blacklisting"
    ]
 
subfolder = "logfile_backup_192.168.1.174"
subfolders = [  "logfile_backup_192.168.1.172",
                "logfile_backup_192.168.1.173",
                "logfile_backup_192.168.1.174"]
 
 
 
for i in range(len(filters)):
    print()
    print("Test Name: ", description[i])
    prrdata = ""
    for fn in os.listdir(subfolder):
        if fn.startswith(filters[i]) and "USB2" in fn: 
            completed = subprocess.run(["python3", "analyze/analyzePRR.py", subfolder + "/" + fn], stdout=subprocess.PIPE)
            prrdata = completed.stdout.decode()
    latencydata = ""
    for subfolder in subfolders:
        completed = subprocess.run(["python3", "analyze/analyzeLatency.py", subfolder, filters[i]], stdout=subprocess.PIPE)
        latencydata = latencydata + completed.stdout.decode()
 
    lat = latencydata.split("Latencies of node with address:  ")
     
    j = 1
    for address in addresses:
        print(j, end=", ")
        if "java" in filters[i]:
            a = address.split(":")
            a[0] = hex(int(a[0], 16) & ~512).replace("0x", "")
            addr = ""
            for ad in a:
                addr = addr + ":" + ad.upper()
            address = addr[1:]
        for line in prrdata.split("\n"):
            if address in line:
                print(line, end="")
        for line in lat:
            if address in line:
                print(line.split("\n")[1].replace("[avr, stddev, maxdev]: ", ", lat: "))
        j += 1
