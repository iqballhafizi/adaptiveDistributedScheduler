/**
 * @file
 *         SF neighbor management
 *
 * @author
 *         Leo Krueger <leo.krueger@tuhh.de
 *
 */

#ifndef SFNEIGH_H_
#define SFNEIGH_H_


#include <stdbool.h>
#include "lib/list.h"
#include "net/linkaddr.h"
#include "sys/ctimer.h"

struct sfsb_nbr {
  struct sfsb_nbr *next;
  linkaddr_t addr;
  clock_time_t notified;
  struct ctimer timer;
  LIST_STRUCT(blacklist);
};

struct sfsb_nbr *sfsb_nbr_add(linkaddr_t addr);
bool sfsb_nbr_remove(struct sfsb_nbr *neigh);
struct sfsb_nbr *sfsb_nbr_get(linkaddr_t addr);
uint8_t sfsb_nbr_num_dedi_links(struct sfsb_nbr *neigh);
uint8_t sfsb_nbr_num_links(struct sfsb_nbr *neigh); 
void sfsb_nbr_do_per_nbr(void(*f)(void *ptr));
void sfsb_nbr_init(void);


#endif /* SFNEIGH_H_ */
