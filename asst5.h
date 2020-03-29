#define PORT 0						//The port we use as default
#define BURST_SIZE 32					//Burst size
#define KNI_IF_NAME "veth_0"				//Name for the KNI interface we'll be creating
#define RTE_MEMPOOL_NAME "rte_mempool_0"		//Name for memory pool
#define RTE_RX_W_RING_NAME "rte_rx_worker_ring_0"	//Name for the rx to worker threads RING 
#define RTE_W_TX_RING_NAME "rte_worker_tx_ring_0"	//Name for the worker to tx threads RING

#define NB_RX_QUEUES 1				//Number of receive queues to setup for our ethernet device
#define NB_TX_QUEUES 1				//Number of transmit queues to set up for our ethernet device
#define RING_SIZE 1024		




struct lcore_args{
	unsigned int port_id;
	struct rte_ring *ring_1;
	struct rte_ring *ring_2;
	struct rte_kni *kni;
};


//Thread main functions
int slave_rx_main(void *args_ptr);
int slave_worker_main(void *args_ptr);
int slave_tx_main(void *args_ptr);

//Intializing functions

struct rte_kni* kni_create(const char* name,struct rte_mempool *mempool);
int init_port(int port_id,struct rte_mempool* mempool);
