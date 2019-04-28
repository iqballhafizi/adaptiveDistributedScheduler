

#include "contiki-lib.h"

#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "../blacklist/sfsb.h"


PROCESS(sf_adaptive, "sf adaptive process");

void sf_adaptive_init(){
    printf("Adaptive: initialized");
    process_start(&sf_adaptive, NULL);
}


uint8_t sf_adaptive_parent_queue_size(){
    struct tsch_neighbor *n = tsch_queue_get_time_source();
    uint8_t size = NULL;
    if(n != NULL){
         size = tsch_queue_packet_count(&n->addr);
         printf("Adaptive: Parent Queue Size %d \n", size);
    }
    return size;
}

void
sf_adaptive_run()
{
  process_post(&sf_adaptive, PROCESS_EVENT_CONTINUE, NULL);
}


 
 PROCESS_THREAD(sf_adaptive, ev, data)
 {
   PROCESS_BEGIN();
 
    static uint8_t queue_50 = 0;
    static uint8_t queue_65 = 0;
    static uint8_t queue_80 = 0;

   while(1) {
     PROCESS_WAIT_EVENT();
     printf("Adaptive: Got event number %d\n queue_50: %d", ev, queue_50);
     uint8_t size = sf_adaptive_parent_queue_size();

     if(size >= 13 ){
            // at queue 80% full
  	        printf("Adaptive: Queue 80\% Full, ask for 3 cells\n");
            if(queue_80 == 0){
                queue_80 = 1; 

                //ask for 3 cells
                struct tsch_neighbor *n = tsch_queue_get_time_source();
                printf("Adaptive: Queue 80\% Full, ask for 3 cells\n");
                sfsb_add_links(&n->addr, 3);
            }
     } 
     else if(size >= 10){
            // at queue 65% full
            printf("Adaptive: Queue 65\% Full, ask for 2 cells\n");
            if(queue_65 == 0){
                queue_65 = 1; 

                //ask for 2 cells
                struct tsch_neighbor *n = tsch_queue_get_time_source();
                printf("Adaptive: Queue 65\% Full, ask for 2 cells\n");
                sfsb_add_links(&n->addr, 2);
            }
     }
     else if(size >= 8){
            // at queue 50% full
            printf("Adaptive: Queue 50\% Full, ask for 1 cell\n");
            if(queue_50 == 0){
                queue_50 = 1; 

                //ask for 1 cells
                struct tsch_neighbor *n = tsch_queue_get_time_source();
                printf("Adaptive: Queue 50\% Full, ask for 1 cell\n");
                sfsb_add_links(&n->addr, 1);
            }
     }
   }
 
   PROCESS_END();
 }







