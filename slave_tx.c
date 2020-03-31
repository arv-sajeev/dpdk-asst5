#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <rte_eal.h>    // The dpdk environment abstraction layer
#include <rte_ethdev.h> //this is the rte ethernet device api library 
#include <rte_cycles.h> //time reference functions are here
#include <rte_lcore.h>  // lcore means logical core and it's some sort of multithreading library 
#include <rte_mbuf.h>   // the memory buffer rings and other satructures mempool as well 
#include <rte_kni.h>

#include "asst5.h"

int slave_tx_main(void *args_ptr)	{
        struct rte_mbuf* buffer[BURST_SIZE];
        uint16_t rx_sz,tx_sz,f;

        //Receive and typecast all args
        struct lcore_args* slave_tx_args = (struct lcore_args*)args_ptr;
        unsigned int port_id = slave_tx_args->port_id;
        struct rte_ring *ring_2  = slave_tx_args->ring_1;
        struct rte_kni *kni = slave_tx_args->kni;

        printf("\nIn slave_tx_main with :-\nPORT_ID - %d\nLCORE_ID - %d\nRTE_RING - %s\nKNI_NAME - %s",port_id,rte_lcore_id(),ring_2->name,rte_kni_get_name(kni));

        printf("\nStarting polling");
        while(1)        {
                // Dequeue packets from ring 
		f = 0;
                rx_sz = rte_ring_dequeue_burst(ring_2,(void *)buffer,BURST_SIZE,NULL);
                if (rx_sz == 0){
			rx_sz = rte_kni_rx_burst(kni,buffer,BURST_SIZE);
			if (rx_sz == 0)	{	
				continue;
			}
			f = 1;
			printf("\nReceived %d packets from : %s",rx_sz,rte_kni_get_name(kni));
                }
		if (f == 0)	{
                	printf("\nReceived %d packets from : %s",rx_sz,ring_2->name);
		}
                //Echo back to source port
		display_packets(buffer,rx_sz);
                tx_sz = rte_eth_tx_burst(PORT,0,buffer,rx_sz);
		if (tx_sz != rx_sz){
                	drop_packets(buffer,tx_sz,rx_sz,"Error while sending");
		}
                printf("\nEchoed:: %d packets to PORT :: %d",tx_sz,port_id);
        }
}


