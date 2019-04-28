/**
 * @file
 *         Blacklist handling
 *
 * @author
 *         Leo Krueger <leo.krueger@tuhh.de>
 *
 */

#include "../blacklist/blacklist.h"

#include "contiki-lib.h"
#include <stdbool.h>
#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"
#include "sys/ctimer.h"
#include "net/mac/tsch/sixtop/sixp-pkt.h"
#include "net/mac/tsch/sixtop/sixp.h"
#include "net/mac/tsch/tsch-private.h"
#include "../blacklist/sfsb.h"
#include "net/linkaddr.h"

#define CHANNEL_SELECTION_VIRTENIO 0

static int sfsb_blacklist_lifetime = SFSB_BLACKLIST_LIFETIME_DEFAULT;

void sfsb_bl_callback_delete(void *ptr);
static inline bool sfsb_bl_check_helper(uint8_t current_channel, struct sfsb_nbr *neigh);

MEMB(mem_bl_entries, struct sfsb_bl_entry, 16);
MEMB(mem_nbr_bl_ent, struct sfsb_bl_entry, 32);
LIST(private_blacklist);

PROCESS(p_sfsb_bl_distribution, "bl_distribution");

void
sfsb_bl_init(void)
{
  PRINTF("sfsb blacklist: init\n");

  list_init(private_blacklist);
  process_start(&p_sfsb_bl_distribution, NULL);
}

uint8_t
sfsb_bl_check(uint8_t current_channel,
              linkaddr_t *addr,
              bool isTx)
{
  // NOTE: PRINTF is in this function not allowed, because called by TSCH slot handling!
  struct sfsb_nbr *neigh;
  uint8_t i = 0, channel = current_channel;

  neigh = sfsb_nbr_get(*addr);
  if (neigh == NULL) { // as we do not know this neighbor, we do not blacklist
    return current_channel;
  }

#if CHANNEL_SELECTION_VIRTENIO
  // VIRTENIO: alternative Implementation: iterate through the hopping sequence and use first not blacklisted channel
  // pseudo-randomize by adding the current channel
  for (i=0; i <= tsch_hopping_sequence_length.val; i++){
      if (!sfsb_bl_check_helper(channel, neigh)) {
          return channel;
      }
      channel = tsch_hopping_sequence[(i + current_channel + 1) % tsch_hopping_sequence_length.val];
  }
#else
  // Implementation by the TUHH
  for(; i < 3; i++) {
    if (!sfsb_bl_check_helper(channel, neigh)) {
      return channel;
    } else {
      channel -= 11;
      channel += CHANNEL_SWAP_OFFSET;
      channel %= 16;
      channel += 11;
    }
  }
#endif

  return 0xFF;
}

static inline bool
sfsb_bl_check_helper(uint8_t current_channel, struct sfsb_nbr *neigh)
{
  struct sfsb_bl_entry *entry;

  entry = sfsb_bl_get(current_channel);
  if (entry != NULL) {  // we have blacklisted the channel
    // does our neighbor have up-to-date infos from us?
    if (neigh->notified > entry->updated) { // TODO: enough? what about 'inserted'?
      return true;
    }
  }

  // is the channel in the blacklist we got from our neighbor?
  if (sfsb_bl_generic_get(current_channel, neigh->blacklist)) {
    return true;
  }

  return false;
}

struct sfsb_bl_entry*
sfsb_bl_add_update(uint8_t channel)
{
  return sfsb_bl_generic_add_update(channel, 0, private_blacklist, &mem_bl_entries);
}

struct sfsb_bl_entry*
sfsb_bl_generic_add_update(uint8_t channel,
                           uint32_t time_compensation,
                           list_t list, struct memb *m)
{
  struct sfsb_bl_entry *entry;

  entry = sfsb_bl_get(channel);
  if (entry != NULL) { // update
    sfsb_bl_update(entry, time_compensation);
  } else { // add
    PRINTF("sfsb blacklist: adding entry for channel %i\n", (int)channel);

    entry = memb_alloc(m);
    if (entry == NULL) {
      return NULL;
    }

    entry->channel = channel;
    entry->inserted = clock_time(); // now
    entry->updated = clock_time(); // now
    entry->list = list;
    entry->mem = m;

    ctimer_set(&entry->timer, (CLOCK_SECOND * sfsb_blacklist_lifetime)
               - ((CLOCK_SECOND / 100) * (time_compensation / 10)),
               sfsb_bl_callback_delete, entry);

    list_add(list, entry);
  }

  return entry;
}

