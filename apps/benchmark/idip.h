/**
 * @file
 *         Simple database to connect an IPv6 address to an unique id (for
 *         example for array operations)
 *
 * @author
 *         Jens Dede <jens.dede@virtenio.de>
 *
 * Commercial software
 * Copyright: (c) VIRTENIO GmbH
 * Distribution of this file without permission of the VIRTENIO shareholders is strictly prohibited.
 */

#ifndef IDIP_H_
#define IDIP_H_

#include <net/ip/uip.h>
#include "stdbool.h"
#include <stdio.h>
#include <string.h>

// Max number of entries
#ifndef IPID_ENTRIES
#define IPID_ENTRIES 20
#endif

// Define some Values for the flag field
#define IPID_INVALID    1   ///< Mark value as invalid / empty
#define IPID_VALID      2   ///< Mark value as valid / used

typedef struct idip_t {
    uint8_t id;             ///< The id of the entry
    uip_ipaddr_t ipaddr;    ///< Corresponding IP address
    uint8_t flag;           ///< Some flags to mark entry
} idip_t;

uip_ipaddr_t getIpAddress(uint8_t id);
uint8_t getId(uip_ipaddr_t address);
uint8_t addIpAddress(uip_ipaddr_t address);
uint8_t removeByAddress(uip_ipaddr_t address);
uint8_t removeById(uint8_t id);
void initIdIp(void);

#endif /* IDIP_H_ */
