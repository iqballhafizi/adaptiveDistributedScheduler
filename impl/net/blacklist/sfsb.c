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

#include "../blacklist/sfsb.h"

#include "contiki-lib.h"

#include "lib/assert.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixp.h"
#include "net/mac/tsch/sixtop/sixp-nbr.h"
#include "net/mac/tsch/sixtop/sixp-pkt.h"
#include "net/mac/tsch/sixtop/sixp-trans.h"
#include <stdbool.h>

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"
#include "sys/ctimer.h"

#include "../blacklist/neighbors.h"
#include "../blacklist/blacklist.h"

#include "../blacklist/sf-adaptive.h"

typedef struct {
  uint16_t timeslot_offset;
  uint16_t channel_offset;
} sfsb_cell_t;

/**typedef struct {
  sfsb_neigh_t* next;
  linkaddr_t addr;
  struct ctimer timer;
} sfsb_neigh_t;**/

bool already_init = false;
static const uint16_t slotframe_handle = 0;
static uint8_t res_storage[4 + SF_SIMPLE_MAX_LINKS * 4];
static uint8_t req_storage[4 + SF_SIMPLE_MAX_LINKS * 4];

static void read_cell(const uint8_t *buf, sfsb_cell_t *cell);
static void print_cell_list(const uint8_t *cell_list, uint16_t cell_list_len);
static void add_links_to_schedule(const linkaddr_t *peer_addr,
                                  uint8_t link_option,
                                  const uint8_t *cell_list,
                                  uint16_t cell_list_len);
static void remove_links_to_schedule(const uint8_t *cell_list,
                                     uint16_t cell_list_len);
static void add_response_sent_callback(void *arg, uint16_t arg_len,
                                       const linkaddr_t *dest_addr,
                                       sixp_output_status_t status);
static void delete_response_sent_callback(void *arg, uint16_t arg_len,
                                          const linkaddr_t *dest_addr,
                                          sixp_output_status_t status);
static void signal_request_sent_callback(void *arg, uint16_t arg_len,
                                          const linkaddr_t *dest_addr,
                                          sixp_output_status_t status);
static void add_req_input(const uint8_t *body, uint16_t body_len,
                          const linkaddr_t *peer_addr);
static void delete_req_input(const uint8_t *body, uint16_t body_len,
                             const linkaddr_t *peer_addr);
static void signal_req_input(const uint8_t *body, uint16_t body_len,
                             const linkaddr_t *peer_addr);
static void init();
static void input(sixp_pkt_type_t type, sixp_pkt_code_t code,
                  const uint8_t *body, uint16_t body_len,
                  const linkaddr_t *src_addr);
static void request_input(sixp_pkt_cmd_t cmd,
                          const uint8_t *body, uint16_t body_len,
                          const linkaddr_t *peer_addr);
static void response_input(sixp_pkt_rc_t rc,
                           const uint8_t *body, uint16_t body_len,
                           const linkaddr_t *peer_addr);
static void callbackLinksAdded(void *ptr);


void
sfsb_new_nbr(linkaddr_t addr)
{
  struct sfsb_nbr *neigh = sfsb_nbr_get(addr);

  if (neigh == NULL) {
    PRINTF("sfsb: got notified of new nbr: ");
    net_debug_lladdr_print((const uip_lladdr_t *)&addr);
    PRINTF("\n");

    neigh = sfsb_nbr_add(addr);

    if (neigh == NULL) {
      return;
    }
  }

  // despite knowing the neighbor already, we got notified
  // so better verify we have cells scheduled with that neighbor
  callbackLinksAdded(neigh);
}

