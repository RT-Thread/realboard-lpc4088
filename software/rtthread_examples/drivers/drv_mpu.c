#include <rtthread.h>
#include "drv_mpu.h"
#include "LPC407x_8x_177x_8x.h"

/************************** PUBLIC DEFINITIONS *************************/
/* Region size definitions */
#define MPU_REGION_SIZE_32B     0x04
#define MPU_REGION_SIZE_64B     0x05
#define MPU_REGION_SIZE_128B    0x06
#define MPU_REGION_SIZE_256B    0x07
#define MPU_REGION_SIZE_512B    0x08
#define MPU_REGION_SIZE_1KB     0x09
#define MPU_REGION_SIZE_2KB     0x0A
#define MPU_REGION_SIZE_4KB     0x0B
#define MPU_REGION_SIZE_8KB     0x0C
#define MPU_REGION_SIZE_16KB    0x0D
#define MPU_REGION_SIZE_32KB    0x0E
#define MPU_REGION_SIZE_64KB    0x0F
#define MPU_REGION_SIZE_128KB   0x10
#define MPU_REGION_SIZE_256KB   0x11
#define MPU_REGION_SIZE_512KB   0x12
#define MPU_REGION_SIZE_1MB     0x13
#define MPU_REGION_SIZE_2MB     0x14
#define MPU_REGION_SIZE_4MB     0x15
#define MPU_REGION_SIZE_8MB     0x16
#define MPU_REGION_SIZE_16MB    0x17
#define MPU_REGION_SIZE_32MB    0x18
#define MPU_REGION_SIZE_64MB    0x19
#define MPU_REGION_SIZE_128MB   0x1A
#define MPU_REGION_SIZE_256MB   0x1B
#define MPU_REGION_SIZE_512MB   0x1C
#define MPU_REGION_SIZE_1GB     0x1D
#define MPU_REGION_SIZE_2GB     0x1E
#define MPU_REGION_SIZE_4GB     0x1F

/* Access permission definitions */
#define MPU_NO_ACCESS                           0x00
#define MPU_PRIVILEGED_ACESS_USER_NO_ACCESS     0x01
#define MPU_PRIVILEGED_RW_USER_READ_ONLY        0x02
#define MPU_FULL_ACCESS                         0x03
#define MPU_UNPREDICTABLE                       0x04
#define MPU_PRIVILEGED_READ_ONLY_USER_NO_ACCESS 0x05
#define MPU_READ_ONLY                           0x06

/* RASR bit definitions */
#define MPU_RASR_REGION_SIZE(n)         ((uint32_t)(n<<1))
#define MPU_RASR_ACCESS_PERMISSION(n)   ((uint32_t)(n<<24))
#define MPU_REGION_ENABLE               ((uint32_t)(1<<0))



void mpu_init(void)
{
    /* Disable MPU */
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    /* - Region 0: 0x00000000 - 0x0007FFFF --- on-chip non-volatile memory
    *      + Size: 512kB
    *      + Acess permission: full access
    */
    MPU->RNR = 0;//indicate MPU region 0
    MPU->RBAR = 0x00000000; // update the base address for the region 0
    MPU->RASR = MPU_RASR_ACCESS_PERMISSION(MPU_FULL_ACCESS)     //full access
                | MPU_RASR_REGION_SIZE(MPU_REGION_SIZE_512KB)   //512Kb size
                | MPU_REGION_ENABLE;                            //region enable

    /* - Region 1: 0x10000000 - 0x1000FFFF --- on-chip SRAM
     *      + Size: 64kB
     *      + Access permission: full access
     */
    MPU->RNR = 1;
    MPU->RBAR = 0x10000000; // update the base address for the region 1
    MPU->RASR = MPU_RASR_ACCESS_PERMISSION(MPU_FULL_ACCESS)
                | MPU_RASR_REGION_SIZE(MPU_REGION_SIZE_64KB)
                | MPU_REGION_ENABLE;

    /* - Region 2: 0x40000000 - 0x400FFFFF --- APB peripheral
    *      + Size: 1MB
    *      + Access permission: full access
    */
    MPU->RNR = 2;
    MPU->RBAR = 0x40000000; // update the base address for the region 2
    MPU->RASR = MPU_RASR_ACCESS_PERMISSION(MPU_FULL_ACCESS)
                | MPU_RASR_REGION_SIZE(MPU_REGION_SIZE_1MB)
                | MPU_REGION_ENABLE;

    /* - Region 3: 0x20080000 - 0x200BFFFF --- AHB peripheral
    *      + Size: 256KB
    *      + AP=b011: full access
    */
    MPU->RNR = 3;
    MPU->RBAR = 0x20080000; // update the base address for the region 3
    MPU->RASR = MPU_RASR_ACCESS_PERMISSION(MPU_FULL_ACCESS)
                | MPU_RASR_REGION_SIZE(MPU_REGION_SIZE_256KB)
                | MPU_REGION_ENABLE;

    /* - Region 4: 0xE0000000 - 0xE00FFFFF --- System control
    *      + Size: 1MB
    *      + Access permission: full access
    */
    MPU->RNR = 4;
    MPU->RBAR = 0xE0000000; // update the base address for the region 4
    MPU->RASR = MPU_RASR_ACCESS_PERMISSION(MPU_FULL_ACCESS)
                | MPU_RASR_REGION_SIZE(MPU_REGION_SIZE_1MB)
                | MPU_REGION_ENABLE;

    /* - Region 5:0x20000000 - 0x20007FFF --- on chip SRAM
    *      + Size: 32kB
    *      + Access permission: full access
    */
    MPU->RNR = 5;
    MPU->RBAR = 0x20000000; // update the base address for the region 5
    MPU->RASR = MPU_RASR_ACCESS_PERMISSION(MPU_FULL_ACCESS)
                | MPU_RASR_REGION_SIZE(MPU_REGION_SIZE_32KB)
                | MPU_REGION_ENABLE;

    /* - Region 5:0xA0000000 - 0xA2000000 --- external SDRAM
    *      + Size: 32MB
    *      + Access permission: full access
    */
    MPU->RNR = 6;
    MPU->RBAR = 0xA0000000; // update the base address for the region 6
    MPU->RASR = MPU_RASR_ACCESS_PERMISSION(MPU_FULL_ACCESS)
                | MPU_RASR_REGION_SIZE(MPU_REGION_SIZE_32MB)
                | MPU_REGION_ENABLE;

    /* Enable the memory fault exception */
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

    /* Enable MPU */
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
}

