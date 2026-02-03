#ifndef __BACNETCOV_H_0109855E_0523_47b2_B8BA_A07E781963D5
#define __BACNETCOV_H_0109855E_0523_47b2_B8BA_A07E781963D5

#include "bacdef.h"
#include "cov.h"
#include "rp.h"
#include "wp.h"
#include "readrange.h"

#define BACNET_COV_MAX_SUBSCRIPTIONS	32

typedef enum
{
	COV_FLOAT,		
	COV_BINARY,	
	COV_UNSIGNED,	
	COV_SIGNED,		
} COV_DATATYPE;

#define COVACK_NONE		0
#define COVACK_WAIT		1

#define COVSTS_NONE		0x00
#define COVSTS_VALID	0x01
#define COVSTS_INIT		0x80

/// @brief COV Value
typedef union 
{
	float 		fv;
	uint32_t 	uv;
	int32_t 	sv;
	bool 		bv;
}COV_VALUE;

/// @brief COV Node object
typedef struct
{
    BACNET_ADDRESS dest;
    uint32_t 	subscriberProcessIdentifier;
    uint32_t 	monitoredObjectIdentifier;
    uint8_t 	issueConfirmedNotifications; 

	uint32_t 	lifetime;          
	uint32_t 	remainTime;
	uint32_t 	ackTime;

	uint8_t	COVType;

	COV_VALUE previous_value;
	COV_VALUE current_value;
	COV_VALUE cov_increment;

	uint8_t	prevStatus;
	uint8_t	curStatus;

	uint8_t	COVStatus;//BYTE	Valid; //yfy 2019-9-24
	uint8_t	dataLink;
	uint8_t	ackStatus;
	uint8_t	retries;
	uint8_t	invokeID;
	uint8_t	dlCh;
}BacnetCOVNode;

extern BacnetCOVNode BacnetCOVSubscribeNode[BACNET_COV_MAX_SUBSCRIPTIONS];

/**
 * @brief  BACnet COV Initialization.
 *
 * @param none
 * @return none
 */
void BacnetCOVInit(void);

/**
 * @brief  BACnet COV Object excution loop, called every 100 ms.
 *
 * @param[in] elapsed_time in ms
 * @return none
 */
void BacnetCOVObj(uint32_t elapsed_time);

/**
 * @brief Subscribe to COV list for BACnet objects.
 *
 * @param[in] src         Source address for subscription.
 * @param[in] cov_data    COV subscription data.
 * @param[out] error_class Error class if subscription fails.
 * @param[out] error_code  Error code if subscription fails.
 * @param[in] bacLink     BACnet link structure.
 * @return true if subscription successful, false otherwise.
 */
bool BacnetCOVListSubscribe(BACNET_ADDRESS * src, BACNET_SUBSCRIBE_COV_DATA * cov_data,
                BACNET_ERROR_CLASS * error_class, BACNET_ERROR_CODE * error_code,
                BACNET_LINK *bacLink);
				uint8_t BacnetCovGetNextFreeInvokeID(void);

/**
 * @brief Handle confirmed COV acknowledgment.
 *
 * @param[in] srcAddr   Source address for the acknowledgment.
 * @param[in] invoke_id Invoke ID of the acknowledgment message.
 * @return none
 */
void BacnetCOVConfirmedAck(BACNET_ADDRESS * srcAddr, uint8_t invoke_id);

/**
 * @brief Encode COV subscribe list by position for read range.
 *
 * @param[out] apdu      APDU buffer to encode into.
 * @param[in] pRequest   Read range request data.
 * @param[in] bacLink    BACnet link structure.
 * @return Number of bytes encoded.
 */
int  BacnetCOVRRSubscribeListEncode(
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest,
    BACNET_LINK * bacLink);
	
#endif  //__BACNETCOV_H_0109855E_0523_47b2_B8BA_A07E781963D5

