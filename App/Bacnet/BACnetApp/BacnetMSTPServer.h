#ifndef __BACNETMSTPSERVER_H_A5512A97_11E9_497f_A9EC_0C15DCAF242A
#define __BACNETMSTPSERVER_H_A5512A97_11E9_497f_A9EC_0C15DCAF242A

#include "AppConfig.h"
#include "bacdef.h"
#include "npdu.h"
#include "cov.h"

#define MAX_MSTP_HEADER (2+1+1+1+2+1+2)
#define MAX_MSTP_MPDU (MAX_MSTP_HEADER + MAX_MSTP_APDU + MAX_NPDU)

/// @brief BACnet MS/TP frame structure
typedef struct
{
    uint8_t    Status;
    uint8_t    FrameType;
    uint8_t    DestAddress;
    uint8_t    SourceAddress;
    uint16_t   DataLength;
    uint8_t    Buffer[MAX_MSTP_MPDU];
}BACNETMSTPFRAME;

/**
 * @brief BACnet MSTP server initialization.
 *
 * @param none
 * @return none
 */
void BacnetMSTPServerInit(void);

/**
 * @brief Set local MSTP MAC address for a specific communitation channel.
 *
 * @param[in] ch   Channel number.
 * @param[in] mac  MAC address to set.
 * @return none
 */
void BacnetMSTPServerSetLocalAddress(uint8_t ch, uint8_t mac);

/**
 * @brief Get BACnet MS/TP broadcast address.
 *
 * @param[out] dest Pointer to store the broadcast address.
 * @return none
 */
void BacnetMSTPServerGetBroadcastAddress (BACNET_ADDRESS * dest);

/**
 * @brief Get our own BACnet MS/TP address for a specific channel.
 *
 * @param[out] my_address Pointer to store our address.
 * @param[in] ch          Channel number.
 * @return none
 */
void BacnetMSTPServerGetMyAddress(BACNET_ADDRESS * my_address, uint8_t ch);

/**
 * @brief Handle received BACnet MS/TP frame.
 *
 * @param[in] ch        Channel number where frame was received.
 * @param[in] MSTPFrame Pointer to the received MSTP frame.
 * @return none
 */
void OnBacnetMSTPReceive(uint8_t ch, BACNETMSTPFRAME *MSTPFrame);

/**
 * @brief Send BACnet MS/TP PDU.
 *
 * @param[in] dest       Destination address.
 * @param[in] npdu_data  pointer to NPDU data structure.
 * @param[in] pdu        pointer to PDU data to be send.
 * @param[in] pdu_len    Length of PDU data.
 * @param[in] bacLink    BACnet link structure.
 * @return Number of bytes sent, 0 or negative value on error.
 */
int BacnetMSTPServerSendPdu(BACNET_ADDRESS * dest, BACNET_NPDU_DATA * npdu_data, uint8_t * pdu, unsigned pdu_len, BACNET_LINK *bacLink);

/**
 * @brief Broadcast I-AM message through BACnet MS/TP.
 *
 * @param none
 * @return none
 */
void BacnetMSTPSendIam(void);

/**
 * @brief Broadcast Network-Number-Is message through BACnet MS/TP.
 *
 * @param none
 * @return none
 */
void BacnetMSTPSendNetworkNumberIs(void);

/**
 * @brief Send COV message through BACnet MS/TP.
 *
 * @param[in] channel Channel number.
 * @param[in] dest    Destination address.
 * @param[in] data    COV data.
 * @param[in] bConfirmNotification Confirm Notification flag.
 * @param[in] id      Invoke ID.
 */
int BacnetMSTPSendCOV(uint8_t channel, BACNET_ADDRESS *dest, BACNET_COV_DATA *data, bool bConfirmNotification, uint8_t id);

/**
 * @brief Get BACnet link structure for MS/TP communication.
 *
 * @param[in] index Channel number.
 * @param[in] bacLink Pointer to store BACnet link structure.
 * @return none
 */
bool BacnetMSTPGetBacLink(uint8_t index, BACNET_LINK * bacLink);

#endif      //endof __BACNETMSTPSERVER_H_A5512A97_11E9_497f_A9EC_0C15DCAF242A