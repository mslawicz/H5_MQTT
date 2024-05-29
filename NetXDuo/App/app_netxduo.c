/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NX_PACKET_PAYLOAD_SIZE  1536
#define NX_PACKET_POOL_SIZE ((NX_PACKET_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)
#define NETMASK 0xFFFFFF00UL // 255.255.255.0
#define NX_IP_SIZE  2048
#define ARP_CACHE_SIZE 1024
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
NX_PACKET_POOL nx_packet_pool;
NX_IP ip_0;
//NX_DNS dns_0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

   /* USER CODE BEGIN App_NetXDuo_MEM_POOL */
  (void)byte_pool;
  /* USER CODE END App_NetXDuo_MEM_POOL */
  /* USER CODE BEGIN 0 */

    /* Initialize the NetX system. */
    nx_system_initialize();

    /* Create a packet pool. */
    ret = nx_packet_pool_create(&nx_packet_pool, "Packet Pool", NX_PACKET_PAYLOAD_SIZE, memory_ptr, NX_PACKET_POOL_SIZE);
    if(ret != NX_SUCCESS)
    {
        return ret;
    }
    
    /* Create an IP instance. */
    ret = nx_ip_create(&ip_0, "IP Instance 0", IP_ADDRESS(192, 168, 1, 101), NETMASK, &nx_packet_pool, nx_stm32_eth_driver, memory_ptr, NX_IP_SIZE, 1);
    if(ret != NX_SUCCESS)
    {
        return ret;
    }

    /* Enable ARP and supply ARP cache memory. */
    nx_arp_enable(&ip_0, memory_ptr, ARP_CACHE_SIZE);

    /* Enable the ICMP. */
    nx_icmp_enable(&ip_0);

    /* Enable the UDP protocol. */
    nx_udp_enable(&ip_0);

    /* Enable the TCP protocol. */
    nx_tcp_enable(&ip_0);
    
    /* DNS initialization */
    ////nx_dns_create(&dns_0, &ip_0, (UCHAR *)"DNS Instance");
    //nx_dns_server_add(&dns_0, IP_ADDRESS_DNS_SERVER);
    
    // Start network
    //nx_ip_address_set(&ip_0, IP_ADDRESS, NETMASK);
  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
