#! /bin/bash

# Upload on all nodes, comment out if you have less connected Preon32Shuttle.

tools/upload_benchmark.sh 0 & \
tools/upload_benchmark.sh 1 & \
tools/upload_benchmark.sh 2 & \
tools/upload_benchmark.sh 3 & \
tools/upload_benchmark.sh 4 & \
tools/upload_benchmark.sh 5 & \
tools/upload_benchmark.sh 6 & \
tools/upload_benchmark.sh 7 & \
tools/upload_benchmark.sh 8 & \
tools/upload_benchmark.sh 9 & \
tools/upload_benchmark.sh 10 & \
tools/upload_benchmark.sh 11