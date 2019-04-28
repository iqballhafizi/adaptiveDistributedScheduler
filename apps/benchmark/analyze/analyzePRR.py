#! /usr/bin/python3

# Simple script to analyze the logging data of the benchmark-app.
# Requires Python3

# Please install first statistics and matplotlib, e.g. on UBUNTU:
# sudo apt install python3-pip
# pip3 install statistics
# pip3 install matplotlib

# Launch script with: python3 analyzeLogs.py [logfile.txt]

import sys
import statistics
import statistics
import collections

propertyNames = ["Count", "Hops", "Entry", "Address"]

def getProperties(line):
    data = line.split("[")
    data = list(map(lambda x: x.replace("]", ""), data))
    data = list(map(lambda x: x.strip(), data))
    properties = []
    for pn in propertyNames:
        prop = [s for s in data if pn in s]
        try:
            properties.append(int(prop[0].replace(pn, '').strip()))
        except:
            properties.append(prop[0].replace(pn, '').strip())
    return tuple(properties)

entries = []
hops = []
prr = []
received = []
hopMin = []
hopMax = []
addresses = []

j = 0
e = 0

# Read dataset and extract statistic informations
try:
    fileobject = open(sys.argv[1], 'r', encoding='utf-8', errors='ignore')
except:
    print("Please provide a correct filename of the log-file as argument!")
    sys.exit(0)
    
try:
    for line in fileobject:
        if "Entry" in line:
            (count, hop, entryNo, address) = getProperties(line)
            if entryNo > e:
                e = entryNo
            while len(entries) < e + 1:
                entries.append([])
                hops.append([])
                prr.append([])
                received.append([])
                hopMin.append([])
                hopMax.append([])
                addresses.append(0)
            entries[entryNo].append(count)
            hops[entryNo].append(hop)
            addresses[entryNo] = address
            j += 1

except Exception as e: 
    print(e)
    print("Read error!")
    print("Check the following line:")
    print(line)
    sys.exit(0)

k = 1
for entry in entries:
    if entry:
        # Check, if we had an restart
        # add the latest value in this case as offset
        lastCount = entry[0]
        offset = 0
        for j in range(len(entry)):
            if entry[j] - lastCount < -100:
                print("Warning: Jump in the count values (node entry " + str(k) + " @ count " +
                      str(lastCount) + "), adjusting the count offset!")
                offset += lastCount
            lastCount = entry[j]
            entry[j] += offset   
        entry.sort()
        k += 1

# Check, if there are duplicate packets, print a warning in this case and the packet numbers
for i in range(len(entries)):
    duplictates = [item for item, count in collections.Counter(entries[i]).items() if count > 1]
    if duplictates:
        print("WARNING: found duplicate packet numbers:", duplictates, "Removing these packet numbers.")
        entries[i] = list(set(entries[i]))
        entries[i].sort()

# Compute the PRR
for j in range(len(entries)):
    if entries[j]:
        received[j] = entries[j][-1]
        error = 0
        lastCount = entries[j][0] - 1
        for count in entries[j]:
            err = count - lastCount - 1;
            error += err
            lastCount = count
        prr[j] = 100.0 - (float(error) / float(received[j]) * 100.0)
    else:
        # Special case: no packets received
        prr[j] = 0.0
        hops[j] = [0, 0, 0]
        received[j] = 0

for j in range(len(entries)):
    print("Node %2d, Address %s, Received %5d, PRR %6.2f%%, Hops [min, mean, max]: %2d, %2.2f, %2d"
          % (j + 1, addresses[j], len(entries[j]), prr[j], min(hops[j]), statistics.mean(hops[j]), max(hops[j])))
    
