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
main(int argc,char **argv){
	
	struct rte_mempool *mempool;
	struct rte_kni *kni;
	struct lcore_args slave_rx_args,slave_tx_args,slave_w_args;
	int n_ports,ret_val;

	//Init the eal with cli arguments
	ret_val = rte_eal_init(argc,argv);
	if (ret_val < 0){
		rte_exit(EXIT_FAILURE,"\nError while initializing rte");
	}
	printf("\nEAL initialised");
	
	//Check if we have enough ports with dpdk driver
	n_ports = rte_eth_dev_count_avail();
	if (n_ports < 2){
		rte_exit(EXIT_FAILURE,"\nNot enough ports");
	}
	printf("\nPorts are available");

	// Check if we have enough lcore 
	if (rte_lcore_count() < 4){
		rte_exit(EXIT_FAILURE,"Not enough lcores");
	}
	printf("\nAtleast 4 lcores are available");

	uint32_t main_lcore_id = rte_lcore_id();
	uint32_t rx_lcore_id = rte_get_next_lcore(main_lcore_id,1,1);
	uint32_t w_lcore_id = rte_get_next_lcore(rx_lcore_id,1,1);
	uint32_t tx_lcore_id = rte_get_next_lcore(w_lcore_id,1,1);
	//Create mempool buffers
	mempool = rte_pktmbuf_pool_create(RTE_MEMPOOL_NAME,2047,130,120,RTE_MBUF_DEFAULT_BUF_SIZE,rte_socket_id());
	printf("RTE_MBUF_DEFAULT_BUF_SIZE=%d\n",RTE_MBUF_DEFAULT_BUF_SIZE);
	if (mempool == NULL){
		rte_exit(EXIT_FAILURE,"\nError during initializing membuf pool");
	}
	printf("\nMempool creation complete");

	// Initialize port
	ret_val = init_port(PORT,mempool);
	if (ret_val < 0){
		rte_exit(EXIT_FAILURE,"\nError while port initilizing");
	}

	// Pre-alloc the kni
	ret_val =  rte_kni_init(2);
	if (ret_val < 0){
		rte_exit(EXIT_FAILURE,"\nError during kni init");
	}
	printf("\nKNI intialization complete.");
	
	// Allocate the kernel
	
	kni = kni_create (KNI_IF_NAME,mempool);
	if (kni == NULL){
		rte_exit(EXIT_FAILURE,"\nError while creating KNI");
	}
	printf("\nKNI interface successfully created");
	const char *kni_name = rte_kni_get_name(kni);
	printf("\nKNI interface created is %s",kni_name);
	// Set up rings for communication 
	
	struct rte_ring *rx_w_ring = rte_ring_create(RTE_RX_W_RING_NAME,64,rte_socket_id(),RING_F_SP_ENQ|RING_F_SC_DEQ);

	if (rx_w_ring == NULL){
		rte_exit(EXIT_FAILURE,"\nError while allocating rx_w_ring");
	}
	printf("\nAllocated rx_w_ring");

	struct rte_ring *w_tx_ring = rte_ring_create(RTE_W_TX_RING_NAME,64,rte_socket_id(),RING_F_SP_ENQ|RING_F_SC_DEQ);

	if (w_tx_ring == NULL){
		rte_exit(EXIT_FAILURE,"\nError while allocating w_tx_ring");
	}
	printf("\nAllocated w_tx_ring");
	// Setting up lcore args

	memset(&slave_rx_args,0,sizeof(slave_rx_args));
	memset(&slave_tx_args,0,sizeof(slave_tx_args));
	memset(&slave_w_args,0,sizeof(slave_w_args));

	// Setting up the port id
	slave_rx_args.port_id = PORT;
	slave_tx_args.port_id = PORT;
	slave_w_args.port_id = PORT;


	// Setting up the kni 
	 slave_rx_args.kni = kni;
	 slave_tx_args.kni = kni;
	 slave_w_args.kni = kni;

	// Setting up the rings
	slave_rx_args.ring_1 = rx_w_ring;
	slave_w_args.ring_1 = rx_w_ring;
	slave_w_args.ring_2 = w_tx_ring;
	slave_tx_args.ring_1 = w_tx_ring;

	// launch remote cores 
	rte_eal_remote_launch(slave_rx_main,&slave_rx_args,rx_lcore_id);
	rte_eal_remote_launch(slave_tx_main,&slave_tx_args,tx_lcore_id);
	rte_eal_remote_launch(slave_worker_main,&slave_w_args,w_lcore_id);


	// Wait for cores to return 
	
	rte_eal_wait_lcore(rx_lcore_id);
	rte_eal_wait_lcore(tx_lcore_id);
	rte_eal_wait_lcore(w_lcore_id);	
}
