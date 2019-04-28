
#ifndef SF_ADAPTIVE_H_
#define SF_ADAPTIVE_H_

#include "net/mac/tsch/tsch-queue.h"

void sf_adaptive_init();
uint8_t sf_adaptive_parent_queue_size();
void sf_adaptive_run();

#endif
