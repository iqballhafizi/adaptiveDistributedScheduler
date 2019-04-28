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
import matplotlib.pyplot as plt
import matplotlib.ticker as plticker
import sys
import statistics

f = open("doc/reports/analysis_20181019.txt", 'r')

testruns = []

nodeNo = []
prr = []
hopsMin = []
hopsMean = []
hopsMax = []
latencyMin = []
latencyMean = []
latencyMax = []

graphdate = "20181023"

for line in f.readlines():
    if "Test Name" in line:
        testName = line.replace("Test Name:", "")
    if "Node " in line:
        l = line.split(",")
        nodeNo.append(int(l[0]))
        prr.append(float(l[4].replace("PRR", "").replace("%", "").strip()))
        hopsMin.append(int(l[7].replace(" max]:", "").strip()))
        hopsMean.append(float(l[8]))
        hopsMax.append(int(l[9].replace("[min", "").strip()))

        try:
            lat = (l[11].replace("max]:", "").replace("[ms]", "").strip().split(" "))
            latencyMin.append(int(lat[0]))
            latencyMean.append(int(lat[1]))
            latencyMax.append(int(lat[2]))
        except:
            latencyMin.append(-1)
            latencyMean.append(-1)
            latencyMax.append(-1)
            
        if nodeNo[-1] == 11:
            testruns.append([testName, nodeNo, prr, hopsMin, hopsMean, hopsMax, latencyMin, latencyMean, latencyMax])
            nodeNo = []
            prr = []
            hopsMin = []
            hopsMean = []
            hopsMax = []
            latencyMin = []
            latencyMean = []
            latencyMax = []

def plotPRR():
    plt.figure()
    # Plot PRR
    for testrun in testruns:
        testName = testrun[0]
        nodeNo = testrun[1]
        prr = testrun[2]
     
        if not "Java" in testName:
            if "TSCH" in testName:
                plt.subplot(212)
                shape = "s"
            else:
                plt.subplot(211)
                shape = "^"
            plt.plot(nodeNo, prr, shape, label=testName)
               
     
    plt.subplot(211)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.subplot(212)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    plt.subplot(211)
    plt.ylim((80, 101))
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.ylabel("PRR in %")
    plt.subplot(212)
    plt.ylim((80, 101))
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.xlabel("Node number")
    plt.ylabel("PRR in %")
    plt.savefig('PRR_' + graphdate + '.png', bbox_inches='tight')

def plotHops():
    plt.figure()
    # Plot Hops
    for testrun in testruns:
        testName = testrun[0]
        nodeNo = testrun[1]
        hopsMin = testrun[3]
        hopsMean = testrun[4]
        hopsMax = testrun[5]
        
        hopsErrMin = []
        for i in range(len(hopsMean)):
            hopsErrMin.append((hopsMin[i] - hopsMean[i]))
        
        hopsErrMax = []
        for i in range(len(hopsMean)):
            hopsErrMax.append((-hopsMax[i] + hopsMean[i]))
        
        if not "Java" in testName:
            if "TSCH" in testName:
                plt.subplot(212)
                shape = "s"
            else:
                plt.subplot(211)
                shape = "^"
            plt.errorbar(nodeNo, hopsMean, fmt=shape, yerr=[hopsErrMax, hopsErrMin], capsize=2, label=testName)
    
    plt.subplot(211)
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    plt.ylim((1, 7))
    plt.ylabel("Number of Hops")
    
    
    plt.subplot(212)
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    plt.ylim((1, 7))
    
    plt.xlabel("Node number")
    plt.ylabel("Number of Hops")
    
    plt.savefig('Hops_' + graphdate + '.png', bbox_inches='tight')