struct sfsb_bl_entry*
sfsb_bl_get(uint8_t channel)
{
  return sfsb_bl_generic_get(channel, private_blacklist);
}

struct sfsb_bl_entry*
sfsb_bl_generic_get(uint8_t channel, list_t list)
{
  struct sfsb_bl_entry *entry;

  for(entry = list_head(list);
      entry != NULL;
      entry = list_item_next(entry)) {
    if (entry->channel == channel) {
      return entry;
    }
  }

  return NULL;
}

void
sfsb_bl_update(struct sfsb_bl_entry *entry, uint32_t time_compensation)
{
  PRINTF("sfsb blacklist: updating entry for channel %" PRIu8 "\n", entry->channel);

  entry->updated = clock_time(); // now

  ctimer_stop(&entry->timer);
  ctimer_set(&entry->timer, (CLOCK_SECOND * sfsb_blacklist_lifetime)
             - ((CLOCK_SECOND / 100) * (time_compensation / 10)),
             sfsb_bl_callback_delete, entry);
}

void
sfsb_bl_remove(struct sfsb_bl_entry *entry)
{
  sfsb_bl_generic_remove(entry, private_blacklist, &mem_bl_entries);
}

void
sfsb_bl_generic_remove(struct sfsb_bl_entry *entry, list_t list, struct memb *m)
{
  PRINTF("sfsb blacklist: deleting entry for channel %" PRIu8 "\n", entry->channel);
  list_remove(list, entry);
  ctimer_stop(&entry->timer);

  memb_free(m, entry);
}

void
sfsb_bl_callback_delete(void *ptr)
{
  struct sfsb_bl_entry *entry = (struct sfsb_bl_entry *) ptr;
  sfsb_bl_generic_remove(entry, entry->list, entry->mem);
}

uint8_t
sfsb_bl_pack(uint8_t *buf, uint8_t maxlen)
{
  // 'period' and 'start' left out for now
  bool end = false;
  struct sfsb_bl_entry *entry;
  uint8_t len = 0;
  struct tsch_asn_t asn = tsch_current_asn;

  /* ATTENTION: different than in protocol description
                         1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|E|P| unused  |entry 1 | entry 2 | ...
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */

  buf[0] = end? 0x40 : 0x0;
  len += 1;

  // include current asn as time reference
  buf[1] = asn.ls4b & 0xFF;
  buf[2] = (asn.ls4b >> 8) & 0xFF;
  buf[3] = (asn.ls4b >> 16) & 0xFF;
  buf[4] = (asn.ls4b >> 24) & 0xFF;
  buf[5] = asn.ms1b;
  len += 5;

  /* ATTENTION: different than in protocol description
                         1                   2
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |channel| unused|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */

  for(entry = list_head(private_blacklist);
      entry != NULL;
      entry = list_item_next(entry)) {
    buf[len] = entry->channel;
    len++;
  }

  return len;
}

