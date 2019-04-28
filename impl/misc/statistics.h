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

#ifndef PREON32_MISC_STATISTICS_H_
#define PREON32_MISC_STATISTICS_H_

#include <stdint.h>
#include <stdio.h>

typedef enum {
    STAT_WAKEUP,
    STAT_CCA_CHECK,
    STAT_RADIO_RX_PACKET,
    STAT_RADIO_TX_OK,
    STAT_RADIO_TX_COLLISION,
    STAT_RADIO_TX_NOACK,
    STAT_RADIO_TX_ERROR,
    M_TX_COLLISION,
    M_TX_NOACK,
    M_TX_OK,
    M_TX_ERROR,
    STAT_RDC_DROP_DUPLICATE,
    STAT_CSMA_DROP_PACKET,
    STAT_LAST_PACKET_TRAIN_LENGTH,
    STAT_EVENT_TYPE_END
} EventType;

/**
 * Reset all values.
 */
void statistics_reset();

/**
 * Add an event to the statistic.
 * @param eventType is the type of the event.
 */
void statistics_add(EventType eventType);

/**
 * Set a value.
 * @param eventType is the type of the event.
 */
void statistics_set(EventType eventType, int value);

/**
 * Returns the actual counter value for a given type.
 * @param eventType is the type of the event.
 * @return the counter value.
 */
uint32_t statistics_get(EventType eventType);

/**
 * Prints the statistic to STDOUT.
 */
void statistics_print();

#endif /* PREON32_MISC_STATISTICS_H_ */