def plotLatency():
    plt.figure()
    # Plot Hops
    for testrun in testruns:
        testName = testrun[0]
        nodeNo = testrun[1]
        latencyMin = testrun[6]
        latencyMean = testrun[7]
        latencyMax = testrun[8]
        
        latencyErrMin = []
        for i in range(len(latencyMean)):
            latencyErrMin.append((latencyMin[i] - latencyMean[i]))
        
        latencyErrMax = []
        for i in range(len(latencyMean)):
            latencyErrMax.append((-latencyMax[i] + latencyMean[i]))
        
        if not "Java" in testName:
            if "TSCH" in testName:
                plt.subplot(212)
                shape = "s"
            else:
                plt.subplot(211)
                shape = "^"
            plt.errorbar(nodeNo, latencyMean, fmt=shape, yerr=[latencyErrMax, latencyErrMin], capsize=2, label=testName)
            print("Latency overall mean: ", testName, statistics.mean(latencyMean))
    
    plt.subplot(211)
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    plt.ylim((0, 25000))
    plt.ylabel("Latency [ms]")
    
    plt.subplot(212)
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    plt.ylim((0, 25000))
    
    plt.xlabel("Node number")
    plt.ylabel("Latency [ms]")
    
    plt.savefig('Latency_' + graphdate + '.png', bbox_inches='tight')


def plotNetRecovery():
    plt.figure()
    rcvtms = []
    recoveryMin = []
    recoveryMean = []
    recoveryMax = []
    testruns = []
    testName = ""
    f = open("doc/reports/netRecovery_20181019.txt", 'r')
    for line in f.readlines():
        if "Test Name" in line:
            testName = line.replace("Test Name:", "").strip() 
        else:
            try:
                t = list(map(float, line.strip().split(" ")))
                recoveryMin.append(t[0])
                recoveryMean.append(t[1])
                recoveryMax.append(t[2])
            except:
                pass
            if len(recoveryMean) == 11:
                testruns.append([testName, recoveryMin, recoveryMean, recoveryMax])
                recoveryMin = []
                recoveryMean = []
                recoveryMax = []
    
    for testrun in testruns:
        testName = testrun[0]
        recoveryMin = testrun[1]
        recoveryMean = testrun[2]
        recoveryMax = testrun[3]
        
        style = 'sr'
        if "TSCH" in testName:
            plt.subplot(212)
        else:
            style = '^g'
            plt.subplot(211)
        
        recoveryErrMin = []
        for i in range(len(recoveryMean)):
            recoveryErrMin.append((recoveryMin[i] - recoveryMean[i]))
             
        recoveryErrMax = []
        for i in range(len(recoveryMean)):
            recoveryErrMax.append((-recoveryMax[i] + recoveryMean[i]))
        
        nodeNo = range(1, 12)
        
        plt.errorbar(nodeNo, recoveryMean, yerr=[recoveryErrMax, recoveryErrMin], 
                     fmt = style, capsize=2, label = testName)
        
        
        ax = plt.gca()
        loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
        ax.xaxis.set_major_locator(loc)
        ax.xaxis.grid(True)
        plt.legend(loc=2)
        
        plt.ylabel("Time to node reachable [s]")
    plt.xlabel("Node number")
    plt.tight_layout(rect=[0, 0, 1, 1])
        
    plt.savefig('NetRecovery_' + graphdate + '.png', bbox_inches='tight')

def plotPRR_VM():
#    plt.figure()
    # Plot PRR
    for testrun in testruns:
        testName = testrun[0]
        nodeNo = testrun[1]
        prr = testrun[2]

        if "Java" in testName and not '12' in testName and not '25' in testName:
            shape = "."
            plt.plot(nodeNo, prr, shape, label=testName, markersize=12)
        if "TSCH" in testName and "minimal" in testName and not "Java" in testName and not '12' in testName and not '25' in testName:
            shape = "x"
            plt.plot(nodeNo, prr, shape, label=testName, markersize=12)


    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.ylim((98, 101))
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    plt.xlabel("Node number")
    plt.ylabel("PRR in %")