static void
callbackLinksAdded(void *ptr)
{
  struct sfsb_nbr *neigh = (struct sfsb_nbr *) ptr;
  uint16_t interval = SFSB_MAX_RETRY_BACKOFF - 60;

  PRINTF("sfsb: nbr ");
  net_debug_lladdr_print((const uip_lladdr_t *)&neigh->addr);
  PRINTF(" has %u dedicated cells scheduled\n", sfsb_nbr_num_dedi_links(neigh));

  if (sfsb_nbr_num_dedi_links(neigh) < SFSB_INIT_NUM_CELLS) {
    if (!tsch_is_associated) {
      // we are not associated, quickly try again
      ctimer_set(&neigh->timer, CLOCK_SECOND, callbackLinksAdded, neigh);
      return;
    }

    if (sixp_trans_find(&neigh->addr) != NULL) {
      // there is another transaction running, backoff up to half the timeout value
      ctimer_set(&neigh->timer, CLOCK_SECOND * ((random_rand() % 0xFFFF) % (60 >> 1)), callbackLinksAdded, neigh);
      return;
    }

    // asssociated and no transaction running, start transaction
    sfsb_add_links(&neigh->addr, SFSB_INIT_NUM_CELLS - sfsb_nbr_num_dedi_links(neigh));
    ctimer_set(&neigh->timer, CLOCK_SECOND * (60 + (((random_rand() % 0xFFFF)) % interval)), callbackLinksAdded, neigh);
  }
}


/*
 * scheduling policy:
 * add: if and only if all the requested cells are available, accept the request
 * delete: if and only if all the requested cells are in use, accept the request
 */

static void
read_cell(const uint8_t *buf, sfsb_cell_t *cell)
{
  cell->timeslot_offset = buf[0] + (buf[1] << 8);
  cell->channel_offset = buf[2] + (buf[3] << 8);
}

static void
print_cell_list(const uint8_t *cell_list, uint16_t cell_list_len)
{
  uint16_t i;
  sfsb_cell_t cell;

  for(i = 0; i < cell_list_len; i += sizeof(cell)) {
    read_cell(&cell_list[i], &cell);
    PRINTF("%u:%u ", cell.timeslot_offset, cell.channel_offset);
  }
}

static void
add_links_to_schedule(const linkaddr_t *peer_addr, uint8_t link_option,
                      const uint8_t *cell_list, uint16_t cell_list_len)
{
  sfsb_cell_t cell;
  struct tsch_slotframe *slotframe;
  int i;

  assert(cell_list != NULL);

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);

  if(slotframe == NULL) {
    return;
  }

  for(i = 0; i < cell_list_len; i += sizeof(cell)) {
    read_cell(&cell_list[i], &cell);
    if(cell.timeslot_offset == 0xffff) {
      continue;
    }

    PRINTF("sfsb: Schedule cell %d:%d as %s with node ",
           cell.timeslot_offset, cell.channel_offset,
           link_option == LINK_OPTION_RX ? "RX" : "TX");
    net_debug_lladdr_print((const uip_lladdr_t *) peer_addr);
    PRINTF("\n");
    tsch_schedule_add_link(slotframe,
                           link_option, LINK_TYPE_NORMAL, peer_addr,
                           cell.timeslot_offset, cell.channel_offset);
  }
}

static void
remove_links_to_schedule(const uint8_t *cell_list, uint16_t cell_list_len)
{
  /* remove all the cells */

  sfsb_cell_t cell;
  struct tsch_slotframe *slotframe;
  int i;

  assert(cell_list != NULL);

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);

  if(slotframe == NULL) {
    return;
  }

  for(i = 0; i < cell_list_len; i += sizeof(cell)) {
    read_cell(&cell_list[i], &cell);
    if(cell.timeslot_offset == 0xffff) {
      continue;
    }

    tsch_schedule_remove_link_by_timeslot(slotframe,
                                          cell.timeslot_offset);
  }
}

static void
add_response_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_output_status_t status)
{
  uint8_t *body = (uint8_t *)arg;
  uint16_t body_len = arg_len;
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  sixp_nbr_t *nbr;

  assert(body != NULL && dest_addr != NULL);

  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
     sixp_pkt_get_cell_list(SIXP_PKT_TYPE_RESPONSE,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                            &cell_list, &cell_list_len,
                            body, body_len) == 0 &&
     (nbr = sixp_nbr_find(dest_addr)) != NULL) {
    add_links_to_schedule(dest_addr, LINK_OPTION_RX,
                          cell_list, cell_list_len);
  }
}

