#ifndef __AUTOMATIONOPTS_H_46FFA867_2906_47f8_9FE1_791918F91A0C
#define __AUTOMATIONOPTS_H_46FFA867_2906_47f8_9FE1_791918F91A0C

//////// IO ////////

//////////////// Analogue Input ///////////////////
        #define TOTALANALOGUEINPUTS		8

        #define TOTALAIDITYPE           1
        #define TOTALAICURRENTTYPE      2
        #define TOTALAIVOLTAGETTYPE     2
        #define TOTALAIRESISTANCETYPE   3
        #define TOTALAITEMPERATURETYPE  3
//////////////// Analogue Input ///////////////////

//////////////// Digital Input ////////////////////
        #define TOTALDIGITALINPUTS	TOTALANALOGUEINPUTS
        #define SUPPORTDI_EXTPORT       TOTALDIGITALINPUTS

        #ifdef SUPPORTDI_EXTPORT
            #define DIGETEXTPORT(channel) AnalogueInputGetADI(channel)
        #endif
//////////////// Digital Input ////////////////////    

//////////////// Analogue Output //////////////////
        #define TOTALANALOGUEOUTPUTS	6
//////////////// Analogue Output //////////////////

//////////////// Digital Output ///////////////////
        #define TOTALDIGITALOUTPUTS	TOTALANALOGUEOUTPUTS
//////////////// Digital Output ///////////////////

/////////////////////////// UIO ///////////////////////
        #define TOTALUIO	        4
/////////////////////////// UIO ///////////////////////

//////////////// Pulse Accumulator ////////////////
        #define TOTALPULSEACCUM		(TOTALDIGITALINPUTS + TOTALUIO)
//////////////// Pulse Accumulator ////////////////

//////////////// Modbus Master  ////////////////
#define MAX_SLAVE_DEVICE		7

#define MAX_SLAVE_DO			3
#define MAX_SLAVE_DI			16
#define MAX_SLAVE_AO			3
#define MAX_SLAVE_AI			5
#define MAX_SLAVE_UIO			4

#define MAX_MASTER_DO			(MAX_SLAVE_DO * MAX_SLAVE_DEVICE)
#define MAX_MASTER_DI			(MAX_SLAVE_DI * MAX_SLAVE_DEVICE)
#define MAX_MASTER_AO			(MAX_SLAVE_AO * MAX_SLAVE_DEVICE)
#define MAX_MASTER_AI			(MAX_SLAVE_AI * MAX_SLAVE_DEVICE)
#define MAX_MASTER_UIO			(MAX_SLAVE_UIO * MAX_SLAVE_DEVICE)
//////////////// Modbus Master  ////////////////

#endif	// end of __AUTOMATIONOPTS_H_46FFA867_2906_47f8_9FE1_791918F91A0C
