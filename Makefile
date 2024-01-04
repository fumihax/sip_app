#
#
#
SUBJ = udp_s udp_c sip_app

#
all: $(SUBJ) 

#
CC   = gcc
AR   = ar

XLIBD = /usr/X11R6/lib
XINCD = /usr/X11R6/include

LCLINC = /usr/local/include
SSLINC = /usr/local/ssl/include 

LIB_DIR = ../JunkBox_Lib
LIB_BSC_DIR = $(LIB_DIR)/Lib
LIB_GRA_DIR = $(LIB_DIR)/gLib
LIB_EXT_DIR = $(LIB_SIR)/xLib

LIB_BSC = $(LIB_BSC_DIR)/libbasic.a
LIB_GRA = $(LIB_GRA_DIR)/libgraph.a
LIB_EXT = $(LIB_EXT_DIR)/libextend.a


#CFLAGS  = -DHAVE_CONFIG_H -DEBUG -DENABLE_TRACE -I$(LCLINC) -I$(LIB_BSC_DIR) -I$(LIB_DIR) -I$(LIB_EXT_DIR) -I$(XINCD) -I$(SSLINC)  
CFLAGS  = -DHAVE_CONFIG_H -DENABLE_TRACE -I$(LCLINC) -I$(LIB_BSC_DIR) -I$(LIB_DIR) -I$(LIB_EXT_DIR) -I$(XINCD) -I$(SSLINC)  

#SIPLIB = -lgc -losip2 -leXosip2
SIPLIB = -losipparser2 -losip2 -leXosip2 -lssl -lpthread
XLIB = -L$(XLIBD) -lX11 
SLIB = -L$(LIB_BSC_DIR) -lbasic -lm
GLIB = -L$(LIB_GRA_DIR) -lgraph $(XLIB)
ELIB = -L$(LIB_EXT_DIR) -lextend
#
#
#

.c.o:
	$(CC) $< $(CFLAGS) -c -O2 


clean:
	-rm -f *.o *~ $(SUBJ)

tgz:
	make clean
	(cd .. && tar cfp - sip_app|gzip > sip_app.tgz)
#
#
#
#

sip_app: sip_app.o sip_session.o sdp_tools.o sip_tools.o $(LIB_BSC) 
	$(CC) $(@).o sip_session.o sdp_tools.o sip_tools.o $(SLIB) $(SIPLIB) -O2 -o $@

udp_s: udp_s.o  $(LIB_BSC) 
	$(CC) $(@).o $(SLIB) -O2 -o $@

udp_c: udp_c.o  $(LIB_BSC) 
	$(CC) $(@).o $(SLIB) -O2 -o $@

xxx: xxx.o  $(LIB_BSC) 
	$(CC) $(@).o $(SLIB) -O2 -o $@



$(LIB_BSC):
	(cd $(LIB_BSC_DIR) && make)


$(LIB_GRA):
	(cd $(LIB_GRA_DIR) && make)

	
$(LIB_EXT):
	(cd $(LIB_EXT_DIR) && make)


