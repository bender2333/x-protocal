/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#ifndef DATALINK_H
#define DATALINK_H

#include "bacdef.h"
#include "npdu.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief BACnet datalink layer initialization.
 *
 * @param[in] ifname Interface name for datalink initialization.
 * @return true if initialization successful, false otherwise.
 */
bool datalink_init (char *ifname);

/** 
 * @brief Send a BACnet PDU through the datalink layer, either IP or MS/TP, refer BACnetIPServer_send_pdu and BacnetMSTPServerSendPdu.
 * @param[in] dest       Destination address.
 * @param[in] npdu_data  NPDU data.
 * @param[in] pdu        PDU data to send.
 * @param[in] pdu_len    Length of PDU data.
 * @param[in] bacLink    BACnet link information.
 * @return Number of bytes sent, or negative value on error.
 */
int datalink_send_pdu(
    BACNET_ADDRESS * dest,
    BACNET_NPDU_DATA * npdu_data,
    uint8_t * pdu,
    unsigned pdu_len, BACNET_LINK *bacLink);

/**
 * @brief Get broadcast address from BACnet link structure.
 *
 * @param[out] dest    Pointer to store the broadcast address.
 * @param[in] bacLink  BACnet link structure.
 * @return none
 */
extern void datalink_get_broadcast_address(
    BACNET_ADDRESS * dest, BACNET_LINK *bacLink);

/**
 * @brief Get our own address for a specific BACnet link structure.
 *
 * @param[out] my_address Pointer to store our address.
 * @param[in] bacLink     BACnet link structure.
 * @return none
 */
extern void datalink_get_my_address(
    BACNET_ADDRESS * my_address, BACNET_LINK *bacLink);

/**
 * @brief Get broadcast address for a specific BACnet link type and Communication Channel (IP, MS/TP).
 *
 * @param[out] my_address Pointer to store our address.
 * @param[in] type        BACnet link type.
 * @param[in] ch          Communication Channel. (used for multiple IP address)
 * @return none
 */
void datalink_get_broadcast_address_by_type(
    BACNET_ADDRESS * dest, BACNET_DATALINK_TYPE type, uint8_t ch);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @defgroup DataLink The BACnet Network (DataLink) Layer
 * <b>6 THE NETWORK LAYER </b><br>
 * The purpose of the BACnet network layer is to provide the means by which 
 * messages can be relayed from one BACnet network to another, regardless of 
 * the BACnet data link technology in use on that network. Whereas the data 
 * link layer provides the capability to address messages to a single device 
 * or broadcast them to all devices on the local network, the network layer 
 * allows messages to be directed to a single remote device, broadcast on a 
 * remote network, or broadcast globally to all devices on all networks. 
 * A BACnet Device is uniquely located by a network number and a MAC address.
 * 
 * Each client or server application must define exactly one of these
 * DataLink settings, which will control which parts of the code will be built:
 * - BACDL_ETHERNET -- for Clause 7 ISO 8802-3 ("Ethernet") LAN
 * - BACDL_ARCNET   -- for Clause 8 ARCNET LAN
 * - BACDL_MSTP     -- for Clause 9 MASTER-SLAVE/TOKEN PASSING (MS/TP) LAN
 * - BACDL_BIP      -- for ANNEX J - BACnet/IP 
 * - BACDL_ALL      -- Unspecified for the build, so the transport can be 
 *                     chosen at runtime from among these choices.
 * - Clause 10 POINT-TO-POINT (PTP) and Clause 11 EIA/CEA-709.1 ("LonTalk") LAN
 *   are not currently supported by this project.
          *//** @defgroup DLTemplates DataLink Template Functions
 * @ingroup DataLink
 * Most of the functions in this group are function templates which are assigned
 * to a specific DataLink network layer implementation either at compile time or
 * at runtime.
 */
#endif