static void
delete_response_sent_callback(void *arg, uint16_t arg_len,
                              const linkaddr_t *dest_addr,
                              sixp_output_status_t status)
{
  uint8_t *body = (uint8_t *)arg;
  uint16_t body_len = arg_len;
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  sixp_nbr_t *nbr;

  assert(body != NULL && dest_addr != NULL);

  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
     sixp_pkt_get_cell_list(SIXP_PKT_TYPE_RESPONSE,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                            &cell_list, &cell_list_len,
                            body, body_len) == 0 &&
     (nbr = sixp_nbr_find(dest_addr)) != NULL) {
    remove_links_to_schedule(cell_list, cell_list_len);
  }
}

static void
add_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{
  uint8_t i;
  sfsb_cell_t cell;
  struct tsch_slotframe *slotframe;
  int feasible_link;
  uint8_t num_cells;
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  uint16_t res_len;

  assert(body != NULL && peer_addr != NULL);

  if(sixp_pkt_get_num_cells(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                            &num_cells,
                            body, body_len) != 0 ||
     sixp_pkt_get_cell_list(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                            &cell_list, &cell_list_len,
                            body, body_len) != 0) {
    PRINTF("sfsb: Parse error on add request\n");
    return;
  }

  PRINTF("sfsb: Received a 6P Add Request for %d cells from node ",
         num_cells);
  net_debug_lladdr_print((const uip_lladdr_t *) peer_addr);
  PRINTF(" with cell list: ");
  print_cell_list(cell_list, cell_list_len);
  PRINTF("\n");

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  if(slotframe == NULL) {
    return;
  }

  if(num_cells > 0 && cell_list_len > 0) {
    memset(res_storage, 0, sizeof(res_storage));
    res_len = 0;

    /* checking availability for requested slots */
    for(i = 0, feasible_link = 0;
        i < cell_list_len && feasible_link < num_cells;
        i += sizeof(cell)) {
      read_cell(&cell_list[i], &cell);
      if(tsch_schedule_get_link_by_timeslot(slotframe,
                                            cell.timeslot_offset) == NULL) {
        sixp_pkt_set_cell_list(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               (uint8_t *)&cell, sizeof(cell),
                               feasible_link*sizeof(cell),
                               res_storage, sizeof(res_storage));
        res_len += sizeof(cell);
        feasible_link++;
      }
    }

    if(feasible_link == num_cells) {
      /* Links are feasible. Create Link Response packet */
      PRINTF("sfsb: Send a 6P Response to node ");
      net_debug_lladdr_print((const uip_lladdr_t *) peer_addr);
      PRINTF("\n");

      sixp_output(SIXP_PKT_TYPE_RESPONSE,
                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                  SF_SIMPLE_SFID,
                  res_storage, res_len, peer_addr,
                  add_response_sent_callback, res_storage, res_len);
    }
  }
}

static void
delete_req_input(const uint8_t *body, uint16_t body_len,
                 const linkaddr_t *peer_addr)
{
  uint8_t i;
  sfsb_cell_t cell;
  struct tsch_slotframe *slotframe;
  uint8_t num_cells;
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  uint16_t res_len;
  int removed_link;

  assert(body != NULL && peer_addr != NULL);

  if(sixp_pkt_get_num_cells(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                            &num_cells,
                            body, body_len) != 0 ||
     sixp_pkt_get_cell_list(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                            &cell_list, &cell_list_len,
                            body, body_len) != 0) {
    PRINTF("sfsb: Parse error on delete request\n");
    return;
  }

  PRINTF("sfsb: Received a 6P Delete Request for %d cells from node ",
         num_cells);
  net_debug_lladdr_print((const uip_lladdr_t *) peer_addr);
  PRINTF(" with cell list ");
  print_cell_list(cell_list, cell_list_len);
  PRINTF("\n");

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  if(slotframe == NULL) {
    return;
  }

  memset(res_storage, 0, sizeof(res_storage));
  res_len = 0;

  if(num_cells > 0 && cell_list_len > 0) {
    /* ensure before delete */
    for(i = 0, removed_link = 0; i < cell_list_len; i += sizeof(cell)) {
      read_cell(&cell_list[i], &cell);
      if(tsch_schedule_get_link_by_timeslot(slotframe,
                                            cell.timeslot_offset) != NULL) {
        sixp_pkt_set_cell_list(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               (uint8_t *)&cell, sizeof(cell),
                               removed_link,
                               res_storage, sizeof(res_storage));
        res_len += sizeof(cell);
      }
    }
  }

  /* Links are feasible. Create Link Response packet */
  PRINTF("sfsb: Send a 6P Response to node ");
  net_debug_lladdr_print((const uip_lladdr_t *) peer_addr);
  PRINTF("\n");

  sixp_output(SIXP_PKT_TYPE_RESPONSE,
              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
              SF_SIMPLE_SFID,
              res_storage, res_len, peer_addr,
              delete_response_sent_callback, res_storage, res_len);
}

