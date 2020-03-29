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

/*
 *	Step 1 Fill up the kni_conf structure
 *	Step 2 Fill up the kni_ops structure
 *	Step 3 Use kni_alloc with these structures to set it up 
 *
*/

//Dummy function 1 for kni_ops
static int
kni_config_network_if(uint16_t port_id,uint8_t if_up){
	printf("\nThe port id in KNI is %d.\nTrying to set = %d",port_id,if_up);
	return 0;
}


// Dummy function 2 for kni_ops
static int 
kni_change_mtu(uint16_t port_id,unsigned int new_mtu){
	printf("\nThe port id in KNI is %d.\nTrying to set MTU = %d",port_id,new_mtu);
	return 0;
}

struct rte_kni*
kni_create(const char* name,struct rte_mempool *mempool){
	printf("\nIn kni_create");
	struct rte_kni_conf kni_conf;
	struct rte_kni_ops kni_ops;
	struct rte_kni* kni;

	if (name == NULL)
		return NULL;
	if (mempool == NULL)
		return NULL;

	memset(&kni_conf,0,sizeof(kni_conf));
	memset(&kni_ops,0,sizeof(kni_ops));

	//Fill up kni_conf
	
	strncpy(kni_conf.name,name,10);
	kni_conf.group_id = 0;
	kni_conf.mbuf_size = RTE_MBUF_DEFAULT_DATAROOM;
	kni_conf.mtu = 1500;
	kni_conf.min_mtu = 68;
	kni_conf.max_mtu = 65535;

	int ret_val = rte_eth_macaddr_get(0,(struct rte_ether_addr*)&kni_conf.mac_addr);
	if (ret_val != 0){
		printf("\nError while filling MAC Address");
		return NULL;
	}
	printf("\nFilled the kni_conf structure");

	kni_ops.config_network_if = kni_config_network_if;
	kni_ops.change_mtu = kni_change_mtu;
	kni_ops.config_mac_address = NULL;
	kni_ops.config_promiscusity = NULL;
	kni_ops.config_allmulticast = NULL;
	
	kni = rte_kni_alloc(mempool,&kni_conf,&kni_ops);
	return kni;
}
