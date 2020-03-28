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