static void
signal_req_input(const uint8_t *body, uint16_t body_len,
                 const linkaddr_t *peer_addr)
{
  struct sfsb_nbr *neigh;

  if(sixp_pkt_get_payload(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_SIGNAL,
                          res_storage, sizeof(res_storage),
                          body, body_len) != 0) {
    PRINTF("sfsb: parse error on SIGNAL\n");
    return;
  }

  neigh = sfsb_nbr_get(*peer_addr);
  if (neigh == NULL) {
    PRINTF("sfsb: got SIGNAL from unknown neighbor\n");
    return;
  }

  // be sure to substract the size of the metadata field
  // which belongs to the payload of the 6P SIGNAL request pkt
  sfsb_bl_unpack(res_storage, body_len - sizeof(sixp_pkt_metadata_t), neigh);
  
}

void
sfsb_bl_send(void *ptr)
{
  uint8_t buf[11];
  uint8_t len = 0;
  linkaddr_t *peer_addr = (linkaddr_t *) ptr;
  struct sfsb_nbr *neigh = sfsb_nbr_get(*peer_addr);

  if (neigh == NULL) {
    return;
  }

  // if initial transaction is not finished,
  // we do not send a blacklist to this neighbor
  if (sfsb_nbr_num_dedi_links(neigh) < SFSB_INIT_NUM_CELLS) {
      PRINTF("sfsb: distribute blacklist skipped, because initial transaction not yet finished.\n");
    return;
  }

  if (!sfsb_bl_updated(neigh->notified)) {
      PRINTF("sfsb: distribute blacklist skipped, because neighbor already notified.\n");
    return;
  }

  memset(req_storage, 0, sizeof(req_storage));
  len = sfsb_bl_pack(buf, 11);

  if(sixp_pkt_set_payload(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_SIGNAL,
                          buf, len,
                          req_storage, sizeof(req_storage)) != 0) {
    PRINTF("sfsb: build error on SIGNAL\n");
    return;
  }

  PRINTF("sfsb: sending SIGNAL to nbr at %lu\n", clock_time());
  sixp_output(SIXP_PKT_TYPE_REQUEST,
              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_SIGNAL,
              SF_SIMPLE_SFID,
              req_storage, len + sizeof(sixp_pkt_metadata_t), peer_addr,
              signal_request_sent_callback, neigh, 0);
}

static void
signal_request_sent_callback(void *arg, uint16_t arg_len,
                             const linkaddr_t *dest_addr,
                             sixp_output_status_t status)
{
  struct sfsb_nbr *neigh = (struct sfsb_nbr *) arg;
  PRINTF("sfsb: SIGNAL callback at %lu\n", clock_time());
  if(status == SIXP_OUTPUT_STATUS_SUCCESS) {
    neigh->notified = clock_time(); // now
  }
}

static void
input(sixp_pkt_type_t type, sixp_pkt_code_t code,
      const uint8_t *body, uint16_t body_len, const linkaddr_t *src_addr)
{
  assert(body != NULL && body != NULL);
  switch(type) {
    case SIXP_PKT_TYPE_REQUEST:
      request_input(code.cmd, body, body_len, src_addr);
      break;
    case SIXP_PKT_TYPE_RESPONSE:
      response_input(code.cmd, body, body_len, src_addr);
      break;
    default:
      /* unsupported */
      break;
  }
}

