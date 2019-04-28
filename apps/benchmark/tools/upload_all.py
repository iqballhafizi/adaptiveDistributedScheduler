#!/usr/bin/env python

# Launches several terminals for uploading on all connected Preon32Shuttle parallel.

import subprocess

command = 'tools/start_benchmark.sh'
terminal = ['gnome-terminal']

# Modify the range according to the number of connected Preon32Shuttle.
for host in range(0, 12):
    terminal.extend(['--tab', '-e', command + ' ' + str(host)])

subprocess.call(terminal)

