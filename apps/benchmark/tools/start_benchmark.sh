#! /bin/bash

# Simple script to distribute the app over Raspberry Pi's to the Preon32.
# Please modify the IP addresses according to your system.

# List of the Server address for each connected Preon32Shuttle.
#Servers=(
#'pi@192.168.1.171' 'pi@192.168.1.171' 'pi@192.168.1.171' 'pi@192.168.1.171'
#)

Servers=(
'pi@192.168.1.172' 'pi@192.168.1.172' 'pi@192.168.1.172' 'pi@192.168.1.172'
'pi@192.168.1.173' 'pi@192.168.1.173' 'pi@192.168.1.173' 'pi@192.168.1.173'
'pi@192.168.1.174' 'pi@192.168.1.174' 'pi@192.168.1.174' 'pi@192.168.1.174'
)

# USB Serial port numbers for each connected Preon32Shuttle.
Ports=(
0 1 2 3
0 1 2 3
0 1 2 3
)

# Ports=(
# 0 0 2
# )

#upload the app
Server=${Servers[$1]}
Port=${Ports[$1]}

echo "Starting:" ${Server} "USB:" ${Port}

# Start the app in a separate screen
ssh ${Server} "sudo ./addPreonDevices.sh && screen -dmS logging ~/logger.sh ${Port}"

# read -rsp $'Press any key to continue...\n' -n 1 key
