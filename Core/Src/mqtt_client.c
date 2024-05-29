#include "mqtt_client.h"
#include "main.h"
#include "nx_api.h"
#include "tx_api.h"
#include "nx_stm32_eth_driver.h"

#define NX_PACKET_PAYLOAD_SIZE  1536
#define NX_PACKET_POOL_SIZE ((NX_PACKET_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)
#define NETMASK 0xFFFFFF00UL // 255.255.255.0
#define NX_IP_SIZE  2048

NX_PACKET_POOL nx_packet_pool;
NX_IP ip_0;
//NX_DNS dns_0;

void nx_application_define(void *first_unused_memory)
{
    UINT status;

    /* Initialize the NetX system. */
    nx_system_initialize();

    /* Create a packet pool. */
    status = nx_packet_pool_create(&nx_packet_pool, "Packet Pool", NX_PACKET_PAYLOAD_SIZE, first_unused_memory, NX_PACKET_POOL_SIZE);
    if(status != NX_SUCCESS)
    {
        Error_Handler();
    }
    
    /* Create an IP instance. */
    nx_ip_create(&ip_0, "IP Instance 0", IP_ADDRESS(192, 168, 1, 101), NETMASK, &nx_packet_pool, nx_stm32_eth_driver, first_unused_memory, NX_IP_SIZE, 1);
    
    // Enable ARP, UDP, TCP, ICMP
    //nx_arp_enable(&ip_0, ARP_CACHE, ARP_CACHE_SIZE);
    //nx_udp_enable(&ip_0);
    //nx_tcp_enable(&ip_0);
    //nx_icmp_enable(&ip_0);
    
    // DNS initialization
    //nx_dns_create(&dns_0, &ip_0, (UCHAR *)"DNS Instance");
    //nx_dns_server_add(&dns_0, IP_ADDRESS_DNS_SERVER);
    
    // Start network
    //nx_ip_address_set(&ip_0, IP_ADDRESS, NETMASK);
}