def plotLatency_VM():
    #plt.figure()
    # Plot Hops
    for testrun in testruns:
        testName = testrun[0]
        nodeNo = testrun[1]
        latencyMin = testrun[6]
        latencyMean = testrun[7]
        latencyMax = testrun[8]
        
        latencyErrMin = []
        for i in range(len(latencyMean)):
            latencyErrMin.append((latencyMin[i] - latencyMean[i]))
        
        latencyErrMax = []
        for i in range(len(latencyMean)):
            latencyErrMax.append((-latencyMax[i] + latencyMean[i]))
        
        if "Java" in testName and not "12" in testName and not "25" in testName:
            plt.plot(nodeNo, latencyMean, 's', label=testName)
        if "TSCH" in testName and "minimal" in testName and not "Java" in testName and not "12" in testName and not "25" in testName:
            plt.plot(nodeNo, latencyMean, '^', label=testName)
    
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    #plt.ylim((0, 25000))
    
    plt.xlabel("Node number")
    plt.ylabel("Latency [ms]")

def plotPRR_SFSB():
#    plt.figure()
    # Plot PRR
    for testrun in testruns:
        testName = testrun[0]
        nodeNo = testrun[1]
        prr = testrun[2]

        print(testName)
        if "TSCH" in testName and not "Java" in testName and not '12' in testName and not '25' in testName and not "TUHH" in testName:
            shape = "."
            plt.plot(nodeNo, prr, shape, label=testName, markersize=10)
        if "TSCH" in testName and "TUHH" in testName and not "Java" in testName:
            shape = "x"
            plt.plot(nodeNo, prr, shape, label=testName, markersize=10)

    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.ylim((98, 101))
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    plt.xlabel("Node number")
    plt.ylabel("PRR in %")

def plotLatency_SFSB():
    #plt.figure()
    # Plot Hops
    for testrun in testruns:
        testName = testrun[0]
        nodeNo = testrun[1]
        latencyMin = testrun[6]
        latencyMean = testrun[7]
        latencyMax = testrun[8]
        
        latencyErrMin = []
        for i in range(len(latencyMean)):
            latencyErrMin.append((latencyMin[i] - latencyMean[i]))
        
        latencyErrMax = []
        for i in range(len(latencyMean)):
            latencyErrMax.append((-latencyMax[i] + latencyMean[i]))
        
        if "TSCH" in testName and not "TUHH" in testName and not "Java" in testName and not "12" in testName and not "25" in testName:
            plt.plot(nodeNo, latencyMean, '.', label=testName, markersize=10)
        if "TSCH" in testName and "TUHH" in testName and not "Java" in testName:
            plt.plot(nodeNo, latencyMean, 'x', label=testName, markersize=10)
    
    ax = plt.gca()
    loc = plticker.MultipleLocator(base=1.0) # this locator puts ticks at regular intervals
    ax.xaxis.set_major_locator(loc)
    ax.xaxis.grid(True)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2)
    plt.tight_layout(rect=[0, 0, 0.65, 1])
    #plt.ylim((0, 25000))
    
    plt.xlabel("Node number")
    plt.ylabel("Latency [ms]")

def printAverages():
    print('"testName";"PRR";"hopsMin";"hopsMean";"hopsMax";"latencyMin";"latencyMean";"latencyMax"')
    for testrun in testruns:
        testName = testrun[0].strip()
        prr = statistics.mean(testrun[2])
        hopsMin = statistics.mean(testrun[3])
        hopsMean = statistics.mean(testrun[4])
        hopsMax = statistics.mean(testrun[5])
        latencyMin = statistics.mean(testrun[6])
        latencyMean = statistics.mean(testrun[7])
        latencyMax = statistics.mean(testrun[8])
        
        print("%s;%2.2f;%2.2f;%2.2f;%2.2f;%2.2f;%2.2f;%2.2f" % 
              (testName, prr, hopsMin, hopsMean, hopsMax, latencyMin, latencyMean, latencyMax))
        
printAverages()
# plotPRR()
# plotHops()
# plotLatency()
# plotNetRecovery()
# 
# plt.subplot(211)
# plotPRR_VM()
# plt.subplot(212)
# plotLatency_VM()
# plt.savefig('VM_Comparision_' + graphdate + '.png', bbox_inches='tight')

# plt.subplot(211)
# plotPRR_SFSB()
# plt.subplot(212)
# plotLatency_SFSB()
# plt.savefig('TUHH_Comparision_' + graphdate + '.png', bbox_inches='tight')
