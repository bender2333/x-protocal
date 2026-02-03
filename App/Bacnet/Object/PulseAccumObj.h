/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
* Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#ifndef _PULSEACCUMOBJ_H_
#define _PULSEACCUMOBJ_H_

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
	void PulseAccumulator_Init(
		void);
    void PulseAccumulator_Property_Lists(
        uint32_t object_instance,
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool PulseAccumulator_Valid_Instance(
        uint32_t object_instance);
    unsigned PulseAccumulator_Count(
        void);
    uint32_t PulseAccumulator_Index_To_Instance(
        unsigned index);
    unsigned PulseAccumulator_Instance_To_Index(
        uint32_t instance);
    bool PulseAccumulator_Object_Instance_Add(
        uint32_t instance);

    bool PulseAccumulator_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool PulseAccumulator_Name_Set(
        uint32_t object_instance,
        char *new_name);

    char *PulseAccumulator_Description(
        uint32_t instance);
    bool PulseAccumulator_Description_Set(
        uint32_t instance,
        char *new_name);

    int PulseAccumulator_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata, BACNET_LINK *bacLink);
    bool PulseAccumulator_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data, BACNET_LINK *bacLink);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif