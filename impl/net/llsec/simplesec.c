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


#include "simplesec.h"

#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"

#include <ecrypt-sync.h>

#include <stdio.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

static uint8_t key[32] = {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
};

static uint8_t mic[2] = {
    0xCA, 0xFE
};

static uint8_t iv[8];
static ECRYPT_ctx ctx;
static uint8_t encrypted[PACKETBUF_SIZE];
static uint8_t decrypted[PACKETBUF_SIZE];

#define IV_BIT_SIZE 64
#define KEY_BIT_SIZE 256

/*---------------------------------------------------------------------------*/
void
set_mic(uint8_t* value)
{
  memcpy(mic, value, sizeof(mic));
}
/*---------------------------------------------------------------------------*/
void
set_key(uint8_t* value)
{
  memcpy(key, value, sizeof(key));
  ECRYPT_keysetup(&ctx, key, KEY_BIT_SIZE, IV_BIT_SIZE);
}
/*---------------------------------------------------------------------------*/
static void
send(mac_callback_t sent, void *ptr)
{
  NETSTACK_MAC.send(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static void
add_security_header(void)
{
    packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_DATAFRAME);
}
/*---------------------------------------------------------------------------*/
static int
create(void)
{
  add_security_header();

  uint8_t *dataptr;
  uint8_t data_len;
  dataptr = packetbuf_dataptr();
  data_len = packetbuf_datalen();

  if (data_len > 0 && data_len + sizeof(mic) < PACKETBUF_SIZE) {
      for (size_t i=0; i<sizeof(mic); i++){
          dataptr[data_len + i] = mic[i];
      }
      memset(iv, 0, sizeof(iv));
      ECRYPT_ivsetup(&ctx, iv);
      ECRYPT_encrypt_bytes(&ctx, dataptr, encrypted, data_len + sizeof(mic));
      memcpy(dataptr, encrypted, data_len + sizeof(mic));
      packetbuf_set_datalen(data_len + sizeof(mic));

      return framer_802154.create();
  } else {
      return FRAMER_FAILED;
  }
}
/*---------------------------------------------------------------------------*/
static int
parse(void)
{
    int result;
    const linkaddr_t *sender;

    result = framer_802154.parse();
    if(result == FRAMER_FAILED) {
        return result;
    }
    sender = packetbuf_addr(PACKETBUF_ADDR_SENDER);
      if(linkaddr_cmp(sender, &linkaddr_node_addr)) {
        PRINTF("simplesec: frame from ourselves\n");
        return FRAMER_FAILED;
    }

    if (packetbuf_datalen() > sizeof(mic)) {
        uint8_t *dataptr;
        uint8_t data_len;
        dataptr = packetbuf_dataptr();
        data_len = packetbuf_datalen();

        memset(iv, 0, sizeof(iv));
        ECRYPT_ivsetup(&ctx, iv);
        ECRYPT_decrypt_bytes(&ctx, dataptr, decrypted, data_len);
        memcpy(dataptr, decrypted, data_len);

        data_len -= sizeof(mic);
        packetbuf_set_datalen(data_len);

        for (size_t i = 0; i < sizeof(mic); i++) {
            if (dataptr[data_len + i] != mic[i]) {
                PRINTF("simplesec: frame with wrong MIC!\n");
                return FRAMER_FAILED;
            }
        }

        return result;
    }
    else {
        return FRAMER_FAILED;
    }
}
/*---------------------------------------------------------------------------*/
static void
input(void)
{
  NETSTACK_NETWORK.input();
}
/*---------------------------------------------------------------------------*/
static int
length(void)
{
  return framer_802154.length() + sizeof(mic);
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  ECRYPT_keysetup(&ctx, key, KEY_BIT_SIZE, IV_BIT_SIZE);
}
/*---------------------------------------------------------------------------*/
const struct llsec_driver simplesec_driver = {
  "simplesec",
  init,
  send,
  input
};
/*---------------------------------------------------------------------------*/
const struct framer simplesec_framer = {
  length,
  create,
  parse
};
/*---------------------------------------------------------------------------*/
