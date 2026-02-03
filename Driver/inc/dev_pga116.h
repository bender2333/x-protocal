#ifndef __DEV_PGA116_H_
#define __DEV_PGA116_H_

#include "MyDevice.h"
#include "EKStdLib.h"

//*****************************************************************************
//
// Macro allowing us to pack the fields of a structure.
//
//*****************************************************************************
#if defined(ccs) ||             \
    defined(codered) ||         \
    defined(gcc) ||             \
    defined(rvmdk) ||           \
    defined(__ARMCC_VERSION) || \
    defined(sourcerygxx)
#define PACKEDSTRUCT __attribute__ ((packed))
#elif defined(ewarm)
#define PACKEDSTRUCT
#else
#error Unrecognized COMPILER!
#endif

// instruction register bits
#define PGA116_INSTR_WRR   (0x2A<<8)  // write to a register
#define PGA116_INSTR_RDR   (0x6A<<8)  // Read register

// gain register bits
#define PGA116_GAINR_01    (0 << 4)     // gain +1
#define PGA116_GAINR_02    (1 << 4)     // gain +2
#define PGA116_GAINR_04    (2 << 4)     // gain +4
#define PGA116_GAINR_08    (3 << 4)     // gain +8
#define PGA116_GAINR_16    (4 << 4)     // gain +16
#define PGA116_GAINR_32    (5 << 4)     // gain +32
#define PGA116_GAINR_64    (6 << 4)     // gain +64
#define PGA116_GAINR_128   (7 << 4)     // gain +128
      
// channel register bits
#define PGA116_CHNR_00     (0)     // channel 0
#define PGA116_CHNR_01     (1)     // channel 1
#define PGA116_CHNR_02     (2)     // channel 2
#define PGA116_CHNR_03     (3)     // channel 3
#define PGA116_CHNR_04     (4)     // channel 4
#define PGA116_CHNR_05     (5)     // channel 5
#define PGA116_CHNR_06     (6)     // channel 6
#define PGA116_CHNR_07     (7)     // channel 7
#define PGA116_CHNR_08     (8)     // channel 8
#define PGA116_CHNR_09     (9)     // channel 9

#define DEV_PGA116_CH_MAX  (8)

extern void dev_pga116_init(void);
extern void dev_pga116_selectChannelGain(u32_t ch, u32_t gain);

#endif
