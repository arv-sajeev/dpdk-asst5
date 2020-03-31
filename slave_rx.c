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

int slave_rx_main(void *args_ptr){
	struct rte_mbuf* buffer[BURST_SIZE];
	uint16_t rx_sz,tx_sz;

	//Receive and typecast all args
	struct lcore_args* slave_rx_args = (struct lcore_args*)args_ptr;
	unsigned int port_id = slave_rx_args->port_id;
	struct rte_ring	*ring_1  = slave_rx_args->ring_1;
	struct rte_kni *kni = slave_rx_args->kni;

	printf("\nIn slave_rx_main with :-\nPORT_ID - %d\nLCORE_ID - %d\nRTE_RING - %s\nKNI_NAME - %s",port_id,rte_lcore_id(),ring_1->name,rte_kni_get_name(kni));
	
	printf("\nStarting polling");

	while(1)	{
		// Receive burst from eth device
		rte_kni_handle_request(kni);
		rx_sz = rte_eth_rx_burst(port_id,0,buffer,BURST_SIZE);

		if (rx_sz == 0){
			continue;
		}

		struct rte_mbuf *PKT = buffer[0];
		unsigned char  *pkt = rte_pktmbuf_mtod(PKT,unsigned char*);
		//Checking if the first bytes matches our pattern or else reject entire burst
		printf("\nIdentifying bytes are %02X and %02X",pkt[4],pkt[5]);
		if (pkt[5] == 0x64 && pkt[4] == 0x35){
			printf("\n\tReceived %d packets on port : %d",rx_sz,port_id);
			tx_sz = rte_kni_tx_burst(kni,buffer,rx_sz);
			printf("\nSend %d packets to kni :: %s",tx_sz,rte_kni_get_name(kni));
		}

		else	{
			drop_packets(buffer,0,rx_sz,"Wrong Destination address");
			continue;
		}


		//Enqueue to ring to worker
		//tx_sz = rte_ring_enqueue_burst(ring_1,(void *)buffer,rx_sz,NULL);
		drop_packets(buffer,tx_sz,rx_sz,"Error while sending");
		//printf("\nEnqueued :: %d packets to :: %s",tx_sz,ring_1->name);
	}
}