static void
request_input(sixp_pkt_cmd_t cmd,
              const uint8_t *body, uint16_t body_len,
              const linkaddr_t *peer_addr)
{
  assert(body != NULL && peer_addr != NULL);

  switch(cmd) {
    case SIXP_PKT_CMD_ADD:
      add_req_input(body, body_len, peer_addr);
      break;
    case SIXP_PKT_CMD_DELETE:
      delete_req_input(body, body_len, peer_addr);
      break;
    case SIXP_PKT_CMD_SIGNAL:
      signal_req_input(body, body_len, peer_addr);
      break;
    default:
      /* unsupported request */
      break;
  }
}
static void
response_input(sixp_pkt_rc_t rc,
               const uint8_t *body, uint16_t body_len,
               const linkaddr_t *peer_addr)
{
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  sixp_nbr_t *nbr;
  sixp_trans_t *trans;

  assert(body != NULL && peer_addr != NULL);

  if((nbr = sixp_nbr_find(peer_addr)) == NULL ||
     (trans = sixp_trans_find(peer_addr)) == NULL) {
    return;
  }

  if(rc == SIXP_PKT_RC_SUCCESS) {
    switch(sixp_trans_get_cmd(trans)) {
      case SIXP_PKT_CMD_ADD:
        if(sixp_pkt_get_cell_list(SIXP_PKT_TYPE_RESPONSE,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                  &cell_list, &cell_list_len,
                                  body, body_len) != 0) {
          PRINTF("sfsb: Parse error on add response\n");
          return;
        }
        PRINTF("sfsb: Received a 6P Add Response with cell list: ");
        print_cell_list(cell_list, cell_list_len);
        PRINTF("\n");
        add_links_to_schedule(peer_addr, LINK_OPTION_TX,
                              cell_list, cell_list_len);
        break;
      case SIXP_PKT_CMD_DELETE:
        if(sixp_pkt_get_cell_list(SIXP_PKT_TYPE_RESPONSE,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                  &cell_list, &cell_list_len,
                                  body, body_len) != 0) {
          PRINTF("sfsb: Parse error on add response\n");
          return;
        }
        PRINTF("sfsb: Received a 6P Delete Response with cell list: ");
        print_cell_list(cell_list, cell_list_len);
        PRINTF("\n");
        remove_links_to_schedule(cell_list, cell_list_len);
        break;
      case SIXP_PKT_CMD_COUNT:
      case SIXP_PKT_CMD_LIST:
      case SIXP_PKT_CMD_CLEAR:
      default:
        PRINTF("sfsb: unsupported response\n");
    }
  }
}

