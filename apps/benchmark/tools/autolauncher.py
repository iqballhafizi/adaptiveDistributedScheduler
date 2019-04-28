#!/usr/bin/python3

# Launches Preon32 applications (preon32binaries/*) on the remote server with a dedicated time schedule.

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


import os
import fnmatch
import sys
import subprocess
import datetime
import time

# Number of retries
RETRIES = 5

# Upload timeout
UPLOAD_TIMEOUT = 120

def printUsage():
    print("Incorrect arguments.")
    print("Usage: python3 autolauncher.py [runtime in minutes] [ports ..]")

if len(sys.argv) < 3:
    printUsage()
    sys.exit(1)

ports = sys.argv
ports.pop(0)

try:
    runtime = int(ports.pop(0))
except:
    printUsage()
    sys.exit(1)

# Get filenames of all binaries
binary_names = fnmatch.filter(os.listdir('preon32binaries'), '*.preon32bin')
binary_namesl4 = fnmatch.filter(os.listdir('preon32binaries'), '*.preon32l4bin')

if len(binary_namesl4) > 0:
    binary_names = binary_namesl4

# Check, if there are any binaries
if len(binary_names) < 1:
    print("ERROR: No binary found.")
    sys.exit(1)

runschedule = {}

def computeSchedule(names):
    print("Schedule for the tests:")
    print("-----------------------")
    print()
    
    stoptime = datetime.datetime.now()
    
    for name in names:
        print("Testname: ", name)
        uploadtime = stoptime.replace(second=0, microsecond=0)
        diff = 10 - (uploadtime.minute % 10)
        if diff < 5:
            diff += 10
        uploadtime = uploadtime + datetime.timedelta(minutes = diff)
        starttime = uploadtime + datetime.timedelta(minutes = 10)
        stoptime = uploadtime + datetime.timedelta(minutes = runtime)
        
        runschedule[name] = { "upload" : uploadtime, "start": starttime, "stop": stoptime }
        diff = 10 - (stoptime.minute % 10)
        if diff < 5:
            diff += 10
        stoptime = stoptime + datetime.timedelta(minutes = diff)
        
        print("Uploadtime", uploadtime)
        print("Starttime", starttime)
        print("Stoptime", stoptime)
        print()

def wait_until(tm):
    while True:
        diff = (tm - datetime.datetime.now()).total_seconds()
        if diff <= 0:
            return
        elif diff <= 0.1:
            time.sleep(0.001)
        elif diff <= 0.5:
            time.sleep(0.01)
        elif diff <= 1.5:
            time.sleep(0.1)
        else:
            time.sleep(1)

computeSchedule(binary_names)

for binary_name in binary_names:
    # Upload on all given port instances
    print("Waiting for Upload-Time ..")
    wait_until(runschedule[binary_name]["upload"])
    for port in ports:
        for r in range(RETRIES):
            try:
                print("Uploading firmware [ " + binary_name + " ] to the Preon32 on port [" + port + " ] @ ", datetime.datetime.now())
                ret = subprocess.run(["python3", "tools/stm32loader.py", 
                                      '-p', port, '-e', '-w', '-q', '-v', 'preon32binaries/' + binary_name], 
                                      stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout = UPLOAD_TIMEOUT)
                if "Verification OK" in ret.stdout.decode("utf-8"):
                    print("Uploading to the Preon32 was successful.")
                    break
                else:
                    print("Uploading Failed, Retry (", r, ")")
            except subprocess.TimeoutExpired:
                print("Uploading Failed, Retry (", r, ")")

    # Now start logging 
    lparallel = ["python3", "tools/log_parallel.py"]
    testname = binary_name.replace("benchmark_app_", "").replace(".preon32bin", "")
    lparallel.append(testname)
    lparallel.append(str(runtime * 60))
    lparallel.extend(ports)

    # Start the test
    print("Waiting for Launch-Time ..")
    wait_until(runschedule[binary_name]["start"])
    print("Launching the test @ ", datetime.datetime.now())
    try:
        subprocess.run(lparallel, timeout = runtime * 60 + 10)
    except subprocess.TimeoutExpired:
        print("Warning: Test timed out!")

    print("Finished logging.")
    print()
