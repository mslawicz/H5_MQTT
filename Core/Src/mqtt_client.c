#include "mqtt_client.h"
#include "main.h"
#include "nx_api.h"
#include "nx_ip.h"
#include "nxd_dns.h"
#include "nx_stm32_eth_driver.h"
#include "nxd_mqtt_client.h"


#define NX_PACKET_PAYLOAD_SIZE  1536
#define NX_PACKET_POOL_SIZE (NX_PACKET_PAYLOAD_SIZE * 10)
#define NETMASK 0xFFFFFF00UL // 255.255.255.0
#define IP_MEMORY_SIZE  2048
#define ARP_CACHE_SIZE 512
#define IP_ADDRESS_DNS_SERVER 0x08080808 // 8.8.8.8 (Google DNS)
#define IP_ADDRESS_CLIENT IP_ADDRESS(192, 168, 1, 101)
#define CLIENT_ID   "STM32"
#define MQTT_THREAD_STACK_SIZE 1024
#define MQTT_THREAD_PRIORITY    2
#define MQTT_CLIENT_STACK_SIZE 1024
#define MQTT_CLIENT_MEMORY_SIZE 1024
//#define BROKER_IP   IP_ADDRESS(3, 73, 167, 11)  //broker.hivemq.com
//#define BROKER_IP   IP_ADDRESS(34, 249, 184, 60)  //broker.emqx.io
//#define BROKER_IP   IP_ADDRESS(192, 168, 1, 18)  //etteplan laptop
#define BROKER_IP   IP_ADDRESS(192, 168, 1, 23)  //Marcin PC
#define MQTT_PORT   NXD_MQTT_CLIENT_NONTLS_PORT
#define MQTT_KEEP_ALIVE_INTERVAL    300
#define TOPIC   "test topic"
#define MESSAGE "message from STM32"
#define PING_DATA   "ping from STM32"

NX_PACKET_POOL nx_packet_pool;
NX_IP ip_0;
NX_DNS dns_0;
UCHAR pool_memory[NX_PACKET_POOL_SIZE];
UCHAR ip_memory[IP_MEMORY_SIZE];
UCHAR arp_cache_memory[ARP_CACHE_SIZE];
NXD_MQTT_CLIENT mqtt_client;
const CHAR* clientId = CLIENT_ID;
UCHAR mqtt_thread_stack[MQTT_THREAD_STACK_SIZE];
ULONG mqtt_client_stack[MQTT_CLIENT_STACK_SIZE];
ULONG mqtt_client_memory_buffer[MQTT_CLIENT_MEMORY_SIZE];
NXD_ADDRESS broker_ip;
CHAR* topic_str = TOPIC;
CHAR* msg_str = MESSAGE;
CHAR* ping_str = PING_DATA;
NX_PACKET ping_response;
NX_PACKET* pResponse;

VOID mqtt_receive_callback(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages);
static VOID mqtt_disconnect_callback(NXD_MQTT_CLIENT *client_ptr);

