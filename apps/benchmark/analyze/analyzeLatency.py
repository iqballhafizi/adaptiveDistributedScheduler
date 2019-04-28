#! /usr/bin/python3

# Compute latency based on the given log files

import re
import os
import fnmatch
import subprocess
import sys
import statistics
import numpy as np
from datetime import datetime
from statistics import mean

def maxDeviation(l):
    return max(abs(np.array(l) - statistics.mean(l)))

def printMeanVariation(x):
    print(" [min, mean, max]: ", end='')
    print(int(min(x)), int(statistics.mean(x)), int(max(x)), " [ms]")

def getIpAddress(filename):
    with open(filename, 'r') as f:
        for i in range(500):
            line = f.readline()
            if "Local IPv6 addresses:" in line:
                address = f.readline()
                return address.strip().split(" ")[-1].replace('[', '').replace(']', '').strip()
            if "Local Addresses: " in line:
                return line.strip().split(" ")[-1].replace('[', '').replace(']', '').strip()

if len(sys.argv) < 2 or len(sys.argv) > 3:
    print("Usage: python3 analyzeLatency.py [directory] [filter (opt.)]")
    sys.exit(0)

# Take the directory name from the args
dir = sys.argv[1]

logNames = fnmatch.filter(os.listdir(dir), '*.log')

if len(sys.argv) == 3:
    logNames = [n for n in logNames if n.startswith(sys.argv[2])]


latestNumber = 0
for ln in logNames:
    with open(dir +'/' + ln, 'r') as f:
        latencies = []
        address = getIpAddress(dir +'/' + ln)
        for line in f:
            if "response" in line and "within" in line:
                number = int(line.split(" ")[-5])
                if not number == latestNumber:
                    latencies.append(int(line.split(" ")[-3]))
                else:
                    print("Warning found duplicate", number)
                latestNumber = number
        if (len(latencies) > 0):
            print("Latencies of node with address: ", address)
            printMeanVariation(latencies)