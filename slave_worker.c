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

int
slave_worker_main(void *args_ptr){
	struct rte_mbuf* buffer[BURST_SIZE];
	uint16_t rx_sz,tx_sz;
	
	//Receive and typecast all args
	struct lcore_args* slave_worker_args = (struct lcore_args*) args_ptr;
	unsigned int port_id = slave_worker_args->port_id;
	struct rte_ring *ring_1 = slave_worker_args->ring_1;
	struct rte_ring *ring_2 = slave_worker_args->ring_2;
	struct rte_kni *kni = slave_worker_args->kni;

	printf("\nIn slave_worker_main with :-\nPORT_ID - %d\nLCORE_ID - %d\nRTE_RING - %s\nKNI_NAME - %s",port_id,rte_lcore_id(),ring_1->name,rte_kni_get_name(kni));

	printf("\nStarting polling");

	while(1)	{

		//Dequeue packets from the ring 
		rx_sz = rte_ring_dequeue_burst(ring_1,(void *)buffer,BURST_SIZE,NULL);
		if (rx_sz == 0)	{
			continue;
		}
		
		//Print the packets received
		printf("\nReceived %d packets from %s\n",rx_sz,ring_1->name);
		display_packets(buffer,rx_sz);
		/*
		for(int i = 0;i < rx_sz;i++){
			//Flip Source and Dest to echo back
			unsigned char t;
			for (int i = 0;i < 6;i++){
				t = pkt[i];
				pkt[i] = pkt[i+6];
				pkt[i+6] = t;
			}
			printf("\nThe flipped packet is");
                	printf("Packet %d of length %d [",i+1,pkt_len);
                 	for (int j = 0;j < pkt_len;j++){
                         	printf(" %02X",pkt[j]);
                        }
                        printf("]\n\n");
                }*/

		//Enqueuing the packets to the next ring
		printf("\nEnqueueing the packets to :: %s",ring_2->name);
		tx_sz = rte_ring_enqueue_burst(ring_2,(void *)buffer,rx_sz,NULL);
		drop_packets(buffer,tx_sz,rx_sz,"Error while sending");
                printf("\nEnqueued :: %d packets to :: %s",tx_sz,ring_2->name);
	}
}