/*---------------------------------------------------------------------------*/
int
sfsb_add_links(linkaddr_t *peer_addr, uint8_t num_links)
{
  uint8_t i = 0, index = 0;
  struct tsch_slotframe *sf =
    tsch_schedule_get_slotframe_by_handle(slotframe_handle);

  uint8_t req_len;
  sfsb_cell_t cell_list[SF_SIMPLE_MAX_LINKS];

  /* Flag to prevent repeated slots */
  uint8_t slot_check = 1;
  uint16_t random_slot = 0;

  assert(peer_addr != NULL && sf != NULL);

  do {
    /* Randomly select a slot offset within TSCH_SCHEDULE_DEFAULT_LENGTH */
    random_slot = ((random_rand() & 0xFF)) % TSCH_SCHEDULE_DEFAULT_LENGTH;

    if(tsch_schedule_get_link_by_timeslot(sf, random_slot) == NULL) {

      /* To prevent repeated slots */
      for(i = 0; i < index; i++) {
        if(cell_list[i].timeslot_offset != random_slot) {
          /* Random selection resulted in a free slot */
          if(i == index - 1) { /* Checked till last index of link list */
            slot_check = 1;
            break;
          }
        } else {
          /* Slot already present in CandidateLinkList */
          slot_check++;
          break;
        }
      }

      /* Random selection resulted in a free slot, add it to linklist */
      if(slot_check == 1) {
        cell_list[index].timeslot_offset = random_slot;
        cell_list[index].channel_offset = ((random_rand() % 0xFF)) % 16;

        index++;
        slot_check++;
      } else if(slot_check > TSCH_SCHEDULE_DEFAULT_LENGTH) {
        PRINTF("sfsb:! Number of trials for free slot exceeded...\n");
        return -1;
        break; /* exit while loop */
      }
    }
  } while(index < SF_SIMPLE_MAX_LINKS);

  /* Create a Sixtop Add Request. Return 0 if Success */
  if(index == 0 ) {
    return -1;
  }

  memset(req_storage, 0, sizeof(req_storage));
  if(sixp_pkt_set_cell_options(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                               SIXP_PKT_CELL_OPTION_TX,
                               req_storage,
                               sizeof(req_storage)) != 0 ||
     sixp_pkt_set_num_cells(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                            num_links,
                            req_storage,
                            sizeof(req_storage)) != 0 ||
     sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                            (const uint8_t *)cell_list,
                            index * sizeof(sfsb_cell_t), 0,
                            req_storage, sizeof(req_storage)) != 0) {
    PRINTF("sfsb: Build error on add request\n");
    return -1;
  }

  /* The length of fixed part is 4 bytes: Metadata, CellOptions, and NumCells */
  req_len = 4 + index * sizeof(sfsb_cell_t);
  sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
              SF_SIMPLE_SFID,
              req_storage, req_len, peer_addr,
              NULL, NULL, 0);

  PRINTF("sfsb: Send a 6P Add Request for %d cells to node ", num_links);
  net_debug_lladdr_print((const uip_lladdr_t *) peer_addr);
  PRINTF(" with cell list: ");
  print_cell_list((const uint8_t *)cell_list, index * sizeof(sfsb_cell_t));
  PRINTF("\n");

  return 0;
}
/*---------------------------------------------------------------------------*/
/* Initiates a Sixtop Link deletion
 */
int
sfsb_remove_links(linkaddr_t *peer_addr)
{
  uint8_t i = 0, index = 0;
  struct tsch_slotframe *sf =
    tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  struct tsch_link *l;

  uint16_t req_len;
  sfsb_cell_t cell;

  assert(peer_addr != NULL && sf != NULL);

  for(i = 0; i < TSCH_SCHEDULE_DEFAULT_LENGTH; i++) {
    l = tsch_schedule_get_link_by_timeslot(sf, i);

    if(l) {
      /* Non-zero value indicates a scheduled link */
      if((linkaddr_cmp(&l->addr, peer_addr)) && (l->link_options == LINK_OPTION_TX)) {
        /* This link is scheduled as a TX link to the specified neighbor */
        cell.timeslot_offset = i;
        cell.channel_offset = l->channel_offset;
        index++;
        break;   /* delete atmost one */
      }
    }
  }

  if(index == 0) {
    return -1;
  }

  memset(req_storage, 0, sizeof(req_storage));
  if(sixp_pkt_set_num_cells(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                            1,
                            req_storage,
                            sizeof(req_storage)) != 0 ||
     sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                            (const uint8_t *)&cell, sizeof(cell),
                            0,
                            req_storage, sizeof(req_storage)) != 0) {
    PRINTF("sfsb: Build error on add request\n");
    return -1;
  }
  /* The length of fixed part is 4 bytes: Metadata, CellOptions, and NumCells */
  req_len = 4 + sizeof(sfsb_cell_t);

  sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
              SF_SIMPLE_SFID,
              req_storage, req_len, peer_addr,
              NULL, NULL, 0);

  PRINTF("sfsb: Send a 6P Delete Request for %d cells to node ", 1);
  net_debug_lladdr_print((const uip_lladdr_t *) peer_addr);
  PRINTF(" with cell list: ");
  print_cell_list((const uint8_t *)&cell, sizeof(cell));
  PRINTF("\n");

  return 0;
}

void init()
{
  if (!already_init) {
    sfsb_nbr_init();
    sfsb_bl_init();
    sf_adaptive_init();
    already_init = true;
  }
}

const sixtop_sf_t sfsb_driver = {
  SF_SIMPLE_SFID,
  CLOCK_SECOND*60,
  &init,
  input,
  NULL
};
