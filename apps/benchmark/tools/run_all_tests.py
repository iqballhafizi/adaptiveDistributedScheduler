#!/usr/bin/python3

# Runs the tests on the remote servers.

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
import sys
import pathlib
import time

# Define maximum log time
MAX_LOG_TIME = 60 * 24 * 7

def printUsage():
    print("Incorrect arguments.")
    print("Usage: python3 run_all_tests.py [runtime in minutes]")

# Check arguments
if (len(sys.argv) != 2):
    printUsage()
    sys.exit(1)
try: 
    sys.argv.pop(0)
    testtime = int(sys.argv.pop(0))
except:
    printUsage()
    sys.exit(1)
if testtime <= 0 or testtime > MAX_LOG_TIME:
    print("Error: Logtime needs to be in the range (1, " + str(MAX_LOG_TIME) + ") minutes.")
    sys.exit(1)

# Distribute first the logging tools / binaries to the Raspberry Pi
print("Distributing of all test files ..")
subprocess.run(["python3", "tools/distribute.py"])
print()

# Add config path and import the test config
sys.path.append(str(pathlib.Path().absolute()) + '/config')
from servers import servers

# Now start the autolauncher on the Raspberry Pi's
# Start the app in a separate screen
for server in servers:
    portnames = ""
    for portname in server["portnames"]:
        portnames += " " + portname
    
    print("Adding all Preon VID/PID ..");
    subprocess.run(["ssh", server["name"], "sudo python3 tools/addPreonDevices.py"])
    print()
    
    print("Launching apps on server " + server["name"]);
    subprocess.run(["ssh", server["name"], 
                    "screen -L logfiles/summary_" + time.strftime("%Y%m%d_%H_%M_%S") + ".log " + 
                    "-dmS testlauncher python3 tools/autolauncher.py " + str(testtime) + portnames])
    print()
