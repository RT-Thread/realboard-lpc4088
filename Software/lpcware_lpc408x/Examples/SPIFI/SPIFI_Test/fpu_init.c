#include "stdint.h"

#define	LPC_CPACR	        0xE000ED88               

#define SCB_MVFR0           0xE000EF40
#define SCB_MVFR0_RESET     0x10110021

#define SCB_MVFR1           0xE000EF44
#define SCB_MVFR1_RESET     0x11000011

void fpu_init(void)
{
// from arm trm manual:
//                ; CPACR is located at address 0xE000ED88
//                LDR.W R0, =0xE000ED88
//                ; Read CPACR
//                LDR R1, [R0]
//                ; Set bits 20-23 to enable CP10 and CP11 coprocessors
//                ORR R1, R1, #(0xF << 20)
//                ; Write back the modified value to the CPACR
//                STR R1, [R0]

                
    volatile uint32_t* regCpacr = (uint32_t*) LPC_CPACR;
    volatile uint32_t* regMvfr0 = (uint32_t*) SCB_MVFR0;
    volatile uint32_t* regMvfr1 = (uint32_t*) SCB_MVFR1;
    volatile uint32_t Cpacr;
    volatile uint32_t Mvfr0;
    volatile uint32_t Mvfr1;    
    char vfpPresent = 0;

    Mvfr0 = *regMvfr0;
    Mvfr1 = *regMvfr1;

    vfpPresent = ((SCB_MVFR0_RESET == Mvfr0) && (SCB_MVFR1_RESET == Mvfr1));
    
    if(vfpPresent)
    {
        Cpacr = *regCpacr;
        Cpacr |= (0xF << 20);
        *regCpacr = Cpacr;   // enable CP10 and CP11 for full access
    }

}

//    #if defined ( __CC_ARM   )      // arm compiler   

//        #endif
//    #elif defined ( __ICCARM__ )    // iar compiler

//        #endif    
//    #elif defined   (  __GNUC__  )  // gnu compiler

//        #endif
//    #elif defined   (  __TASKING__  )   // tasking compiler

//       #endif
//    #endif
