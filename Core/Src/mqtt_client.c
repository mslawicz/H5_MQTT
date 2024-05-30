#include "mqtt_client.h"
#include "main.h"
#include "nx_api.h"
#include "nxd_dns.h"
#include "nx_stm32_eth_driver.h"

#define NX_PACKET_PAYLOAD_SIZE  1536
#define NX_PACKET_POOL_SIZE ((NX_PACKET_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)
#define NETMASK 0xFFFFFF00UL // 255.255.255.0
#define NX_IP_SIZE  2048
#define ARP_CACHE_SIZE 1024
#define IP_ADDRESS_DNS_SERVER 0x08080808 // 8.8.8.8 (Google DNS)
#define IP_ADDRESS_CLIENT IP_ADDRESS(192, 168, 1, 101)

NX_PACKET_POOL nx_packet_pool;
NX_IP ip_0;
NX_DNS dns_0;
UCHAR pool_memory[NX_PACKET_POOL_SIZE];
UCHAR arp_cache_memory[ARP_CACHE_SIZE];

VOID mqttClientThreadEntry(ULONG initial_input)
{
    UNUSED(initial_input);
    UINT status;

    /* Initialize the NetX system. */
    nx_system_initialize();

    /* Create a packet pool. */
    status = nx_packet_pool_create(&nx_packet_pool, "Packet Pool", NX_PACKET_PAYLOAD_SIZE, pool_memory, NX_PACKET_POOL_SIZE);

    /* Create an IP instance. */
    status |= nx_ip_create(&ip_0, "IP Instance 0", IP_ADDRESS_CLIENT, NETMASK, &nx_packet_pool, nx_stm32_eth_driver, pool_memory, NX_IP_SIZE, 1);

    /* Enable ARP and supply ARP cache memory. */
    status |= nx_arp_enable(&ip_0, arp_cache_memory, ARP_CACHE_SIZE);

    /* Enable the ICMP. */
    status |= nx_icmp_enable(&ip_0);

    /* Enable the UDP protocol. */
    status |= nx_udp_enable(&ip_0);

    /* Enable the TCP protocol. */
    status |= nx_tcp_enable(&ip_0);

    /* DNS initialization */
    status |= nx_dns_create(&dns_0, &ip_0, (UCHAR *)"DNS Instance");
    status |= nx_dns_server_add(&dns_0, IP_ADDRESS_DNS_SERVER);

    /* Start network */
    status |= nx_ip_address_set(&ip_0, IP_ADDRESS_CLIENT, NETMASK);

    if(status)
    {
        Error_Handler();
    }

    while(1)
    {
        tx_thread_sleep(MS_TO_TICKS(1000));
    } 
}