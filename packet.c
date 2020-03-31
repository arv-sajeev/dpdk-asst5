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

int drop_packets(struct rte_mbuf** buffer,int start,int end,const char *msg){
	printf("\nDropping packets - %s",msg);
	for (int i = start;i < end;i++){
		rte_pktmbuf_free(buffer[i]);
		printf("\n\tDropped packet %d",i);
	}	
	return 0;
}

int display_packets(struct rte_mbuf** buffer,int rx_sz){
	for (int i = 0;i < rx_sz;i++){
		struct rte_mbuf *PKT = buffer[i];
		unsigned char *pkt = rte_pktmbuf_mtod(PKT,unsigned char*);
		int pkt_len = PKT->pkt_len;
		printf("\nPacket %d of length %d :: \n [",i,pkt_len);
		for (int j = 0;j < pkt_len;j++){
			printf(" %02X",pkt[j]);
		}
		printf("]\n\n");
	}
	return 0;
}


