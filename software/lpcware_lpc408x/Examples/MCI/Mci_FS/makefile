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
EXECNAME    =mci_fs_test

EXDIRINC	=$(CURDIR)

#FSLIB       = EFSL
FSLIB       = FatFs
# Include directory
ifeq ($(FSLIB), FatFs)
TMPPATH 		+= -I$(EXDIRINC)/FatFs
ADDOBJS		    +=  $(EXDIRINC)/FatFs/ff.o \
					$(EXDIRINC)/FatFs/option/ccsbcs.o \
					$(EXDIRINC)/main/fatfs_main.o \
					$(EXDIRINC)/main/fs_mci.o\
					$(EXDIRINC)/main/monitor.o
else
TMPPATH 		+= -I$(EXDIRINC)/efsl/conf \
					-I$(EXDIRINC)/efsl/inc \
					-I$(EXDIRINC)/efsl/inc/interfaces \
					-I$(EXDIRINC)/main
ADDOBJS		    +=  $(EXDIRINC)/efsl/src/debug.o \
					$(EXDIRINC)/efsl/src/dir.o \
					$(EXDIRINC)/efsl/src/disc.o \
					$(EXDIRINC)/efsl/src/efs.o \
					$(EXDIRINC)/efsl/src/extract.o \
					$(EXDIRINC)/efsl/src/fat.o \
					$(EXDIRINC)/efsl/src/file.o \
					$(EXDIRINC)/efsl/src/fs.o \
					$(EXDIRINC)/efsl/src/ioman.o \
					$(EXDIRINC)/efsl/src/ls.o \
					$(EXDIRINC)/efsl/src/mkfs.o \
					$(EXDIRINC)/efsl/src/partition.o \
					$(EXDIRINC)/efsl/src/plibc.o \
					$(EXDIRINC)/efsl/src/time.o \
					$(EXDIRINC)/efsl/src/ui.o \
					$(EXDIRINC)/efsl/src/interfaces/if_lpc177x_8x.o \
					$(EXDIRINC)/main/efsl_main.o \
					$(EXDIRINC)/main/fs_mci.o \
					$(EXDIRINC)/main/monitor.o
endif


########################################################################
#
# Pick up the configuration file in make section
#
########################################################################
include ../../../makesection/makeconfig 
include $(PROJ_ROOT)/makesection/makerule/example/makefile.ex
