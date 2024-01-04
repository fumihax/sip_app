

#include  "sip_session.h"


#define  MAX_ARGS   100
#define  SIP_PORTNO "5060"
#define  VPN_PORTNO "8000"

int   main(int, char**);

void  sip_main_term(int signal);
void  sip_child_term(int signal);

