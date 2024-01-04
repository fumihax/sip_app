
#include  "sip_tools.h"
#include  "sdp_tools.h"


int  send_regist(sip_param* param);

int  send_invite(sip_param* param);
int  recive_invite(sip_param* param);

int  session_start(sip_param* param);
int  session_term_wait(sip_param* param);
int  session_terminate(sip_param* param);

int  get_sdp_paramter(sip_param* param, sdp_message_t* sdp);


