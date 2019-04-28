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

#include "radio-simplesec.h"

#include <ecrypt-sync.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <lib/crc16.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

static uint8_t key[32] = { 0 };

static uint8_t iv[8];
static ECRYPT_ctx ctx;

#define IV_BIT_SIZE 64
#define KEY_BIT_SIZE 256

void simplesec_set_key(uint8_t* value) {
    memcpy(key, value, sizeof(key));
    ECRYPT_keysetup(&ctx, key, KEY_BIT_SIZE, IV_BIT_SIZE);
}

static Uint8 encrypt(Uint8 in){
    return ECRYPT_encrypt_byte(&ctx, in);
}

bool simplesec_encrypt(FrameBuffer* frame){
    uint16_t mic;
    if (frame->parameter.length == 0){
        return true;
    }
    if (frame->parameter.length + sizeof(mic) <= MAX_FRAME_LENGTH) {
        // Compute the CRC of the frame as MIC
        mic = crc16_data(frame->buffer, frame->parameter.length, 0);
        frame->buffer[frame->parameter.length] = mic & 0xFF;
        frame->buffer[frame->parameter.length + 1] = (mic >> 8) & 0xFF;
        frame->parameter.length += sizeof(mic);

        // Now encrypt the packet
        memset(iv, 0, sizeof(iv));
        ECRYPT_ivsetup(&ctx, iv);
        ctx.current_index = 0;

        // Transmit & encrypt the frame via SPI
        spi_transceiveBlockPollingFilter(trx.spi, frame->buffer, NULL, frame->parameter.length, encrypt);

        return true;
    } else {
        return false;
    }
}

bool simplesec_decrypt(FrameBuffer* frame) {
    uint16_t mic, check;
    if (frame->parameter.length == 0){
        frame->parameter.fcsOk = true;
        return true;
    }
    if (frame->parameter.length > sizeof(mic) && frame->parameter.length <= MAX_FRAME_LENGTH) {
        memset(iv, 0, sizeof(iv));
        ECRYPT_ivsetup(&ctx, iv);
        ctx.current_index = 0;

        // Receive & decrypt the frame via SPI
        spi_transceiveBlockPollingFilter(trx.spi, NULL, frame->buffer, frame->parameter.length, encrypt);

        mic = frame->buffer[frame->parameter.length - 2] | (frame->buffer[frame->parameter.length - 1] << 8);
        frame->parameter.length -= sizeof(mic);

        check = crc16_data(frame->buffer, frame->parameter.length, 0);

        // Flag packet as invalid, when the mic not correct
        Bool isValid = check == mic;
        frame->parameter.fcsOk = isValid;
        return isValid;
    } else {
        frame->parameter.fcsOk = false;
        return false;
    }
}
