#!/usr/bin/python3

#
# Copyright (c) 2018, Torsten Hueter, VIRTENIO GmbH.
# All rights reserved.
#
# Distributes binaries to remote servers.
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
import pathlib

# Add config path and import the test config
sys.path.append(str(pathlib.Path().absolute()) + '/config')
from servers import servers

for server in servers:
    login = server["name"]

    print("Distributing files to server ", server["name"])

    # Copy autolauncher to the raspberry pi / compile logger.c
    subprocess.run(['ssh', login, 'mkdir', '-p', '~/tools'])
    subprocess.run(['rsync', '-az', 'tools/autolauncher.py', login + ":~/tools/autolauncher.py"])
    subprocess.run(['rsync', '-az', 'tools/logger.c', login + ":~/tools/logger.c"])
    subprocess.run(['ssh', login, 'gcc', 'tools/logger.c', '-o', 'tools/logger', '-lm'])
    subprocess.run(['rsync', '-az', 'tools/stm32loader.py', login + ":~/tools/stm32loader.py"])
    subprocess.run(['rsync', '-az', 'tools/addPreonDevices.py', login + ":~/tools/addPreonDevices.py"])
    subprocess.run(['rsync', '-az', 'tools/log_parallel.py', login + ":~/tools/log_parallel.py"])


    # Copy the binaries to the raspi
    subprocess.run(['ssh', login, 'mkdir', '-p', '~/preon32binaries'])
    subprocess.run(['ssh', login, 'rm', '~/preon32binaries/*'])

    for fn in os.listdir('preon32binaries'):
        if '.preon32bin' in fn or '.preon32l4bin' in fn:
            subprocess.run(['rsync', '-az', 'preon32binaries/' + fn, login + ":~/preon32binaries/" + fn])