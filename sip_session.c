
#include "sip_session.h"




///////////////////////////////////////////////////////////////////////
//
//	REGISTER
//

int  send_regist(sip_param* param)
{
	int err;

	err = sip_regist_send_and_answer(param);
	if (err==EXOSIP_REGISTRATION_SUCCESS) {
		DEBUG_MODE print_message("send_regist: Registration Success.\n");
	}	
	else {
		DEBUG_MODE print_message("send_regist: Registration Failure.\n");
		return -1;
	}
		
	sip_regist_thread(param);
	
	return err;
}





///////////////////////////////////////////////////////////////////////
//
//	INVITE
//

int  send_invite(sip_param* param)
{
	int err;
	sdp_message_t* sdp;

	err = sip_invite_send_and_answer(param);
	DEBUG_MESG("sip_invite_send_and_answer = %d\n", err);

    if (err!=EXOSIP_CALL_ANSWERED) {
		sip_session_terminate(param);
		return -1;
    }

    sdp = sdp_get_body(param);
	err = get_sdp_paramter(param, sdp);
	if (err!=0) {
		sip_session_terminate(param);
		return -2;
	}
    sdp_message_free(sdp);
    DEBUG_MESG("sdp_get_body: %s:%d  %s:%s\n\n", param->remoteip, param->remoteport, param->keytype, param->keydata);

    err = sip_send_ack(param);
    if (err!=0) {
		sip_session_terminate(param);
		return -3;
	}

	return err;
}




int  recive_invite(sip_param* param)
{
	int pid = -1;
	int err;
	char* tmp = NULL;

	DEBUG_MESG("\nrecive_invite: new invite event wait start\n");

	//子プロセス用の起動時引数セット
	char** argv;
	argv = (char**)malloc(sizeof(char*)*5);
	argv[0] = dup_str(param->prog);
	argv[1] = dup_str(ltostr(param->localport));
	argv[2] = dup_str(param->vpn_localip);
	argv[3] = dup_str(param->vpn_remoteip);
	argv[4] = NULL;
	param->argv = argv;

	err = sip_invite_wait(param);
	if (err==0) pid = sip_invite_accept(param);

	return pid;
}





///////////////////////////////////////////////////////////////////////
//
//	SESSION	
//

int  session_start(sip_param* param)
{
	int err;

	/* ベタベタ (^^;*/
	if (param->server==OFF) {
		char** argv;
		argv = (char**)malloc(sizeof(char*)*6);
		argv[0] = dup_str(param->prog);
		argv[1] = dup_str(param->remoteip);
		argv[2] = dup_str(ltostr(param->remoteport));
		argv[3] = dup_str(param->vpn_localip);
		argv[4] = dup_str(param->vpn_remoteip);
		argv[5] = NULL;
		param->argv = argv;
	} 

	err = sip_session_start(param);
	DEBUG_MESG("session_start: セッションがスタートしました\n\n");

	return err;
}





int  session_term_wait(sip_param* param)
{
	int err;

	err = sip_session_term_wait(param);
	return err;
}





int  session_terminate(sip_param* param)
{
	int err;

	err = sip_session_terminate(param);
	DEBUG_MESG("\nsession_terminate: セッションが終了しました\n");

	return err;
}









/**   
    注意：
        現在の所，取り出すデータは上位アプリに非常に依存したコーディングになっている．
        取り出すデータ：keytype, keydata, attribute(IP4, PORT)
*/
int  get_sdp_paramter(sip_param* param, sdp_message_t* sdp)
{    
    char* tmp;

	if (sdp==NULL) return -1;

//  sdp_print_body(sdp);
    sdp_show_body(sdp);

    if (param->remoteip!=NULL) free(param->remoteip);
    if (param->keytype!=NULL)  free(param->keytype);
    if (param->keydata!=NULL)  free(param->keydata);
    param->remoteport = -1;
   
    param->remoteip = sdp_get_media_attr(sdp, "IP4", 0);
    tmp = sdp_get_media_attr(sdp, "PORT", 0);
    if (tmp!=NULL) {
        param->remoteport = atoi(tmp);
        free(tmp);
    }
    param->vpn_localip  = sdp_get_media_attr(sdp, "VPN_LOCAL_ADDR", 0);
    param->vpn_remoteip = sdp_get_media_attr(sdp, "VPN_REMOTE_ADDR", 0);

    param->keytype = sdp_get_media_keytype(sdp, 0);
    param->keydata = sdp_get_media_keydata(sdp, 0);

   
    return 0;
}









///////////////////////////////////////////////////////////////////////
//
//	JUNK Code	
//
#if 0

int  recive_invite(sip_param* param)
{
	int pid;
	eXosip_event_t* ev;
	char* tmp = NULL;

	DEBUG_MESG("\nrecive_invite: new invite event wait start\n");

	ev = sip_invite_wait(param);
/*	osip_from_to_str(ev->request->from, &tmp);
    if (tmp != NULL) {
		print_message("From -> %s\n %s\n", tmp, ev->textinfo);
	}
*/
	eXosip_event_free(ev);

	pid = sip_invite_accept(param);

	return 0;
}







#endif
