#!/usr/bin/python3

# Starts multiple threads to log parallel from connected Preon32.

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

import threading
import sys
import subprocess
import time
import pathlib

def printUsage():
    print("Usage: python3 log_parallel.py [testname] [timeout] [portname1] [portname2] ..")

def logger(port, testname):
    print("Starting logging on port: ", port)
    pathlib.Path('./logfiles').mkdir(parents=True, exist_ok=True)
    filename = 'logfiles/' + testname + "_" + time.strftime("%Y%m%d_%H_%M_%S") + "_" + port.replace("/dev/tty", "") + ".log"
    f = open(filename, 'w')
    subprocess.call(["tools/logger", port, str(timeout)], stdout = f)
    f.close()
 
ports = sys.argv.copy()

if len(ports) < 3:
    printUsage()
    sys.exit(1)

try:
    ports.pop(0)
    testname = ports.pop(0)
    timeout = int(ports.pop(0))
except:
    printUsage()
    sys.exit(1)
 
# Create threads and add the port names as argument
threads = [ threading.Thread(target=logger, args=[ports[i], testname]) for i in range(len(ports)) ]
 
for thread in threads:
    thread.start()
 
for thread in threads:
    thread.join()