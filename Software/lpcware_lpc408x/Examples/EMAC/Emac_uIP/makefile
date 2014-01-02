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
EXECNAME    		=emac_uIP_test

APP_NAME			= WEB_SERVER
//APP_NAME			= WEB_CLIENT
//APP_NAME			= SMTP_APP
//APP_NAME			= TELNET_APP

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
				-I$(EXDIRINC)/lpc_port \
				-I$(EXDIRINC)/uip 
	
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
				$(EXDIRINC)/lpc_port/Emac_uIP.o 
				
ifeq ($(APP_NAME), WEB_SERVER)	
CTMPFLAGS += -DWEB_SERVER
TMPPATH 		+= -I$(EXDIRINC)/apps/webserver

# uIP 'apps' files
ADDOBJS		+=  $(EXDIRINC)/apps/webserver/httpd.o \
				$(EXDIRINC)/apps/webserver/httpd-cgi.o \
				$(EXDIRINC)/apps/webserver/httpd-fs.o \
				$(EXDIRINC)/apps/webserver/http-strings.o 
endif	

ifeq ($(APP_NAME), WEB_CLIENT)	
CTMPFLAGS += -DWEB_CLIENT -DWEB_BROWSER -DDNS_USED -DDHCP_ENABLE
TMPPATH 		+= -I$(EXDIRINC)/apps/webclient \
				-I$(EXDIRINC)/apps/webbrowser \
				-I$(EXDIRINC)/apps/dhcpc \
				-I$(EXDIRINC)/apps/resolv		

# uIP 'apps' files
ADDOBJS		+=  $(EXDIRINC)/apps/webclient/webclient.o \
				$(EXDIRINC)/apps/webclient/webclient-strings.o \
				$(EXDIRINC)/apps/webbrowser/htmlparser.o \
				$(EXDIRINC)/apps/webbrowser/html-strings.o \
				$(EXDIRINC)/apps/dhcpc/dhcpc.o \
				$(EXDIRINC)/apps/resolv/resolv.o \
				$(EXDIRINC)/uip/uiplib.o
endif
	
ifeq ($(APP_NAME), SMTP_APP)
CTMPFLAGS += -DSMTP_APP -DDNS_USED -DDHCP_ENABLE	
TMPPATH 		+= -I$(EXDIRINC)/apps/smtp \
				-I$(EXDIRINC)/apps/dhcpc \
				-I$(EXDIRINC)/apps/resolv	
# uIP 'apps' files
ADDOBJS		+=  $(EXDIRINC)/apps/smtp/smtp.o \
				$(EXDIRINC)/apps/smtp/smtp-strings.o \
				$(EXDIRINC)/lib/base64.o \
				$(EXDIRINC)/apps/dhcpc/dhcpc.o \
				$(EXDIRINC)/apps/resolv/resolv.o 	
endif
	
ifeq ($(APP_NAME), TELNET_APP)
CTMPFLAGS += -DTELNET_APP	
TMPPATH 		+= -I$(EXDIRINC)/apps/telnetd
# uIP 'apps' files
ADDOBJS		+=  $(EXDIRINC)/apps/telnetd/telnetd.o \
				$(EXDIRINC)/apps/telnetd/shell.o \
				$(EXDIRINC)/lib/memb.o 
endif				
include $(PROJ_ROOT)/makesection/makerule/example/makefile.ex
