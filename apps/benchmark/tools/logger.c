/*
 * Copyright (c) 2018, Torsten Hueter, VIRTENIO GmbH.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the VIRTENIO GmbH nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE VIRTENIO GMBH BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Logs the output of a connected Preon32 and adds a time stamp.

#include <math.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <stdbool.h>
#include <stropts.h>
#include <poll.h>

#include <errno.h>

// Timeout for output [seconds] -
// the logger resets the Preon32 after this period, if it doesn't get any output.
#define OUTPUT_TIMEOUT (2 * 60)

// Log printing
#define printlog(...) { printTimestamp(); printf(__VA_ARGS__); printf("\n"); }

/// The baud rate of the connected Preon32.
#define BAUDRATE B115200

/// Size of the char buffer.
#define BUFFERSIZE 255

/// Device name, e.g. /dev/ttyUSB0
static char deviceName[BUFFERSIZE];

/// Prints the actual time stamp
static void printTimestamp() {
    char buf[BUFFERSIZE];
    int ms;
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    ms = lrint(tv.tv_usec / 1000.0);
    if (ms >= 1000) {
        ms -= 1000;
        tv.tv_sec++;
    }

    tm_info = localtime(&tv.tv_sec);

    strftime(buf, BUFFERSIZE, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s,%03d ", buf, ms);
}

/**
 * Delay function
 *
 * @param milliseconds is the delay time in milliseconds.
 */
static void delay(unsigned long milliseconds) {
    unsigned long d;
    clock_t start, end;
    d = milliseconds * (CLOCKS_PER_SEC / 1000);
    start = end = clock();
    while ((signed long) (start - end) < d) {
        start = clock();
    }
}

// Serial port communication code taken from https://www.raspberrypi.org/forums/viewtopic.php?t=153385
// See also http://www.cmrr.umn.edu/~strupp/serial.html#config

/**
 * Set the interface attributes.
 *
 * @param fd File descriptor
 * @param speed baud rate
 * @param parity parity value
 */
static void setInterfaceAttribs(int fd, int speed, int parity) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        printlog("[LOGGER] Error: failed to get attributes (%s)! Exiting ..", deviceName);
        exit(-1);
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printlog("[LOGGER] Error: failed to set attributes (%s)! Exiting ..", deviceName);
        exit(-1);
    }
}

/**
 * Set the blocking attributes.
 */
static void setBlocking(int fd, int should_block) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        printlog("[LOGGER] Error: failed to get attributes (%s)! Exiting ..", deviceName);
        return;
    }

    tty.c_cc[VMIN] = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printlog("[LOGGER] Error: failed to set attributes (%s)! Exiting ..", deviceName);
    }
}

/**
 * Reset the Preon32, run the program from flash.
 *
 * @param fd File descriptor.
 */
static void resetPreon32(int fd) {
    int flag = TIOCM_RTS;
    ioctl(fd, TIOCMBIS, &flag);
    delay(10);
    flag = TIOCM_DTR;
    ioctl(fd, TIOCMBIC, &flag);
    delay(10);
    ioctl(fd, TIOCMBIS, &flag);
    delay(10);
}

/**
 * Erases the temporary char buffer.
 */
void eraseBuffer(char* buf) {
    for (int i = 0; i < BUFFERSIZE; i++) {
        buf[i] = 0;
    }
}

/**
 * Starts logging of the Preon32.
 *
 * @param modemdevice is the device string, e.g. /dev/ttyUSB0
 * @param timeout is the timeout for logging in [s]
 */
void startLogging(char* modemdevice, int timeout) {
    int fd;
    char buf[255];
    struct pollfd fds[1];
    int ret, res;

    // Start of log recording
    unsigned long start = time(NULL);

    strncpy(deviceName, modemdevice, BUFFERSIZE);

    /* open the device */
    fd = open(modemdevice, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == 0) {
        perror(modemdevice);
        printlog("[LOGGER] Error: failed to open device (%s)! Exiting ..", deviceName);
        exit(-1);
    }

    setInterfaceAttribs(fd, BAUDRATE, 0);
    setBlocking(fd, 0); // enable blocking

    /* Open STREAMS device. */
    fds[0].fd = fd;
    fds[0].events = POLLRDNORM;

    printlog("%s", "[Logger] Starting.");
    printlog("%s", "[Logger] Resetting the Preon32.");
    resetPreon32(fd);

    eraseBuffer(buf);
    time_t lastoutput = time(NULL);
    int k = 0;
    for (;;) {
        if (time(NULL) - lastoutput > OUTPUT_TIMEOUT) {
            printlog("[Logger] Got no output within the time limit ..");
            printlog("%s", "[Logger] Resetting the Preon32.");
            resetPreon32(fd);
            lastoutput = time(NULL);
        }

        if (time(NULL) - start > timeout) {
            printlog("%s", "[Logger] Log time elapsed.");
            return;
        }

        ret = poll(fds, 1, 1000);
        if (ret > 0) {
            if (fds[0].revents & POLLHUP) {
                printlog("[LOGGER] Got hang up event ..");
            }
            if (fds[0].revents & POLLRDNORM) {
                if (k < BUFFERSIZE - 1) {
                    res = read(fd, &buf[k], 1);
                    if (res > 0) {
                        lastoutput = time(NULL); // Update the last output time
                        buf[k + 1] = 0; // Always terminate string
                        if (buf[k] == 10) {
                            printTimestamp();
                            printf("%s", buf);
                            eraseBuffer(buf);
                            k = 0;
                        } else {
                            k++;
                        }
                    }
                } else {
                    // Force printing
                    printTimestamp();
                    printf("%s", buf);
                    eraseBuffer(buf);
                    k = 0;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc == 3) {
        int timeout = atoi(argv[2]);
        if (timeout > 60 * 60 * 24 * 7) {
            printf("Maximum timeout is: %i s. Exiting ..", 60 * 60 * 24 * 7);
            exit(-1);
        }
        startLogging(argv[1], timeout);
        printlog("[Logger] finished.");
    } else {
        printf("Usage: ./logger [portname] [timeout(seconds)]\n");
    }
    return 0;
}
