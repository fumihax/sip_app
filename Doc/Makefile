
.SUFFIXES: .doc



all: sdp_tools.doc sip_tools.doc 



C2DOCD = ../../JunkBox_Lib/Tools
C2DOC = $(C2DOCD)/c2doc
TO_EUC = nkf -e
CC = gcc



sip_tools.doc:  ../sip_tools.c  ../sip_tools.h  $(C2DOC)
	$(C2DOC)  ../$(*).h | $(TO_EUC) >  $@
	$(C2DOC)  ../$(*).c | $(TO_EUC) >> $@

sdp_tools.doc:  ../sdp_tools.c  ../sdp_tools.h  $(C2DOC)
	$(C2DOC)  ../$(*).h | $(TO_EUC) >  $@
	$(C2DOC)  ../$(*).c | $(TO_EUC) >> $@


clean: 
	rm -f *.doc  *~


$(C2DOC): 
	(cd $(C2DOCD); make c2doc)

