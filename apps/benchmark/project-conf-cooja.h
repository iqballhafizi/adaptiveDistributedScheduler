/*
 * Copyright (c) 2018, VIRTENIO GmbH
 * Copyright (c) 2015, SICS Swedish ICT.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \author Simon Duquennoy <simonduq@sics.se>
 * \author Torsten Hueter, VIRTENIO GmbH
 *
 * Project config is only used for Cooja!
 */

#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

#if USE_TSCH

/* Set to run orchestra */
#ifndef USE_ORCHESTRA
#define USE_ORCHESTRA 0
#endif /* USE_ORCHESTRA */

/*******************************************************/
/********* Enable RPL non-storing mode *****************/
/*******************************************************/

//#undef UIP_CONF_MAX_ROUTES
//#define UIP_CONF_MAX_ROUTES 0 /* No need for routes */
//#undef RPL_CONF_MOP
//#define RPL_CONF_MOP RPL_MOP_NON_STORING /* Mode of operation*/
#undef ORCHESTRA_CONF_RULES
#define ORCHESTRA_CONF_RULES { &eb_per_time_source, &unicast_per_neighbor_rpl_ns, &default_common }

/*******************************************************/
/********************* Enable TSCH *********************/
/*******************************************************/

/* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

/* IEEE802.15.4 frame version */
#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012

/* TSCH and RPL callbacks */
#define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval
#define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network
#define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network

/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/

/** Defines the default hopping sequence. */
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE      TSCH_HOPPING_SEQUENCE_16_16

/* TSCH logging. 0: disabled. 1: basic log. 2: with delayed
 * log messages from interrupt */

#undef TSCH_LOG_QUEUE_LEN
#define TSCH_LOG_QUEUE_LEN 32

#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 2


/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0xabcd

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 0

/** Default schedule length, larger values improve energy efficiency */
#if USE_BLACKLISTING
// For blacklisting
#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH       63
#undef SIXTOP_CONF_MAX_TRANSACTIONS
#define SIXTOP_CONF_MAX_TRANSACTIONS            8
/* TSCH link layer neighbor callback */
#define TSCH_QUEUE_ADD_NBR_CALLBACK sfsb_new_nbr
/* TSCH blacklist callback */
#define TSCH_BLACKLIST_CALLBACK sfsb_bl_check
#else
#ifndef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH      63
#endif
#endif

// Timing parameter for TSCH
#define TSCH_CONF_DEFAULT_TIMESLOT_LENGTH       10000
#define TSCH_CONF_RX_WAIT                       4000

// EB Period: Set to lower values to accelerate association times
#define TSCH_CONF_EB_PERIOD                     (10 * CLOCK_SECOND)
#define TSCH_CONF_MAX_EB_PERIOD                 (20 * CLOCK_SECOND)

#if USE_ORCHESTRA
/* See apps/orchestra/README.md for more Orchestra configuration options */
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0 /* No 6TiSCH minimal schedule */
#define TSCH_CONF_WITH_LINK_SELECTOR 1 /* Orchestra requires per-packet link selection */
/* Orchestra callbacks */
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed
#endif /* USE_ORCHESTRA */

// Timing for the EXP5438 target
#if CONTIKI_TARGET_EXP5438 
/* Delay between GO signal and SFD: radio fixed delay + 4Bytes preample + 1B SFD -- 1Byte time is 32us
 * ~327us + 129preample = 456 us */
#define RADIO_DELAY_BEFORE_TX ((unsigned)US_TO_RTIMERTICKS(456))
/* Delay between GO signal and start listening
 * ~50us delay + 129preample + ?? = 183 us */
#define RADIO_DELAY_BEFORE_RX ((unsigned)US_TO_RTIMERTICKS(183))
/* Delay between the SFD finishes arriving and it is detected in software */
#define RADIO_DELAY_BEFORE_DETECT 0
#endif

#else

/*----------------------------------------------------------------------------*/
/*                          ContikiMAC flags                                 */
/*----------------------------------------------------------------------------*/

/* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     contikimac_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  contikimac_framer
#undef NETSTACK_CONF_LLSEC
#define NETSTACK_CONF_LLSEC   nullsec_driver

#endif

/*******************************************************/
/************* Other system configuration **************/
/*******************************************************/

#if CONTIKI_TARGET_COOJA
#define COOJA_CONF_SIMULATE_TURNAROUND 0
#endif /* CONTIKI_TARGET_COOJA */

#endif /* __PROJECT_CONF_H__ */
