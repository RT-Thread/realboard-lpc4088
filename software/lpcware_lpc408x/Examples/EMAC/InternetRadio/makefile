######################################################################## 
# $Id:: makefile 1516 2008-12-17 00:28:46Z pdurgesh                    $
# 
# Project: Debugger loadable example makefile
#
# Notes:
#     This type of image is meant to be loaded and executed through a
#     debugger and will not run standalone and cannot be FLASHed into
#     the board.
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
PROJ_ROOT			= $(CURDIR)/../../..
EXECNAME    = emac_internet_radio

########################################################################
#
# Pick up the configuration file in make section
#
########################################################################
include ../../../makesection/makeconfig 

EXDIRINC	=$(CURDIR)
# Include directory
TMPPATH 		+= -I$(EXDIRINC)/common \
				-I$(EXDIRINC)/lib \
				-I$(EXDIRINC)/lib/mp3codec \
				-I$(EXDIRINC)/lib/mp3codec/pub \
				-I$(EXDIRINC)/lpc_port \
				-I$(EXDIRINC)/uip 
CTMPFLAGS += -DDHCP_ENABLE				
	
# Create a list of aditional files will be compiled
# Retarget object files
ADDOBJS		+=  $(EXDIRINC)/common/Serial.o \
				$(EXDIRINC)/common/printf-stdarg.o \
				$(EXDIRINC)/common/syscalls.o 

				# uIP 'uip' files
ADDOBJS		+=  $(EXDIRINC)/uip/psock.o \
				$(EXDIRINC)/uip/timer.o \
				$(EXDIRINC)/uip/uip.o \
				$(EXDIRINC)/uip/uip_arp.o 
# 'lpc17xx_port' files 								
ADDOBJS		+=  $(EXDIRINC)/lpc_port/emac.o \
				$(EXDIRINC)/lpc_port/clock-arch.o \
				$(EXDIRINC)/lpc_port/decoder.o \
				$(EXDIRINC)/lpc_port/Emac_uIP.o 
				
TMPPATH 		+= -I$(EXDIRINC)/apps/webclient \
				-I$(EXDIRINC)/apps/webbrowser \
				-I$(EXDIRINC)/apps/dhcpc \
				-I$(EXDIRINC)/apps/resolv		

# uIP 'apps' files
ADDOBJS		+=  $(EXDIRINC)/apps/webclient/webclient.o \
				$(EXDIRINC)/apps/webclient/webclient-strings.o \
				$(EXDIRINC)/apps/dhcpc/dhcpc.o \
				$(EXDIRINC)/apps/resolv/resolv.o \
				$(EXDIRINC)/uip/uiplib.o

# uIP 'lib' files				
ADDOBJS		+=  $(EXDIRINC)/lib/mp3codec/bitstream.o \
				$(EXDIRINC)/lib/mp3codec/buffers.o \
				$(EXDIRINC)/lib/mp3codec/dct32.o \
				$(EXDIRINC)/lib/mp3codec/dequant.o \
				$(EXDIRINC)/lib/mp3codec/dqchan.o \
				$(EXDIRINC)/lib/mp3codec/huffman.o \
				$(EXDIRINC)/lib/mp3codec/hufftabs.o \
				$(EXDIRINC)/lib/mp3codec/imdct.o \
				$(EXDIRINC)/lib/mp3codec/mp3dec.o \
				$(EXDIRINC)/lib/mp3codec/mp3tabs.o \
				$(EXDIRINC)/lib/mp3codec/scalfact.o \
				$(EXDIRINC)/lib/mp3codec/stproc.o \
				$(EXDIRINC)/lib/mp3codec/subband.o \
				$(EXDIRINC)/lib/mp3codec/trigtabs.o \
				$(EXDIRINC)/lib/mp3codec/polyphase.o				
include $(PROJ_ROOT)/makesection/makerule/example/makefile.ex
