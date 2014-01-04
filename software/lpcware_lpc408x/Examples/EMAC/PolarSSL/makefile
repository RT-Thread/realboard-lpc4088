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
EXECNAME    = emac_polar_ssl
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

# Include directory
TMPPATH 		+= -I$(EXDIRINC)/common \
				-I$(EXDIRINC)/lib \
				-I$(EXDIRINC)/polarssl/include \
				-I$(EXDIRINC)/lpc_port \
				-I$(EXDIRINC)/uip 
#CTMPFLAGS += -DDHCP_ENABLE				
	
# Create a list of aditional files will be compiled
# Retarget object files
ADDOBJS		+=  $(EXDIRINC)/common/Serial.o \
				$(EXDIRINC)/common/printf-stdarg.o \
				$(EXDIRINC)/common/syscalls.o 

# Startup
ADDOBJSS    += $(EXDIRINC)/GCC/startup_LPC407x_8x_177x_8x.o				
				# uIP 'uip' files
ADDOBJS		+=  $(EXDIRINC)/uip/psock.o \
				$(EXDIRINC)/uip/timer.o \
				$(EXDIRINC)/uip/uip.o \
				$(EXDIRINC)/uip/uip_arp.o 
# 'lpc17xx_port' files 								
ADDOBJS		+=  $(EXDIRINC)/lpc_port/emac.o \
				$(EXDIRINC)/lpc_port/clock-arch.o \
				$(EXDIRINC)/lpc_port/ssl_client1.o \
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

# polarssl files				
ADDOBJS		+=  $(EXDIRINC)/polarssl/library/aes.o \
				$(EXDIRINC)/polarssl/library/arc4.o \
				$(EXDIRINC)/polarssl/library/asn1parse.o \
				$(EXDIRINC)/polarssl/library/base64.o \
				$(EXDIRINC)/polarssl/library/bignum.o \
				$(EXDIRINC)/polarssl/library/blowfish.o \
				$(EXDIRINC)/polarssl/library/camellia.o \
				$(EXDIRINC)/polarssl/library/certs.o \
				$(EXDIRINC)/polarssl/library/cipher.o \
				$(EXDIRINC)/polarssl/library/cipher_wrap.o \
				$(EXDIRINC)/polarssl/library/ctr_drbg.o \
				$(EXDIRINC)/polarssl/library/debug.o \
				$(EXDIRINC)/polarssl/library/des.o \
				$(EXDIRINC)/polarssl/library/dhm.o \
				$(EXDIRINC)/polarssl/library/gcm.o \
				$(EXDIRINC)/polarssl/library/md.o \
				$(EXDIRINC)/polarssl/library/md_wrap.o \
				$(EXDIRINC)/polarssl/library/md2.o \
				$(EXDIRINC)/polarssl/library/md4.o \
				$(EXDIRINC)/polarssl/library/md5.o \
				$(EXDIRINC)/polarssl/library/padlock.o \
				$(EXDIRINC)/polarssl/library/pbkdf2.o \
				$(EXDIRINC)/polarssl/library/pem.o \
				$(EXDIRINC)/polarssl/library/pkcs11.o \
				$(EXDIRINC)/polarssl/library/rsa.o \
				$(EXDIRINC)/polarssl/library/sha1.o \
				$(EXDIRINC)/polarssl/library/sha2.o \
				$(EXDIRINC)/polarssl/library/sha4.o \
				$(EXDIRINC)/polarssl/library/ssl_cache.o \
				$(EXDIRINC)/polarssl/library/ssl_cli.o \
				$(EXDIRINC)/polarssl/library/ssl_tls.o \
				$(EXDIRINC)/polarssl/library/x509parse.o \
				$(EXDIRINC)/polarssl/library/xtea.o 
include $(PROJ_ROOT)/makesection/makerule/example/makefile.ex
