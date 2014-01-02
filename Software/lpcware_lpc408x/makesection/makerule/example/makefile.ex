######################################################################## 
# $Id:: makefile.ex 1388 2008-12-01 19:20:06Z pdurgesh                 $
# 
# Project: Example makefile
#
# Notes:
#     This makefile generates an image that will load in SDRAM at
#     address 0x80000000. The image can be loaded and run with the
#     stage 1 loader or with a debugger.
#
# Description: 
#  Makefile
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

########################################################################
#
# Pick up the configuration file in make section
#
########################################################################
#include ../../makeconfig

########################################################################
#
# Pick up the default build rules 
#
########################################################################

include $(PROJ_ROOT)/makesection/makerule/LPC/make.LPC.$(TOOL)

########################################################################
#
# Pick up the assembler and C source files in the directory
# Included Cortex-Mx core files  
#
########################################################################
include $(PROJ_ROOT)/makesection/makerule/common/make.rules.ftypes
#ADDOBJS     += $(PROJ_ROOT)/Core/$(CMCORE_TYPE)/CoreSupport/core_cm3.o 
#ADDOBJS     += $(PROJ_ROOT)/Core/$(CMCORE_TYPE)/DeviceSupport/$(MANUFACTURE)/$(DEVICE)/system_$(DEVICE).o
ADDOBJS     += $(PROJ_ROOT)/Core/Device/$(MANUFACTURE)/$(DEVICE)/Source/Templates/system_$(DEVICE).o  

########################################################################
#
# For some Flag defined in header file in Example directory  
#
########################################################################
CFLAGS += -I$(EXDIRINC) $(TMPPATH) 

# Define __BUILD_WITH_EXAMPLE__ symbol in this case (build example mode)
CFLAGS += -D__BUILD_WITH_EXAMPLE__=1 $(CTMPFLAGS)


