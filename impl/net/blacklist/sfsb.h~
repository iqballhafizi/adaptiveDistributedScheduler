/*
 * Copyright (c) 2016, Yasuyuki Tanaka
 * Copyright (c) 2016, Centre for Development of Advanced Computing (C-DAC).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *         A 6P Scheduling Function with Soft Blacklisting (SFSB)
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 *         Leo Krueger <leo.krueger@tuhh.de> (used sf-simple as basis for sfsb)
 */

#ifndef _SIXTOP_SFSB_H_
#define _SIXTOP_SFSB_H_

#include "net/linkaddr.h"
#include "net/mac/tsch/tsch-queue.h"
#include "net/mac/tsch/sixtop/sixtop.h"

int sfsb_add_links(linkaddr_t *peer_addr, uint8_t num_links);
int sfsb_remove_links(linkaddr_t *peer_addr);
void sfsb_new_nbr(linkaddr_t addr);


#define SF_SIMPLE_MAX_LINKS  5
#define SF_SIMPLE_SFID       0xf0

#define SFSB_INIT_NUM_CELLS 1
// unit s
#define SFSB_MAX_RETRY_BACKOFF 90
// unit ms
#define SFSB_BLACKLIST_UPDATE_TIMEOUT 25000
// unit ms
#define SFSB_PIGGYBACKING_BACKOFF 700

#define CHANNEL_SWAP_OFFSET 7


extern const sixtop_sf_t sfsb_driver;
void sfsb_bl_send(void *ptr);

#endif /* !_SIXTOP_SF_SIMPLE_H_ */
