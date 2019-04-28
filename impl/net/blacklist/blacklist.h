/**
 * @file
 *         Blacklist handling
 *
 * @author
 *         Leo Krueger <leo.krueger@tuhh.de
 *
 */

#ifndef SFSB_BLACKLIST_H_
#define SFSB_BLACKLIST_H_

#include "net/mac/tsch/tsch-queue.h"
#include "sys/ctimer.h"
#include <stdbool.h>

#include "../blacklist/neighbors.h"

// unit s
#define SFSB_BLACKLIST_LIFETIME_DEFAULT (60 * 10) // 10 min.

struct sfsb_bl_entry {
	struct sfsb_bl_entry *next;
	uint8_t channel;
        clock_time_t inserted;
        clock_time_t updated;
        struct ctimer timer;
        list_t list; // TODO: dirty workaround
        struct memb *mem; // TODO: dirty workaround
};

uint8_t sfsb_bl_check(uint8_t current_channel,
                      linkaddr_t *addr,
                      bool isTx);


struct sfsb_bl_entry *sfsb_bl_add_update(uint8_t channel);
struct sfsb_bl_entry *sfsb_bl_generic_add_update(uint8_t channel,
                                                 uint32_t time_compensation,
                                                 list_t list, struct memb *m);
struct sfsb_bl_entry *sfsb_bl_get(uint8_t channel);
struct sfsb_bl_entry *sfsb_bl_generic_get(uint8_t channel, list_t list);
void sfsb_bl_remove(struct sfsb_bl_entry *entry);
void sfsb_bl_generic_remove(struct sfsb_bl_entry *entry, list_t list, struct memb *m);

void sfsb_bl_update(struct sfsb_bl_entry *entry, uint32_t time_compensation);
uint8_t sfsb_bl_pack(uint8_t *buf, uint8_t maxlen);
void sfsb_bl_unpack(uint8_t *buf, uint8_t len, struct sfsb_nbr *neigh);
bool sfsb_bl_updated(clock_time_t since);
void sfsb_bl_notify_process();

void sfsb_bl_print();
void sfsb_bl_set_lifetime(int lifetime);

void sfsb_bl_init(void);


#endif /* SFSB_BLACKLIST_H_ */
