#ifndef __CALENDAR_H__
#define __CALENDAR_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bacdef.h"
#include "bacstr.h"
#include "bacenum.h"

#define CALENDAR_DATE		0
#define CALENDAR_DATERANGE	1
#define CALENDAR_WEEKNDAY	2
#define CALENDAR_UNDEFINE	0xFF

typedef struct 
{
   BACNET_DATE startDate;
   BACNET_DATE endDate;
}BACNET_DATE_RANGE;


typedef union
{
   BACNET_DATE bdate; /* 0 */
   BACNET_DATE_RANGE dateRange; /* 1 */
   BACNET_OCTET_STRING weekNDay; /* 2 */
}BACNET_CALENDAR_ENTRY;


typedef struct _BACnetCalendarListEntry
{
   uint8_t tag; /* 0, 1, or 2 */
   BACNET_CALENDAR_ENTRY calendar;
   //struct _BACnetCalendarListEntry *next;		//for link list type
}BACNET_CALENDAR_ENTRY_LIST;

int calendar_encode_entry_list(uint8_t *apdu, BACNET_CALENDAR_ENTRY_LIST *date_list, uint16_t totalEntryList, BACNET_LINK *bacLink);
int calendar_decode_entry_list(uint8_t *apdu, int apdu_len, BACNET_CALENDAR_ENTRY_LIST *pDateList, uint16_t totalEntryList, BACNET_LINK *bacLink);
int calendar_encode_entry(uint8_t *apdu, BACNET_CALENDAR_ENTRY_LIST *entry, BACNET_LINK *bacLink);
int calendar_decode_entry(uint8_t *apdu, int apdu_len, BACNET_CALENDAR_ENTRY_LIST *pDateList, BACNET_LINK *bacLink);

#endif	//end of __CALENDAR_H__