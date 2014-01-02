######################################################################## 
# $Id:: make.lpc32xx.gnu 635 2008-04-18 21:52:52Z wellsk               $
# 
# Project: LH7A404 toolset rules for GNU toolset
# 
# Description: 
#     Make rules for the GNU toolset
# 
######################################################################## 
# Software that is described herein is for illustrative purposes only  
# which provides customers with programming information regarding the  
# products. This software is supplied "AS IS" without any warranties.  
# NXP Semiconductors assumes no responsibility or liability for the 
# use of the software, conveys no license or title under any patent, 
# copyright, or mask work right to the product. NXP Semiconductors 
# reserves the right to make changes in the software without 
# notification. NXP Semiconductors also make no representation or 
# warranty that such application will be suitable for the specified 
# use without further testing or modification. 
########################################################################

include $(PROJ_ROOT)/makesection/makerule/common/make.rules.environment

GNUTOOLS		='$(GNU_INSTALL_ROOT)/bin'
THUMB2GNULIB 	='$(GNU_INSTALL_ROOT)/lib/gcc/arm-none-eabi/$(GNU_VERSION)/thumb2'
THUMB2GNULIB2	='$(GNU_INSTALL_ROOT)/arm-none-eabi/lib/thumb2'

OPTIM			= 0

#===================== C compile flag ============================
CFLAGS    		= -c 
CFLAGS			+= -mcpu=$(CPU) 
CFLAGS			+= -mthumb 
CFLAGS			+= -Wall 
CFLAGS			+= -O$(OPTIM) 
CFLAGS			+= -mapcs-frame 
CFLAGS			+= -D__thumb2__=1 
CFLAGS	 		+= -msoft-float 
CFLAGS			+= -gdwarf-2 
CFLAGS   		+= -mno-sched-prolog 
CFLAGS			+= -fno-hosted 
ifeq ($(CPU), cortex-m3)
CFLAGS			+= -mtune=$(CPU) 
endif
ifeq ($(CPU), cortex-m3)
CFLAGS			+= -march=armv7-m 
CFLAGS			+= -mfix-$(CPU)-ldrd  
else
CFLAGS			+= -march=armv7e-m 
endif
CFLAGS   		+= -ffunction-sections 
CFLAGS			+= -fdata-sections 

#================= note =================================
#CFLAGS			+= -mthumb-interwork  
#CFLAGS			+= -mno-bit-align 
#CFLAGS			+= mstructure-size-boundary=8
#CFLAGS			+= -Wpacked
#CFLAGS			+= -Wpadded
#CFLAGS			+= -fpack-struct=0 

#================ note2 ================================
#CFLAGS			+= -fno-builtin
#CFLAGS			+= -fno-strict-aliasing  
#CFLAGS			+= -D PACK_STRUCT_END=__attribute\(\(packed\)\)
#CFLAGS			+= -D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)
#CFLAGS			+= -fmessage-length=0 
#CFLAGS			+= -funsigned-char 
#CFLAGS			+= -Wextra 
#CFLAGS			+= -MMD 
#CFLAGS			+= -MP 
#CFLAGS			+= -MF"$(@:%.o=%.d)" 
#CFLAGS			+= -MT"$(@:%.o=%.d)" 

#================ Build Folder Include ========================
CFLAGS                  += $(TOOLDIRINC)
CFLAGS   		+= -I$(FWLIB_INC_DIR)
CFLAGS   		+= -I$(CMCORE_INC_DIR) -I$(DEVICE_INC_DIR) -I$(BOARD_SUPPORT_INC_DIR)

#================ Asm compile flag ========================
AFLAGS    		= -mcpu=$(CPU) 
AFLAGS   		+= -I$(FWLIB_INC_DIR) -I$(CMCORE_INC_DIR) -I$(DEVICE_INC_DIR)  -I$(BOARD_SUPPORT_INC_DIR) -gdwarf-2 



#CC		 		= $(GNUTOOLS)/arm-none-eabi-gcc
CC       		= $(GNUTOOLS)/arm-none-eabi-gcc-$(GNU_VERSION)
AS       		= $(GNUTOOLS)/arm-none-eabi-as
AR       		= $(GNUTOOLS)/arm-none-eabi-ar -r
LD       		= $(GNUTOOLS)/arm-none-eabi-gcc
NM       		= $(GNUTOOLS)/arm-none-eabi-nm
OBJDUMP  		= $(GNUTOOLS)/arm-none-eabi-objdump
OBJCOPY  		= $(GNUTOOLS)/arm-none-eabi-objcopy
READELF  		= $(GNUTOOLS)/arm-none-eabi-readelf
CODESIZE 		= $(GNUTOOLS)/arm-none-eabi-size


#================ LD flag ========================
#LDFLAGS  		= -nostartfiles 
#LDFLAGS 		+= -nodefaultlibs 
#LDFLAGS 		+= -nostdlib 


LK       		=  -static -mcpu=$(CPU) -mthumb -mthumb-interwork
#LK			= -static
#LK       		+= -Wl,--start-group $(TARGET_FWLIB_LIB) 
LK       		+= -Wl,--start-group 
LK 				+= -L$(THUMB2GNULIB) -L$(THUMB2GNULIB2)
LK       		+= -lc -lg -lstdc++ -lsupc++ 
LK 				+= -lgcc -lm 
LK       		+= -Wl,--end-group 
#LK       		+= -Wl,--gc-sections -Wl,--print-gc-sections 



MAP      		= -Xlinker -Map -Xlinker
LDESC    		= -Xlinker -T  
ENTRY    		= -e
BIN      		= -bin
EXT      		=.elf
LEXT     		= 
REC      		=.srec
HEX		 		=.hex
