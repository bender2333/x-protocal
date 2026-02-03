#ifndef __BACNETIPSERVER_H_E6CFE6D0_F0BD_4497_8761_7B19130F17BB
#define __BACNETIPSERVER_H_E6CFE6D0_F0BD_4497_8761_7B19130F17BB

#include "App.h"
#include "bacdef.h"
#include "npdu.h"
#include "cov.h"
#include "bvlc.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

/// @brief BACnet IP server IP information structure
typedef struct 
{
	/* data */
	int sock;
	struct sockaddr_in local_addr;
	struct sockaddr_in broadcast_addr;
	uint16_t port;
	uint16_t max_buff;
}BACNETIPSTRUCT;

#define BIP_FOREIGN_DEVICE_NONE		0
#define BIP_FOREIGN_DEVICE_SENT		1
#define BIP_FOREIGN_DEVICE_ALIVE	2

#define MAX_IP_HEADER (1 + 1 + 2)
#define MAX_IP_MPDU (MAX_IP_HEADER + MAX_IP_APDU + MAX_NPDU)

extern BYTE m_BIPRegForeignStatus;

extern BYTE m_BacnetIPTransmitBuff[];//MAX_IP_MPDU + 5];
extern BACNET_ADDRESS BIP_Broadcast_Address;
extern BACNET_ADDRESS BIP_Local_Address;

/**
 * @brief BACnet IP server initialization.
 *
 * @param none
 * @return true if initialization successful, false otherwise.
 */
bool BacnetIPServerInit();

/**
 * @brief Get BACnet IP broadcast address.
 *
 * @param[out] dest Pointer to destination address structure, to store the broadcast address.
 * @return none
 */
void BacnetIPServerGetBroadcastAddress(BACNET_ADDRESS * dest); 

/**
 * @brief Get our own BACnet IP address.
 *
 * @param[out] my_address Pointer to store our address.
 * @return none
 */
void BacnetIPServerGetMyAddress(BACNET_ADDRESS * my_address);

/**
 * @brief Get BACnet IP broadcast address and convert it to uint32_t.
 *
 * @param none
 * @return Broadcast address as uint32_t.
 */
uint32_t BacnetIPServerGetBroadcastAddress32(void); 

/**
 * @brief Get our own BACnet IP address and convert it to uint32_t.
 *
 * @param[out] none
 * @return our address as uint32_t.
 */
uint32_t BacnetIPServerGetMyAddress32(void);

/**
 * @brief Get BACnet IP server UDP socket.
 *
 * @param none
 * @return socket descriptor on success, -1 on erro
 */
int BacnetIPServerGetSock(void);

/**
 * @brief Get BACnet IP server UDP port number.
 *
 * @param none
 * @return UDP Port number.
 */
uint16_t BacnetIPServerGetPort(void);

/**
 * @brief Get BACnet IP server subnet mask.
 *
 * @param none
 * @return Subnet mask as uint32_t.
 */
uint32_t BacnetIPServerGetSubnetMask32(void);

/**
 * @brief Get BACnet IP server gateway address.
 *
 * @param none
 * @return Gateway address as uint32_t.
 */
uint32_t BacnetIPServerGetGateway32(void);

/**
 * @brief Check if BACnet Foreign Device mode is enabled.
 *
 * @param none
 * @return TRUE if enabled, FALSE otherwise.
 */
BOOL IsDeviceFDEnable(void);

/**
 * @brief Check if BACnet BBMD mode is enabled. (only applicable when BBMD is supported)
 *
 * @param none
 * @return TRUE if enabled, FALSE otherwise.
 */
BOOL IsDeviceBBMDEnable(void);

/**
 * @brief Send BACnet IP PDU.
 *
 * @param[in] dest       Destination address.
 * @param[in] npdu_data  pointer to NPDU data structure.
 * @param[in] pdu        pointer to PDU data to be send.
 * @param[in] pdu_len    Length of the PDU data.
 * @param[in] bacLink    BACnet link structure.
 * @return Number of bytes sent, 0 or negative value on error.
 */
int BacnetIPServerSendPdu(BACNET_ADDRESS * dest, BACNET_NPDU_DATA * npdu_data, uint8_t * pdu, unsigned pdu_len, BACNET_LINK *bacLink);

/**
 * @brief Foreign device object rxecution loop, keep track the time to live and re-register the device as FD after timeout.
 *
 * @param none
 * @return none
 */
void BacnetIPForeignDeviceObj(void);

/**
 * @brief Broadcast I-AM message through BACnet IP.
 *
 * @param none
 * @return none
 */
void BacnetIPSendIam(void);

/**
 * @brief Broadcast Network-Number-Is message through BACnet IP.
 *
 * @param none
 * @return none
 */
void BacnetIPSendNetworkNumberIs(void);

/**
 * @brief Send COV notification through BACnet IP.
 *
 * @param[in] dest               Destination address for COV notification.
 * @param[in] data               COV data to send.
 * @param[in] bConfirmNotification Whether to send confirmed notification.
 * @param[in] id                 Invoke ID for the message.
 * @return Invoke ID used for the message.
 */
BYTE BacnetIPSendCOV(BACNET_ADDRESS *dest, BACNET_COV_DATA * data, BOOL bConfirmNotification, BYTE id);

/**
 * @brief Get BACnet link structure for IP communication.
 *
 * @param[out] bacLink Pointer to store BACnet link structure.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL BacnetIPGetBacLink(BACNET_LINK * bacLink);

/**
 * @brief BACnet IP server task function, executes all BACnet IP communication and data processing.
 *
 * @param[in] pvParameters Task parameters.
 * @return none
 */
void bacnet_ip_server_task(void *pvParameters);

#endif  //end of __BACNETIPSERVER_H_E6CFE6D0_F0BD_4497_8761_7B19130F17BB