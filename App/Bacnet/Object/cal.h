#ifndef __CAL_H__
#define __CAL_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "calendar.h"
#include "readrange.h"

#define TOTALCALENDAROBJECT		2
#define MAXCALENDARENTRIES		16

typedef struct
{
	uint8_t    Name[MAX_CHARACTER_STRING_BYTES];
	BACNET_CALENDAR_ENTRY_LIST dateList[MAXCALENDARENTRIES];
}CALENDAROBJATTR;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void Calendar_Property_Lists(
        uint32_t object_instance,
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool Calendar_Valid_Instance(
        uint32_t object_instance);
    unsigned Calendar_Count(
        void);
    uint32_t Calendar_Index_To_Instance(
        unsigned index);
    unsigned Calendar_Instance_To_Index(
        uint32_t instance);

    bool Calendar_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Calendar_Name_Set(
        uint32_t object_instance,
        char *new_name);

    char *Calendar_Description(
        uint32_t instance);
    bool Calendar_Description_Set(
        uint32_t instance,
        char *new_name);


    int Calendar_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata, BACNET_LINK *bacLink);
    bool Calendar_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data, BACNET_LINK *bacLink);

    void Calendar_Cleanup(void);

    void Calendar_Init(void);

	void CalendarUpdate(int year, int month, int day);

	bool Calendar_Present_Value(uint32_t object_instance);

    void calUpdateDB(uint32_t object_instance);

	int CalendarCompareDate(BACNET_DATE *bNow, BACNET_DATE *bDate, bool bIgnoreWDay);
	bool CalendarIsWithinDateRange(BACNET_DATE *bNow, BACNET_DATE *startDate, BACNET_DATE *endDate);
	int CalendarWeekOfMonth(int day);
    bool CalendarIsActive(BACNET_DATE *now, BACNET_CALENDAR_ENTRY_LIST *dateList);

    bool CalendarGetRRInfo(
        BACNET_READ_RANGE_DATA * pRequest,  /* Info on the request */
        RR_PROP_INFO * pInfo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#define CALENDAR_OBJ_FUNCTIONS \
    OBJECT_CALENDAR, Calendar_Init, Calendar_Count, \
    Calendar_Index_To_Instance, Calendar_Valid_Instance, \
    Calendar_Object_Name, Calendar_Read_Property, Calendar_Write_Property, \
    Calendar_Property_Lists, NULL, NULL, NULL

#endif	//end of __CAL_H__