void sfsb_bl_unpack(uint8_t *buf, uint8_t len, struct sfsb_nbr *neigh)
{
  bool start, end, period;
  uint8_t length_per_entry = 1;
  uint8_t e_channel; //, e_start, e_end, e_period;
  struct tsch_asn_t asn;
  uint32_t late_time = 0;

  // at least one byte for options
  if (len < 1) {
    return;
  }

  start = buf[0] & 0x80? true : false;
  end = buf[0] & 0x40? true : false;
  period = buf[0] & 0x20? true : false;

  asn.ls4b = buf[1];
  asn.ls4b |= ((uint32_t) buf[2] << 8);
  asn.ls4b |= ((uint32_t) buf[3] << 16);
  asn.ls4b |= ((uint32_t) buf[4] << 24);
  asn.ms1b = buf[5];

  late_time =  TSCH_ASN_DIFF(tsch_current_asn, asn) * (TSCH_CONF_DEFAULT_TIMESLOT_LENGTH / 1000);

  PRINTF("sfsb blacklist: received blacklist %" PRIu32 " ms late\n", late_time);

  if (start) length_per_entry++;
  if (end) length_per_entry++;
  if (period) length_per_entry++;

//PRINTF("sfsb blacklist: got blacklist from ");
//net_debug_lladdr_print((const uip_lladdr_t *)&neigh->addr);
//PRINTF(" with options 0x%x and channels ", buf[0]);

  for (uint8_t i = 6; i < len; i+=length_per_entry) {
    e_channel = buf[i];
//    if (start) e_start = buf[i+1];
//    if (end) e_end = buf[i+2];
//    if (period) e_period = buf[i+3];

//  PRINTF("%" PRIu8 " ", e_channel);
    sfsb_bl_generic_add_update(e_channel, late_time, neigh->blacklist, &mem_nbr_bl_ent); 
  }

//PRINTF("\n");
}

bool
sfsb_bl_updated(clock_time_t since)
{
  struct sfsb_bl_entry *entry;

  for(entry = list_head(private_blacklist);
      entry != NULL;
      entry = list_item_next(entry)) {
    if (entry->inserted > since || entry->updated > since) {
      return true; 
    }
  }
  
  return false;
}

PROCESS_THREAD(p_sfsb_bl_distribution, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("sfsb blacklist: distribution process started\n");

  while(true) {
    PROCESS_YIELD();
    sfsb_nbr_do_per_nbr(sfsb_bl_send); 
  }

  PROCESS_END();
}

void
sfsb_bl_notify_process()
{
  process_post(&p_sfsb_bl_distribution, PROCESS_EVENT_CONTINUE, NULL);
}


static void
sfsb_bl_print_neighbor(void *ptr)
{
    linkaddr_t *peer_addr = (linkaddr_t *) ptr;
    struct sfsb_nbr *neigh = sfsb_nbr_get(*peer_addr);
    struct sfsb_bl_entry *entry;
    PRINTF("#\n");
    PRINTF("NBR link address: ");
    net_debug_lladdr_print((const uip_lladdr_t *)&neigh->addr);
    PRINTF("\n");
    for(entry = list_head(neigh->blacklist); entry != NULL; entry = list_item_next(entry)) {
        PRINTF("Channel: %i\n", entry->channel);
        PRINTF("Insertion timestamp: %lu\n", entry->inserted);
        PRINTF("Updated timestamp: %lu\n", entry->updated);
        PRINTF("Blacklist timeout timestamp: %lu\n", etimer_expiration_time(&entry->timer.etimer));
    }
    PRINTF("#\n");
}

void
sfsb_bl_set_lifetime(int lifetime){
  if (lifetime > 0 && lifetime < (60 * 60)){ // Limit between 1s and 1 hour
      PRINTF("Setting blacklist lifetime to %i [s].\n", lifetime);
      sfsb_blacklist_lifetime = lifetime;
  }
}


// Print the blacklist
void
sfsb_bl_print()
{
  struct sfsb_bl_entry *entry;

  printf("\n--\n");
  printf("Private blacklist\n");
  printf("--\n");
  for(entry = list_head(private_blacklist); entry != NULL; entry = list_item_next(entry)) {
      printf("Channel: %i\n", entry->channel);
      printf("Insertion timestamp: %lu\n", entry->inserted);
      printf("Updated timestamp: %lu\n", entry->updated);
      printf("Blacklist timeout timestamp: %lu\n", etimer_expiration_time(&entry->timer.etimer));
  }
  printf("--\n");
  printf("Neighbor blacklist\n");
  printf("--\n");
  sfsb_nbr_do_per_nbr(sfsb_bl_print_neighbor);
  printf("--\n");
}
