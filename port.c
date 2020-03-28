#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>


#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_kni.h>

#include "asst5.h"


/*
 *	To open and intialize a port and associate a mempool to it 
 *	Step 1 - Confgiure the ethernet device
 *		rte_eth_dev_configure() 
 *	Step 2 - Check if the rx and tx descriptors adhere to the limits of the actual device
 *		rte_eth_dev_adjust_nb_rx_tx_desc()	(OPTIONAL PROBABLY) just a check
 *	Step 3 Allocate and set up a receive queue for the ethernet device
 *		rte_eth_rx_queue_setup()
 *	Step 4 Allocate and set up a transmit queue for the ethernet device
 *		rte_eth_tx_queue_setup()
 *	Step 5 Start teh ethernet device with the setup parameters
 *		rte_eth_dev_start()
 *
 */


static const struct
rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = RTE_ETHER_MAX_LEN,	
	},
}

int
init_port(int port_id,struct rte_mempool *mempool){
	struct rte_eth_dev_info dev_info;
	struct rte_eth_tx_conf tx_conf;
	uint16_t rx_rings = RING_SIZE;
	uint16_t tx_rings = RING_SIZE;
	struct rte_eth_conf port_conf = port_conf_default;
	int ret_val = rte_eth_dev_info_get(port_id,&dev_info);

	//Get device info of the specified port
	if (ret_val != 0){
		printf("\nError during dev_info_get");
		return ret_val;
	}
	
	// Enable tx offlaoding with fast mem buffer??
	if(dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE){
		port_conf.txmode.offloads |= DEV_TX_OFFLOAD_MBUF_FAST_FREE;
	}

	// set up the trasmit queue to default settings
	tx_conf = dev_info.default_txconf;
	tx_conf.offloads = port_conf.txmode.offloads;

	// Configure the device using the dev_info and
	ret_val =  rte_eth_dev_configure(port_id,NB_RX_QUEUES,NB_TX_QUEUES,&port_conf);
	if (ret_val != 0){
		printf("\nError during rte_eth_dev_configure");
		return ret_val;
	}
	printf("\nConfiguring the device complete...");
	
	// Limit check
	ret_val =  rte_eth_dev_adjust_nb_rx_tx_desc(port_id,&rx_rings,&tx_rings);
	if (ret_val != 0){
		printf("\nError during rte_eth_dev_adjust_nb_rx_tx_desc");
		return ret_val;
	}
	printf("\nNumber of rings adhere to device limits");

	//RX queue setup
	ret_val = rte_eth_rx_queue_setup(port_id,NB_RX_QUEUES-1,rx_rings,rte_eth_dev_socket_id(port_id),NULL,mempool);
	if (ret_val < 0){
		printf("\nError while rx_queue setup");
		return ret_val;
	}
	printf("\nRX queue setup successfull");
	//TX queue setup
	ret_val = rte_eth_tx_queue_setup(port_id,NB_TX_QUEUES-1,tx_rings,rte_eth_dev_socket_id(port_id),&tx_conf);
	if (ret_val < 0){
		printf("\nError while tx_queue setup");
		return ret_val;
	}
	printf("\nTX queue setup successfull");

	//Start the device
	ret_val = rte_eth_dev_start(port_id)
	if (ret_val != 0){
		printf("\nError while starting the ethernet device");
		return ret_val;
	}

	printf("\nThe ethernet device is up and running \n");
	printf("\nPort initialization is complete");

	//Enter promiscuous mode
	ret_val = rte_ethpromiscuous_enable(port_id);
	if (ret_val != 0){
		printf("\nUnable to enter to promiscuous mode");
		return ret_val;
	}
	printf("\nEntered promiscuous mode\nReturning from init_port");
	return ret_val;
}

