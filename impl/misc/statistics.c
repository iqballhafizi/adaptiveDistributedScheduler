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

#include <statistics.h>

#if CONTIKI_TARGET_PREON32 || CONTIKI_TARGET_PREON32L4
#include <definitions.h>
#include <system.h>
#include <stdio.h>
#else
#define ATOMIC_BLOCK_START
#define ATOMIC_BLOCK_END
#endif

static volatile uint32_t eventCounters[STAT_EVENT_TYPE_END];

char* eventStrings[] =  {
        "STAT_WAKEUP",
        "STAT_CCA_CHECK",
        "STAT_RADIO_RX_PACKET",
        "STAT_RADIO_TX_OK",
        "STAT_RADIO_TX_COLLISION",
        "STAT_RADIO_TX_NOACK",
        "STAT_RADIO_TX_ERROR",
        "M_TX_COLLISION",
        "M_TX_NOACK",
        "M_TX_OK",
        "M_TX_ERROR",
        "STAT_RDC_DROP_DUPLICATE",
        "STAT_CSMA_DROP_PACKET",
        "STAT_LAST_PACKET_TRAIN_LENGTH",
        "STAT_EVENT_TYPE_END"
};

void statistics_reset(){
    ATOMIC_BLOCK_START
    for (int i=0; i<STAT_EVENT_TYPE_END; i++){
        eventCounters[i] = 0;
    }
    ATOMIC_BLOCK_END
}

void statistics_add(EventType eventType){
    ATOMIC_BLOCK_START
    eventCounters[eventType] += 1;
    ATOMIC_BLOCK_END
}

void statistics_set(EventType eventType, int value){
    ATOMIC_BLOCK_START
    eventCounters[eventType] = value;
    ATOMIC_BLOCK_END
}

uint32_t statistics_get(EventType eventType){
    uint32_t retval;
    ATOMIC_BLOCK_START
    retval = eventCounters[eventType];
    ATOMIC_BLOCK_END
    return retval;
}

void statistics_print(){
    printf("##\n");
    for (int i=0; i<STAT_EVENT_TYPE_END; i++){
        printf("%s : %u\n", eventStrings[i], (unsigned int)statistics_get(i));
    }
    printf("##\n");
}
