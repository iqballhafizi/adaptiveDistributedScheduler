/**
 * \file
 *         Flags for benchmark app
 * \author
 *         Jens Dede <jens.dede@virtenio.de>
 *
 * Commercial software <br>
 * Copyright: (c) VIRTENIO GmbH <br>
 * Distribution of this file without permission of the VIRTENIO shareholders is strictly prohibited. <br>
 */

// Simple header file for benchmark constants

#ifndef __BENCHMARK_CONF__
#define __BENCHMARK_CONF__

// Is the packet a request or a reply?
#define BENCHMARK_TYPE_DATA         0
#define BENCHMARK_TYPE_REQUEST      1
#define BENCHMARK_TYPE_REPLY        2
#define BENCHMARK_TYPE_RTT          3

// What kind of information is transmitted?
#define BENCHMARK_CMD_TIMING        1
#define BENCHMARK_CMD_ROUTING       2

#endif
