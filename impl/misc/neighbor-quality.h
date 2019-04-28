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

#ifndef PREON32_MISC_NEIGHBOR_QUALITY_H_
#define PREON32_MISC_NEIGHBOR_QUALITY_H_

#include <contiki.h>
#include <net/ipv6/uip-ds6-nbr.h>

/// This is the struct for the neighbor quality table
typedef struct uip_ds6_nbr_quality {
    linkaddr_t addr;
    int lqi;
    int rssi;
} uip_ds6_nbr_quality_t;

/**
 * Overrides the rpl_ipv6_neighbor_callback and adds functions for keeping track of
 * lqi / rssi.
 *
 * @param nbr is the neighbor.
 */
void custom_ipv6_neighbor_callback(uip_ds6_nbr_t *nbr);

/**
 * Save the latest packet quality.
 */
void neighbor_quality_save();

/**
 * Get the latest packet quality or NULL if no data was recorded.
 */
const uip_ds6_nbr_quality_t* neighbor_quality_get(uip_ds6_nbr_t *nbr);

#endif /* PREON32_MISC_NEIGHBOR_QUALITY_H_ */