VOID mqttClientThreadEntry(ULONG initial_input)
{
    UNUSED(initial_input);
    UINT status;

    /* Initialize the NetX system. */
    printf("Initialization of NX system\r\n");
    nx_system_initialize();

    /* Create a packet pool. */
    status = nx_packet_pool_create(&nx_packet_pool, "Packet Pool", NX_PACKET_PAYLOAD_SIZE, pool_memory, NX_PACKET_POOL_SIZE);
    printf("NX packet pool creation with status %X\r\n", status);

    /* Create an IP instance. */
    status = nx_ip_create(&ip_0, "IP Instance 0", IP_ADDRESS_CLIENT, NETMASK, &nx_packet_pool, nx_stm32_eth_driver, ip_memory, IP_MEMORY_SIZE, 1);
    printf("NX IP creation with status %X\r\n", status);

    /* Enable ARP and supply ARP cache memory. */
    status = nx_arp_enable(&ip_0, arp_cache_memory, ARP_CACHE_SIZE);
    printf("NX ARP creation with status %X\r\n", status);

    /* Enable the ICMP. */                
    status = nx_icmp_enable(&ip_0);
    printf("NX ICMP creation with status %X\r\n", status);

    /* Enable the UDP protocol. */
    status = nx_udp_enable(&ip_0);
    printf("NX UDP creation with status %X\r\n", status);

    /* Enable the TCP protocol. */
    status = nx_tcp_enable(&ip_0);
    printf("NX TCP creation with status %X\r\n", status);

    /* DNS initialization */
    status = nx_dns_create(&dns_0, &ip_0, (UCHAR *)"DNS Instance");
    status = nx_dns_server_add(&dns_0, IP_ADDRESS_DNS_SERVER);
    printf("NX adding DNS with status %X\r\n", status);

    /* Start network */
    status = nx_ip_address_set(&ip_0, IP_ADDRESS_CLIENT, NETMASK);
    printf("NX setting IP with status %X\r\n", status);

    /* MQTT client initialization */
    status = nxd_mqtt_client_create(&mqtt_client, "MQTT Client", (CHAR*)clientId, strlen(clientId), &ip_0, &nx_packet_pool, mqtt_client_stack, MQTT_CLIENT_STACK_SIZE, MQTT_THREAD_PRIORITY, mqtt_client_memory_buffer, MQTT_CLIENT_MEMORY_SIZE);
    printf("MQTT client initialized with status %X\r\n", status);

    // Register the receive callback
    status = nxd_mqtt_client_receive_notify_set(&mqtt_client, mqtt_receive_callback);
    printf("MQTT client receive notification set with status %X\r\n", status);

    /* Register the disconnect notification function. */
    status = nxd_mqtt_client_disconnect_notify_set(&mqtt_client, mqtt_disconnect_callback);
    printf("MQTT client disconnect callback set with status %X\r\n", status);

    /* Connect to MQTT broker */
    broker_ip.nxd_ip_version = 4;
    broker_ip.nxd_ip_address.v4 = BROKER_IP;
    status = nxd_mqtt_client_connect(&mqtt_client, &broker_ip, MQTT_PORT, MQTT_KEEP_ALIVE_INTERVAL, NX_TRUE, NX_WAIT_FOREVER);
    printf("MQTT client connecting to broker with status %X\r\n", status);

    if((status != NX_SUCCESS) && (status != NX_IN_PROGRESS))
    {
        //Error_Handler();
    }

    //status = nxd_mqtt_client_subscribe(&mqtt_client, topic_str, strlen(topic_str), MQTT_PUBLISH_QOS_LEVEL_0);
    //printf("MQTT client subscribed with status %X\r\n", status);

    while(1)
    {
        tx_thread_sleep(MS_TO_TICKS(2000));
        HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
        pResponse = &ping_response;
        status = nxd_icmp_ping(&ip_0, &broker_ip, ping_str, strlen(ping_str), &pResponse, 80);
        if(status == NX_SUCCESS)
        {
            printf("ping response with length %lu\r\n", pResponse->nx_packet_length);
            nx_packet_release(pResponse);
        }
        else
        {
            printf("ping no response with status %X\r\n", status);
        }
        tx_thread_sleep(MS_TO_TICKS(2000));
        status = nxd_mqtt_client_publish(&mqtt_client, topic_str, strlen(topic_str), msg_str, strlen(msg_str), NX_FALSE, MQTT_PUBLISH_QOS_LEVEL_0, NX_NO_WAIT);
        printf("client published with status %X\r\n", status);
    } 
}


VOID mqtt_receive_callback(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    UNUSED(client_ptr);
    UNUSED(number_of_messages);
    HAL_GPIO_TogglePin(LED_Y_GPIO_Port, LED_Y_Pin);
    printf("client received a message\r\n");
}

/* Define the disconnect notify function. */
static VOID mqtt_disconnect_callback(NXD_MQTT_CLIENT *client_ptr)
{
    UNUSED(client_ptr);
    printf("client disconnected from broker\r\n");
}
