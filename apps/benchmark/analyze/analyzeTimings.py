#! /usr/bin/python3

import matplotlib.pyplot as plt
import sys
import re
import statistics
import numpy as np

# Only timings for the specified frame length are analyzed
frameLength = 123
statLines = 13

try:
    timingLog = sys.argv[1]
except:
    print("Please provide the filename for the log-file!");
    sys.exit(0)

def maxDeviation(l):
    return max(abs(np.array(l) - statistics.mean(l)))

def printMeanVariation(x):
    print(" [avr, stddev, maxdev]: ", end='')
    print(int(statistics.mean(x)), int(statistics.stdev(x)), int(maxDeviation(x)), " [us]")

def filter(x):
    return (abs(x) < 20000 and x > 0)

def filterInvalidValues(values):
    return [x for x in values if filter(x)]

def filterInvalidValues2(values1, values2):
    retval1 = []
    retval2 = []
    for i in range(len(values1)):
        if filter(values1[i]) and filter(values2[i]):
            retval1.append(values1[i])
            retval2.append(values2[i])
            
    return (retval1, retval2)

timingStats = {}
try:
    f = open(timingLog, 'r')
except:
    print("Error: Please check the log-file!")
    sys.exit(0)

lines = f.readlines()
f.close()

for i in range(len(lines)):
    try:
        if str(frameLength) + " rx" in lines[i] or str(frameLength) + " tx" in lines[i]:
            stats = lines[i:i + statLines]
            for statline in stats:
                if "timing." in statline:
                    statline = statline[statline.find("timing."):]
                    property_name = statline.split(" = ")[0]
                    property_value = float(re.search(r'\d+', statline).group())
                    if property_name not in timingStats:
                        timingStats[property_name] = [property_value]
                    else:
                        timingStats[property_name].append(property_value)
    except:
        print("Read error, line: ", i)
        print(line)

if "timing.rx_pending_packet" in timingStats.keys():
    (rx_pending_packet, rx_wait_for_transmit_ack) = \
                            filterInvalidValues2(timingStats["timing.rx_pending_packet"], 
                                                 timingStats["timing.rx_wait_for_transmit_ack"])
                            
    rx_packet_processing = -(np.array(rx_pending_packet) - np.array(rx_wait_for_transmit_ack))
    
    print("RX processing time: packet received", end='')
    printMeanVariation(rx_packet_processing)
    
    (rx_transmit_ack, rx_end) = \
                            filterInvalidValues2(timingStats["timing.rx_transmit_ack"], 
                                                 timingStats["timing.rx_end"])
    
    ack_transmit_till_end_slot = -(np.array(rx_transmit_ack) - np.array(rx_end))
    print("RX processing time: ACK transmit till slot end", end='')
    
    printMeanVariation(ack_transmit_till_end_slot)

if "timing.tx_packet_prepared" in timingStats.keys():
    print("Max TX preparing time: " , max(timingStats["timing.tx_packet_prepared"]))

for k in timingStats:
    timingStats[k] = filterInvalidValues(timingStats[k])

for k in timingStats:
    print(k + " : ", end='')
    if len(timingStats[k]) > 2:
        printMeanVariation(timingStats[k])
    else:
        print(k + " dropped, to few data points for the statistic.")
    
for k in timingStats:
    plt.plot(range(len(timingStats[k])), timingStats[k], label = k)
plt.grid(True)
plt.legend(loc='best')
plt.show()