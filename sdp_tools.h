/*
*/

#ifndef  _SDP_TOOLS_H
#define  _SDP_TOOLS_H

int   sdp_set_body (sip_param* param, osip_message_t* mesg);
int   sdp_set_body2(sip_param* param, osip_message_t* mesg);

sdp_message_t*  sdp_get_body(sip_param* param);
char* sdp_get_media_keytype(sdp_message_t* sdp, int pos); 
char* sdp_get_media_keydata(sdp_message_t* sdp, int pos);
char* sdp_get_media_attr(sdp_message_t* sdp, char* key, int pos); 

void  sdp_print_body(sdp_message_t* sdp);
void  sdp_show_body(sdp_message_t* sdp);

/**/

#endif

