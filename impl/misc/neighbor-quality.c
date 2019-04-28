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

#include <stdio.h>

#include "neighbor-quality.h"
#include "net/nbr-table.h"
#include "net/packetbuf.h"
#include "net/rpl/rpl.h"

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"

extern void rpl_ipv6_neighbor_callback(uip_ds6_nbr_t *nbr);

uip_ds6_nbr_quality_t ds6_neighbors_quality[NBR_TABLE_MAX_NEIGHBORS];

/**
 * Gets the index of the neighbor table entry.
 * @param table is the neighbor table.
 * @param item is the item of the table
 * @return the index or -1 if the item is not part of the table.
 */
static int index_from_item(nbr_table_t *table, const nbr_table_item_t *item) {
    if (table == NULL || item == NULL){
        return -1;
    } else {
        int retval = (int) ((char *) item - (char *) table->data) / table->item_size;
        if (retval < NBR_TABLE_MAX_NEIGHBORS && retval >= 0){
            return retval;
        } else {
            return -1;
        }
    }
}

static uip_ds6_nbr_quality_t latest_packet_quality;

/**
 * Gets the latest packet quality.
 * @param address is the address of the sender.
 * @return pointer to the quality struct or NULL if the address does not match.
 */
static uip_ds6_nbr_quality_t* getLatestPacketQuality(const linkaddr_t* address){
    if (address != NULL){
        if (linkaddr_cmp(address, &latest_packet_quality.addr)){
            return &latest_packet_quality;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

void neighbor_quality_save(){
    linkaddr_copy(&latest_packet_quality.addr,
                packetbuf_addr(PACKETBUF_ADDR_SENDER));
    latest_packet_quality.rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
    latest_packet_quality.lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
}

void custom_ipv6_neighbor_callback(uip_ds6_nbr_t *nbr){
    rpl_ipv6_neighbor_callback(nbr);
    if (nbr != NULL) {
        int idx = index_from_item(ds6_neighbors, nbr);
        if (idx != -1) {
            // Check, if latest packet matches
            uip_ds6_nbr_quality_t* pq =
                    getLatestPacketQuality((const linkaddr_t*)uip_ds6_nbr_get_ll(nbr));
            if (pq != NULL) {
                PRINTF("### Latest neighbor quality is: LQI %i; RSSI %i\n", pq->lqi, pq->rssi);
                ds6_neighbors_quality[idx] = *pq;
            }
        }
    }
}

const uip_ds6_nbr_quality_t* neighbor_quality_get(uip_ds6_nbr_t *nbr){
    int idx = index_from_item(ds6_neighbors, nbr);
    if (idx != -1) {
        return &ds6_neighbors_quality[idx];
    } else {
        return NULL;
    }
}
