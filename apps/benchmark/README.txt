Benchmark-App
-------------

This folder contains the benchmark application. With this application, packet losses of the 6LoWPAN network 
can be determined.

This is done as follows:

There are 2 types of systems: Root and Node

The root is the controller for the test and at the same time the DAG root for 6LoWPAN.

The node sends UDP packets to the root at regular intervals. These packets contain a 16-bit counter that is 
incremented each time a message is sent. The root receives these packages and generates a simple statistic.
The packet loss and PRR are displayed for the received packets.

The program is uploaded via "tools/do.sh 0", whereby the 0 must be replaced by the number of the serial interface.

At startup, the app checks whether GPIO1 is pulled to ground. To do this, pins 1 and 2 on pin header P2 on 
the Preon32Shuttle must be short-circuited. When GPIO1 is pulled to ground, the node acts as root.

If you're using a different pin for the device role, please configure it in the benchmark_app.h (GPIO_ROOT).

For multihop networks, adjust the transmit power in benchmark_app.h (BENCHMARK_TX_POWER) to force multiple hops.

--

You can find some useful scripts in the folder tools/ to deploy the app using Raspberry Pi's.

addPreonDevices.py - adds VIRTENIO device vid/pids to the Linux driver
autolauncher.py - Launches Preon32 applications (preon32binaries/*) on the remote server with a dedicated time schedule.
backupLogfiles.py - Retrieves backup files of the logfiles stored on the Raspberry Pi.
close_rpi_screens.py - Closes the logging screens on the Raspberry Pi's.
delete_all_binaries.py - Erases the binaries on the servers. 
delete_all_logfiles.py - Deletes all logfiles on the remote servers.
distribute.py - Distributes binaries to remote servers.
do-binary-variants.py - Creates custom binaries: reads config/testconfig.py, modifies config files and compiles to a binary.
do-binary.sh - Cleans, compiles and builds a binary for uploading.  
do.py - Compiles and uploads the app on a Preon32Shuttle.
log_parallel.py - Starts multiple threads to log parallel from connected Preon32.
logger.c - Logs the output of a connected Preon32 and adds a time stamp.
run_all_test.py - Runs the tests on the remote servers.
start_all.sh - Start app on all nodes from start_benchmark.sh.
start_benchmark.sh - Starts a logging screen on a Raspberry Pi with the app.
stm32loader - Bootloader for the Preon32.
upload_benchmark.sh - Uploads the benchmark app to the given servers in this file.
upload_all.py / upload_all.sh - uploads on all connected Preon32Shuttle parallel (the python script runs on UBUNTU only).

The above scripts need to be edited depending on how you have connected the Preon32Shuttles to the Raspberry Pi's
(please modify IP address / port number etc.)

--

Analysis of the logging data:

folder preon32/tools/analysis/

Scripts to compute some statistics of the log files (recorded with script logger.sh on a Raspberry Pi!). 

analyzeAllLogs.py - Analyses PRR of all files in the current directory.
analyzeLatency.py - Analyses the latency, creates statistic of the given log file (usually from an end-node)
analyzePRR.py - Analyses PRR from the given root log file.
analyzeTimings.py - Analyses slot timings (if enabled in tsch-slot-operation.h).

