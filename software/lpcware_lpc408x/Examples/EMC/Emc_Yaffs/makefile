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
EXECNAME    =emc_yaffs
CUSTOM_LINKER_SCRIPT	= 1
CUSTOM_STARTUP			= 1


########################################################################
#
# Pick up the configuration file in make section
#
########################################################################
include ../../../makesection/makeconfig 

EXDIRINC	=$(CURDIR)

LDSCRIPTIROM = $(EXDIRINC)/GCC/ldscript_rom_gnu.ld

# Startup
ADDOBJSS    += $(EXDIRINC)/GCC/startup_LPC407x_8x_177x_8x.o	

# Include directory
TMPPATH 		+= -I$(EXDIRINC)/yaffs2 \
				-I$(EXDIRINC)/yaffs2/direct \
				-I$(EXDIRINC)/yaffs2/direct/basic-test 

ADDOBJS		+=  $(EXDIRINC)/yaffs2/direct/yaffsfs.o \
				$(EXDIRINC)/yaffs2/yaffs_guts.o \
				$(EXDIRINC)/yaffs2/yaffs_nand.o \
				$(EXDIRINC)/yaffs2/direct/yaffs_nandif.o \
				$(EXDIRINC)/yaffs2/yaffs_tagsvalidity.o \
				$(EXDIRINC)/yaffs2/yaffs_checkptrw.o \
				$(EXDIRINC)/yaffs2/yaffs_qsort.o \
				$(EXDIRINC)/yaffs2/yaffs_tagscompat.o \
				$(EXDIRINC)/yaffs2/yaffs_ecc.o \
				$(EXDIRINC)/yaffs2/yaffs_packedtags2.o \
				$(EXDIRINC)/yaffs2/direct/basic-test/yaffsnewcfg.o \
				$(EXDIRINC)/yaffs2/direct/basic-test/ynandsim.o \
				$(EXDIRINC)/yaffs2/yaffs_yaffs1.o \
				$(EXDIRINC)/yaffs2/yaffs_yaffs2.o \
				$(EXDIRINC)/yaffs2/yaffs_allocator.o \
				$(EXDIRINC)/yaffs2/yaffs_nameval.o \
				$(EXDIRINC)/yaffs2/yaffs_verify.o \
				$(EXDIRINC)/yaffs2/yaffs_bitmap.o				
include $(PROJ_ROOT)/makesection/makerule/example/makefile.ex
