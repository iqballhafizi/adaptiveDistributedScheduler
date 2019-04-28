#! /usr/bin/python3

# Analyze the time until all nodes are reachable
#
# WORK IN PROGRESS, not yet usable !!

import sys
import re
import statistics
import fnmatch
import os
import datetime
from email import message

def isRootLog(filename):
    f = open(filename, 'r')
    # Search in the first 100 lines for the ROOT identifier
    for i in range(100):
        if "Device role: ROOT (sink)" in f.readline():
            f.close()
            return True
    f.close()
    return False

if len(sys.argv) < 2 or len(sys.argv) > 3:
    print("Usage: python3 analyzeNetRecovery.py [directory] [filter (opt.)]")
    sys.exit(0)

# Take the directory name from the args
dir = sys.argv[1]

logNames = fnmatch.filter(os.listdir(dir), '*.log')

if len(sys.argv) == 3:
    logNames = [n for n in logNames if n.startswith(sys.argv[2])]

isPrint = True

tms = [[], [], [], [], [], [], [], [], [], [], [], []]
messageTimes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
rebootTime = 0

for ln in logNames:
    if isRootLog(dir + '/' + ln):
        f = open(dir + '/' + ln, 'r', errors='ignore')
        
        for line in f.readlines():
            # Search for : Rebooting NOW!
            if "Rebooting NOW!" in line:
                # Get the timestamp from the actual line
                rebootTime = datetime.datetime.strptime(line.replace("Rebooting NOW!", "").strip(), "%Y-%m-%d %H:%M:%S,%f")
                isPrint = True
                
                if not messageTimes[1] == 0:
                    for i in range(len(messageTimes)):
                        tms[i].append(messageTimes[i])
                messageTimes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            if "Found address in the database" in line and not rebootTime == 0:
                ls = line.split(" ")
                t = ls[0] + " " + ls[1]
                nodeTime = datetime.datetime.strptime(t, "%Y-%m-%d %H:%M:%S,%f")
                nodeNo = int(ls[-1])
                if nodeNo == 11:
                    isPrint = False
                if messageTimes[nodeNo] == 0:
                    messageTimes[nodeNo] = (nodeTime - rebootTime).seconds 

for i in range(1, 12):
    tms[i] = [x for x in tms[i] if x != 0]
    print(min(tms[i]), statistics.mean(tms[i]), max(tms[i]))
