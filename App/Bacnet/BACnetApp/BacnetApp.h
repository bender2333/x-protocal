#ifndef __BACNETAPP_H_ACFAFD35_341D_4831_8286_85865820316B
#define __BACNETAPP_H_ACFAFD35_341D_4831_8286_85865820316B

#include "datalink.h"


extern uint32_t recipient_scan_tmr;//timer loop count to keep track on notification class recipients

/**
 * @brief Get BACnet link structure for a specific port.
 *
 * @param[in] port     Port number to get link information for.
 * @param[out] bacLink Pointer to store the BACnet link structure.
 * @return true if link structure was retrieved successfully, false otherwise.
 */
bool BacnetGetBacLink(uint8_t port, BACNET_LINK * bacLink);

/**
 * @brief  BACnet application initialization for datalinks, BACnet objects and COV.
 *
 * @param none
 * @return none
 */
void BacnetAppInit(void);

/**
 * @brief Get the next available invoke ID for BACnet communication.
 *
 * @param none
 * @return Next available invoke ID.
 */
uint8_t BacnetGetNextInvokeID(void);

/**
 * @brief BACnet application task for all BACnet objects and COV executions loop.
 *
 * @param[in] pvParameters Pointer to task parameters.
 * @return none
 */
void bacnet_app_task(void *pvParameters);

#endif      //__BACNET_H_ACFAFD35_341D_4831_8286_85865820316B