########################################################################
#
# Object file in FW library directory  
#
########################################################################
FWSRC 	= $(wildcard $(PROJ_ROOT)/$(FWLIB)/source/*.c)
FWSRCC  = $(wildcard $(PROJ_ROOT)/$(FWLIB)/source/*.cpp)
FWASM	= $(wildcard $(PROJ_ROOT)/$(FWLIB)/source/*.$(ASM_EXT))
FWOBJS  = $(FWSRC:%.c=%.o) $(FWSRCC:%.c=%.o) $(FWASM:%.$(ASM_EXT)=%.o)

########################################################################
#
# Object file in Board Support directory  
#
########################################################################
BOARDSRC 	= $(wildcard $(PROJ_ROOT)/BoardSupport/*.c)
BOARDSRCC  = $(wildcard $(PROJ_ROOT)/BoardSupport/*.cpp)
BOARDASM	= $(wildcard $(PROJ_ROOT)/BoardSupport/*.$(ASM_EXT))
BOARDOBJS  = $(BOARDSRC:%.c=%.o) $(BOARDSRCC:%.c=%.o) $(BOARDASM:%.$(ASM_EXT)=%.o)

########################################################################
#
# GNU compiler/linker specific stuff
#
########################################################################

ifeq ($(TOOL), gnu)

MEXT        =.map
MAPFILE     =$(EXECNAME)
#CFLAGS      +=-gdwarf-2
AFLAGS 		+=-gdwarf-2
ifneq ($(CUSTOM_STARTUP), 1)
ADDOBJSS    += $(PROJ_ROOT)/Core/Device/$(MANUFACTURE)/$(DEVICE)/Source/Templates/GCC/startup_$(DEVICE).o
endif

# Define Required Linker script file in each build mode
ifneq ($(CUSTOM_LINKER_SCRIPT), 1)
LDSCRIPTIROM = $(PROJ_ROOT)/makesection/makerule/linker/ldscript_rom_$(TOOL).ld
endif
LDSCRIPTIRAM = $(PROJ_ROOT)/makesection/makerule/linker/ldscript_ram_$(TOOL).ld

endif

########################################################################
#
# Arm compiler/linker specific stuff
#
# ARM examples enter via __main and are linked at address 0x80000000
#
########################################################################

ifeq ($(TOOL), ads)
MEXT        =.map
MAPFILE     =$(EXECNAME)
CFLAGS      +=-g
AFLAGS      +=-g
LDSCRIPT    =../linker/ldscript_iram_ads.ld
ADDOBJS     += pad.o
endif
ifeq ($(TOOL), rvw)
MEXT        =.map
MAPFILE     =$(EXECNAME)
CFLAGS      +=-g
AFLAGS      +=-g
LDSCRIPT    =../linker/ldscript_iram_ads.ld
ADDOBJS     += pad.o
endif
ifeq ($(TOOL), keil)
MEXT        =.map
MAPFILE     =$(EXECNAME)
CFLAGS      +=-g
AFLAGS      +=-g
LDSCRIPT    =../linker/ldscript_iram_ads.ld
ADDOBJS     += pad.o
endif

########################################################################
#
# IAR compiler/linker specific stuff
#
########################################################################

ifeq ($(TOOL), iar)
MEXT        =.map
MAPFILE     =$(EXECNAME)
CFLAGS      +=
AFLAGS      +=
LDESC       = --config
LDSCRIPT    =../linker/ldscript_iram_$(TOOL).icf
ADDOBJS     += 
endif

########################################################################
#
# Rules to build the executable 
#
########################################################################

default: rom

ifeq ($(TOOL), gnu)

ifeq ($(CPU), cortex-m3)
rom: rom_m3
ram: ram_m3
else
rom: rom_m4
ram: ram_m4
endif

rom_m3: LDSCRIPT=$(LDSCRIPTIROM)
rom_m3: AFLAGS += --defsym RAM_MODE=0 
rom_m3: clean_rom_m3 debug_status  $(OBJS) $(ADDOBJS) $(ADDOBJSS)  $(FWOBJS) $(BOARDOBJS)
	$(LD) $(OBJS) $(ADDOBJS) $(ADDOBJSS) $(FWOBJS) $(BOARDOBJS) $(LDFLAGS) $(LK) $(SCAN) $(MAP) \
	$(MAPFILE)$(MEXT) $(LDESC) $(LDSCRIPT) -o $(EXECNAME)$(EXT)
	$(ELFTOHEX) $(EXECNAME)$(EXT) $(EXECNAME)$(HEX)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME)$(REC)
#	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EFLTBINOPT) $(EXECNAME).bin
	$(MKDIR) GCC/Flash_M3
	$(MV) $(MAPFILE)$(MEXT) GCC/Flash_M3/$(MAPFILE)$(MEXT)
	$(MV) $(EXECNAME)$(EXT) GCC/Flash_M3/$(EXECNAME)$(EXT)
	$(MV) $(EXECNAME)$(HEX) GCC/Flash_M3/$(EXECNAME)$(HEX)
	$(MV) $(EXECNAME)$(REC) GCC/Flash_M3/$(EXECNAME)$(REC)
	$(CODESIZE) GCC/Flash_M3/$(EXECNAME)$(EXT)
	
rom_m4: LDSCRIPT=$(LDSCRIPTIROM)
rom_m4: AFLAGS += --defsym RAM_MODE=0 
rom_m4: CFLAGS += -DCORE_M4=1 
rom_m4: clean_rom_m4 debug_status  $(OBJS) $(ADDOBJS) $(ADDOBJSS)  $(FWOBJS) $(BOARDOBJS)
	$(LD) $(OBJS) $(ADDOBJS) $(ADDOBJSS) $(FWOBJS) $(BOARDOBJS) $(LDFLAGS) $(LK) $(SCAN) $(MAP) \
	$(MAPFILE)$(MEXT) $(LDESC) $(LDSCRIPT) -o $(EXECNAME)$(EXT)
	$(ELFTOHEX) $(EXECNAME)$(EXT) $(EXECNAME)$(HEX)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME)$(REC)
#	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EFLTBINOPT) $(EXECNAME).bin
	$(MKDIR) GCC/Flash_M4
	$(MV) $(MAPFILE)$(MEXT) GCC/Flash_M4/$(MAPFILE)$(MEXT)
	$(MV) $(EXECNAME)$(EXT) GCC/Flash_M4/$(EXECNAME)$(EXT)
	$(MV) $(EXECNAME)$(HEX) GCC/Flash_M4/$(EXECNAME)$(HEX)
	$(MV) $(EXECNAME)$(REC) GCC/Flash_M4/$(EXECNAME)$(REC)
	$(CODESIZE) GCC/Flash_M4/$(EXECNAME)$(EXT)	

ram_m3: LDSCRIPT=$(LDSCRIPTIRAM) 
ram_m3: AFLAGS += --defsym RAM_MODE=1
ram_m3: CFLAGS += -D__RAM_MODE__=1 
ram_m3: clean_ram_m3 debug_status $(OBJS) $(ADDOBJS) $(ADDOBJSS) $(FWOBJS) $(BOARDOBJS)
	$(LD) $(OBJS) $(ADDOBJS) $(ADDOBJSS) $(FWOBJS) $(BOARDOBJS) $(LDFLAGS) $(LK) $(SCAN) $(MAP) \
	$(MAPFILE)$(MEXT) $(LDESC) $(LDSCRIPT) -o $(EXECNAME)$(EXT) 
	$(ELFTOHEX) $(EXECNAME)$(EXT) $(EXECNAME)$(HEX)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME)$(REC)
#	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EFLTBINOPT) $(EXECNAME).bin
	$(MKDIR) GCC/Ram_M3
	$(MV) $(MAPFILE)$(MEXT) GCC/Ram_M3/$(MAPFILE)$(MEXT)
	$(MV) $(EXECNAME)$(EXT) GCC/Ram_M3/$(EXECNAME)$(EXT)
	$(MV) $(EXECNAME)$(HEX) GCC/Ram_M3/$(EXECNAME)$(HEX)
	$(MV) $(EXECNAME)$(REC) GCC/Ram_M3/$(EXECNAME)$(REC)
	$(CODESIZE) GCC/Ram_M3/$(EXECNAME)$(EXT)
	
ram_m4: LDSCRIPT=$(LDSCRIPTIRAM) 
ram_m4: AFLAGS += --defsym RAM_MODE=1
ram_m4: CFLAGS += -D__RAM_MODE__=1 
ram_m4: CFLAGS += -DCORE_M4=1 
ram_m4: clean_ram_m4 debug_status $(OBJS) $(ADDOBJS) $(ADDOBJSS) $(FWOBJS) $(BOARDOBJS)
	$(LD) $(OBJS) $(ADDOBJS) $(ADDOBJSS) $(FWOBJS) $(BOARDOBJS) $(LDFLAGS) $(LK) $(SCAN) $(MAP) \
	$(MAPFILE)$(MEXT) $(LDESC) $(LDSCRIPT) -o $(EXECNAME)$(EXT) 
	$(ELFTOHEX) $(EXECNAME)$(EXT) $(EXECNAME)$(HEX)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME)$(REC)
#	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EFLTBINOPT) $(EXECNAME).bin
	$(MKDIR) GCC/Ram_M4
	$(MV) $(MAPFILE)$(MEXT) GCC/Ram_M4/$(MAPFILE)$(MEXT)
	$(MV) $(EXECNAME)$(EXT) GCC/Ram_M4/$(EXECNAME)$(EXT)
	$(MV) $(EXECNAME)$(HEX) GCC/Ram_M4/$(EXECNAME)$(HEX)
	$(MV) $(EXECNAME)$(REC) GCC/Ram_M4/$(EXECNAME)$(REC)
	$(CODESIZE) GCC/Ram_M4/$(EXECNAME)$(EXT)	
endif


# Print DEBUG MODE Status
debug_status:
	$(ECHO) "DEBUG MODE Status -->" $(DEBUG_MODE)

clean_objs: realclean lpc_clean
	@$(RM) $(ADDOBJS)
	@$(RM) $(ADDOBJSS)
	@$(RM) $(FWOBJS)
	@$(RM) $(BOARDOBJS)
	
clean_ram_m3: clean_objs
	@$(RMDIR) "GCC/Ram_M3"	
	
clean_ram_m4: clean_objs
	@$(RMDIR) "GCC/Ram_M4"		

clean_rom_m3: clean_objs
	@$(RMDIR) "GCC/Flash_M3"

clean_rom_m4: clean_objs
	@$(RMDIR) "GCC/Flash_M4"
	
cleanall: clean_ram clean_rom
	@$(RMDIR) "GCC"	

########################################################################
#
# Pick up the compiler and assembler rules
#
########################################################################

include $(PROJ_ROOT)/makesection/makerule/common/make.rules.build

.PHONY: debug_status 