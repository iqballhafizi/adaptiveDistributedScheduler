/*
 * Copyright (c) 2018, VIRTENIO GmbH.
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

#ifndef __RADIO_SIMPLESEC_H__
#define __RADIO_SIMPLESEC_H__

#include <stdint.h>
#include <stdbool.h>
#include <spi.h>

#include <dev/packetFifo.h>

#define SIMPLESEC_MIC_LENGTH 2

/**
 * \file
 *         Simple encryption methods using the stream cipher ChaCha20.
 *         Used directly on lowest level by the radio driver.
 * \author
 *         Torsten Hueter, VIRTENIO GmbH.
 */

/**
 * Set the encryption/decryption key.
 */
void simplesec_set_key(uint8_t* value);

/**
 * Encrypts the given packet and transmits it over SPI.
 */
bool simplesec_encrypt(FrameBuffer* frame);

/**
 * Receives a packet from the transceiver and decrypts the given packet.
 */
bool simplesec_decrypt(FrameBuffer* frame);

#endif /* __RADIO_SIMPLESEC_H__ */
