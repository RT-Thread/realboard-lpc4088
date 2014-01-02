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
PROJ_ROOT	= $(CURDIR)/../../..
EXECNAME    =lcd_test
EXDIRINC	=$(CURDIR)

# Include directory
TMPPATH 		+= -I$(EXDIRINC)/app  
# Create a list of aditional files will be compiled
# Retarget object files
ADDOBJS		+=  $(EXDIRINC)/app/Cursor.o \
				$(EXDIRINC)/app/main.o \
				$(EXDIRINC)/app/NXP_logo_16.o \
				$(EXDIRINC)/app/NXP_logo_24.o \
				$(EXDIRINC)/app/NXP_logo_2.o
########################################################################
#
# Pick up the configuration file in make section
#
########################################################################
include ../../../makesection/makeconfig 
include $(PROJ_ROOT)/makesection/makerule/example/makefile.ex
