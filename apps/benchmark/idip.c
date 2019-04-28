/**
 * @file
 *         Simple database to connect an IPv6 address to an unique id (for
 *         example for array operations)
 *
 * @author
 *         Jens Dede <jens.dede@virtenio.de>
 *
 * Commercial software <br>
 * Copyright: (c) VIRTENIO GmbH <br>
 * Distribution of this file without permission of the VIRTENIO shareholders is strictly prohibited. <br>
 */

#include "idip.h"

#include "net/ip/uip-debug.h"

static idip_t idList[IPID_ENTRIES];

/**
 * Initialize the database, mark all entries as invalid and unused.
 * Can also be used to reset the database.
 */
void initIdIp(void) {
    uint8_t i = 0;
    // Mark all entries as empty
    for (i = 0; i < IPID_ENTRIES; i++) {
        idList[i].flag = IPID_INVALID;
        idList[i].id = i + 1;
    }
}

/**
 * Adds an IP address to table
 * @param address
 * 			IP address to add
 * @return
 * 			id where the address is stored
 */
uint8_t addIpAddress(uip_ipaddr_t address) {
    uint8_t i = 0;
    uint8_t id;
    // First check if entry is still in database
    id = getId(address);
    if (id != 0){
        PRINTA("Found address in the database, index %i\n", id);
        return id;
    }

    PRINTA("Adding a new entry to the database: ");
    uip_debug_ipaddr_print(&address);
    PRINTA("\n");

    for (i = 0; i < IPID_ENTRIES; i++) {
        if (idList[i].flag == IPID_INVALID) {
            idList[i].flag = IPID_VALID;
            memcpy(&idList[i].ipaddr, &address, sizeof(address));
            return i + 1;
        }
    }
    return 0; // return 0 if table is full
}

/**
 * Convert id to IP address
 * @param id
 * 			ID to convert
 * @return
 * 			IP address corresponding to given id
 */
uip_ipaddr_t getIpAddress(uint8_t id) {
    uip_ipaddr_t retAddr;
    uint8_t i;
    memset(&retAddr, 0, sizeof(retAddr));	// return 0 if not found
    for (i = 0; i < IPID_ENTRIES; i++) {
        if (idList[i].flag == IPID_VALID && idList[id].id == id)
            return idList[i].ipaddr;
    }
    return retAddr;  // return zero-address
}

/**
 * Convert IP address to id
 * @param address
 * 			ip address to convert
 * @return
 * 			Returns corresponding id
 */
uint8_t getId(uip_ipaddr_t address) {
    uint8_t i;
    for (i = 0; i < IPID_ENTRIES; i++) {
        if (idList[i].flag == IPID_VALID
                && memcmp(&address, &idList[i].ipaddr, sizeof(address)) == 0)
            return idList[i].id;
    }
    return 0; // nothing found
}

/**
 * Removes a database entry. Entry is selected by IP address
 * @param address
 * 			Address corresponding to the entry to delete
 * @return
 * 			1 if success, 0 if fail
 */
uint8_t removeByAddress(uip_ipaddr_t address) {
    uint8_t i;
    for (i = 0; i < IPID_ENTRIES; i++) {
        if (idList[i].flag == IPID_VALID
                && memcmp(&address, &idList[i].ipaddr, sizeof(address)) == 0) {
            idList[i].flag = IPID_INVALID;
            return 1;
        }
    }
    return 0;
}

/**
 * Remove a database entry. Entry is selected by id
 * @param id
 * 			id corresponding to the entry to delete
 * @return
 * 			1 if success, 0 if fail
 */
uint8_t removeById(uint8_t id) {
    uint8_t i;
    for (i = 0; i < IPID_ENTRIES; i++) {
        if (idList[i].flag == IPID_VALID && idList[i].id == id) {
            idList[i].flag = IPID_INVALID;
            return 1;
        }
    }
    return 0;
}
