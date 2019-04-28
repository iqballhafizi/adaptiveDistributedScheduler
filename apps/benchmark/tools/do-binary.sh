#!/bin/bash

# Creates a binary to program it on another computer.

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

# Configure name for binary output
APP_NAME=benchmark_app

# Do a cleanup to ensure no old binaries are used for build.

echo "Cleanup..."
make TARGET=preon32 clean

# Check if arm-objcopy exists with correct prefix
type arm-eabi-objcopy >/dev/null 2>&1
if [ $? -eq 0 ]; then
  echo "Using arm-eabi-objcopy"
  ARM_OBJCOPY=arm-eabi-objcopy
else
  type arm-none-eabi-objcopy >/dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "Using arm-none-eabi-objcopy"
    ARM_OBJCOPY=arm-none-eabi-objcopy
  else
    echo "Error finding objcopy"
    exit 1
  fi
fi

# Make binary, drop non error messages
make TARGET=preon32 -j8 1>/dev/null \
&& $ARM_OBJCOPY -O binary $APP_NAME.preon32 $APP_NAME.bin

if [ $? -ne 0 ]; then
    echo "Error while creating binary"
else
    echo "created binary $APP_NAME.bin"
fi
