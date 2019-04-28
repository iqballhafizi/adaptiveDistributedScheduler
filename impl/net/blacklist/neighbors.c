/**
 * @file
 *         SF neighbor management
 *
 * @author
 *         Leo Krueger <leo.krueger@tuhh.de>
 *
 */

#include "../blacklist/neighbors.h"

#include "contiki-lib.h"
#include <stdbool.h>

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"
#include "sys/ctimer.h"
#include "lib/memb.h"
#include "net/mac/tsch/tsch-queue.h"

MEMB(neighbors, struct sfsb_nbr, 10);
LIST(neighbor_list);

void
sfsb_nbr_init(void)
{
  PRINTF("neighbor-mgmt: init\n");

  list_init(neighbor_list);
}

struct sfsb_nbr*
sfsb_nbr_add(linkaddr_t addr)
{
  struct sfsb_nbr *neigh;

  neigh = memb_alloc(&neighbors);
  if (neigh == NULL) {
    return NULL;
  }

  neigh->addr = addr;
  neigh->notified = 0;
  LIST_STRUCT_INIT(neigh, blacklist);
  list_add(neighbor_list, neigh);

  return neigh;
}

bool
sfsb_nbr_remove(struct sfsb_nbr *neigh)
{
  list_remove(neighbor_list, neigh);
  ctimer_stop(&neigh->timer);

  memb_free(&neighbors, neigh);

  return true;
}

struct sfsb_nbr*
sfsb_nbr_get(linkaddr_t addr)
{
  struct sfsb_nbr *neigh;

  for(neigh = list_head(neighbor_list);
      neigh != NULL;
      neigh = list_item_next(neigh)) {
    if (linkaddr_cmp(&neigh->addr, &addr) > 0) {
      return neigh;
    }
  }

  return NULL;
}

uint8_t
sfsb_nbr_num_dedi_links(struct sfsb_nbr *neigh)
{
  struct tsch_neighbor *tsch_neigh;

  tsch_neigh = tsch_queue_get_nbr(&neigh->addr);

  if (tsch_neigh != NULL) {
    return tsch_neigh->dedicated_tx_links_count;
  }

  return 0;
}

uint8_t
sfsb_nbr_num_links(struct sfsb_nbr *neigh)
{
  struct tsch_neighbor *tsch_neigh;

  tsch_neigh = tsch_queue_get_nbr(&neigh->addr);

  if (tsch_neigh != NULL) {
    return tsch_neigh->tx_links_count;
  }

  return 0;
}

void
sfsb_nbr_do_per_nbr(void(*f)(void *ptr))
{
  struct sfsb_nbr *neigh;

  for(neigh = list_head(neighbor_list);
      neigh != NULL;
      neigh = list_item_next(neigh)) {
    f((void *) &neigh->addr);
  }
}
