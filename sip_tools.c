/**
  SIP用ツール　                          by Fumi.Iseki
                                         (C) 2006 9/1  
*/



#include "sip_tools.h"
#include "sdp_tools.h"




/**
void  sip_param_init(sip_param* param)

    機能：SIP用パラメータを初期化する．

	引数：param  SIP用パラメータ

	戻り値：なし

*/
void  sip_param_init(sip_param* param)
{
	param->tid   = -1;
	param->rid   = -1;
	param->iid   = -1;
	param->cid   = 0;
	param->did   = 0;

	param->type   = -1;
	param->auth   = ON;
	param->server = ON;
	param->cpid   = -1;
	param->ppid   = -1;

	param->prog = NULL;
	param->argv = NULL;
	param->env  = NULL;

	param->inInvite  = OFF;
	param->inRegist  = OFF;
	param->inSession = OFF;
	param->inWait    = OFF;
	param->inExit    = OFF;
	param->inChild   = OFF;

	param->from      = NULL;
	param->to        = NULL;
	param->proxy     = NULL;
	param->contact   = NULL;
	param->expires   = 3600;

	param->localip   = NULL;
	param->remoteip  = NULL;
	param->username  = NULL;
	param->media     = NULL;
	param->protocol  = NULL;
	param->fmtlist   = NULL;
	param->keytype   = NULL;
	param->keydata   = NULL;
	param->localport = 0;
	param->remoteport= 0;

	param->mssg      = NULL;
	param->rgst      = NULL;
	param->invt      = NULL;

	param->vpn_localip   = NULL;
	param->vpn_remoteip  = NULL;
}





/**
void  sip_param_free(sip_param* param)

    機能：SIP用パラメータ変数をクリアする．
　　　　　ただし，eXosipのメッセージ変数はクリアされないので
          eXosip_quit と併用する．

	引数：param  SIP用パラメータ

	戻り値：なし

*/
void  sip_param_free(sip_param* param)
{
	if (param->prog!=NULL) 	   free(param->prog);
//	if (param->argv!=NULL)	   free(param->argv);
//	if (param->env!=NULL)	   free(param->env);

	if (param->from!=NULL)     free(param->from);
	if (param->to!=NULL)       free(param->to);
	if (param->proxy!=NULL)    free(param->proxy);
	if (param->contact!=NULL)  free(param->contact);

	if (param->localip!=NULL)  free(param->localip);
	if (param->remoteip!=NULL) free(param->remoteip);
	if (param->username!=NULL) free(param->username);
	if (param->media!=NULL)    free(param->media);
	if (param->protocol!=NULL) free(param->protocol);
	if (param->fmtlist!=NULL)  free(param->fmtlist);
	if (param->keytype!=NULL)  free(param->keytype);
	if (param->keydata!=NULL)  free(param->keydata);

	if (param->vpn_localip!=NULL)  free(param->vpn_localip);
	if (param->vpn_remoteip!=NULL) free(param->vpn_remoteip);
/*
	if (param->invt!=NULL) osip_free(param->mssg);
	if (param->rgst!=NULL) osip_free(param->rgst);
	if (param->invt!=NULL) osip_free(param->invt);
*/
	sip_param_init(param);
}





/**
int  sip_terminate(sip_param* param)

    機能：parm->ppid で指定したプロセスのSIPセッションを終了する．

	引数：param  SIP用パラメータ
		  param->ppid   セッションを停止させるプロセスのPID．通常は一番上（親）のPID．

　　戻り値：0: していされたプロセスのセッションを停止させた．呼び出し側でそのまま終了させる．
			1: 指定されたプロセスではないので何もしなかった．呼び出し側でそのままリターンさせる．

*/
int  sip_terminate(sip_param* param)
{
	int  pid;

	pid = getpid();

    if (param->inExit!=ON && param->ppid==pid) {
        param->inExit = ON;

		//DEBUG_MESG("sip_terminate:\n");

		// BYE を送信
		if (param->inInvite==ON || param->inSession==ON) {
			if (param->inInvite ==ON) param->inInvite  = OFF;
			if (param->inSession==ON) param->inSession = OFF;
			sip_session_terminate(param);
		}

		// REGISTを抹消
		param->expires = 0;
		sip_regist_resend(param);

		eXosip_lock();
        eXosip_register_remove(param->rid);
		eXosip_unlock();
		eXosip_quit();

        sip_param_free(param);
		return 0;
	}
    return 1;
}









/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// REGISTER
//

/**
int sip_regist_send_and_answer(sip_param* param)

    機能：パラメータの内容に従って，サーバへレジストして返答を待つ．

	引数：param  SIP用パラメータ

　　戻り値： 0 以上:  レジストに対する eXosipメッセージ
					  	see /usr/local/include/eXosip2/eXosip.h
		     0 未満:  エラー
*/
int sip_regist_send_and_answer(sip_param* param)
{
	int err;

	while (param->inInvite==ON) {
		DEBUG_MODE print_message("Sleeping in sip_regist_send_and_answer.\n");
		usleep(SIP_USLEEP_TIME);
	}
	param->inRegist = ON;

	err = sip_regist_send(param);
	if (err<0) {
		param->inRegist = OFF;
		return err;
	}

	err = sip_regist_answer(param);
	param->inRegist = OFF;
	
	return err;
}




/**
int  sip_regist_send(sip_param* param)

    機能：パラメータの内容に従って，サーバへレジストメッセージを送信する．

	引数：param  SIP用パラメータ

　　戻り値： 0以上： 正常にレジストメッセージをサーバに送信した． 
		     0未満： エラー
*/
int  sip_regist_send(sip_param* param)
{
	int err;

	eXosip_lock();
	err = eXosip_register_build_initial_register(param->from, param->proxy, param->contact, param->expires, &(param->rgst));
	param->rid = err;
	if (err<0) {
		eXosip_unlock();
		return err;
	}

	err = eXosip_register_send_register(param->rid, param->rgst);
	eXosip_unlock();
	return err;
}




/**
int  sip_regist_resend(sip_param* param)

    機能：パラメータの内容に従って，サーバへレジストメッセージを再送信する．

	引数：param  SIP用パラメータ

　　戻り値： 0以上： 正常に再レジストメッセージをサーバに送信した． 
		     0未満： エラー
*/
int  sip_regist_resend(sip_param* param)
{
	int err;

	eXosip_lock();
	err = eXosip_register_build_register(param->rid, param->expires, &(param->rgst));
	if (err<0) {
		eXosip_unlock();
		return err;
	}

	err = eXosip_register_send_register(param->rid, param->rgst);  
	eXosip_unlock();
	return err;
}





/**
int  sip_regist_answer(sip_param* param)

    機能：レジストに対するサーバからの返答を待つ．

	引数：param  SIP用パラメータ

　　戻り値： 0 以上:  レジストに対する eXosipメッセージ
					  	see /usr/local/include/eXosip2/eXosip.h
		     0 未満:  エラー
*/
int  sip_regist_answer(sip_param* param)
{
	int  err, etype;
	int  brk_flg = OFF;

    eXosip_event_t *event;

	Loop {
      	event = eXosip_event_wait(0, 0);
      	if (event==NULL) {
			usleep(SIP_USLEEP_TIME);
			continue;
		}
		etype = event->type;
        DEBUG_MESG("sip_regist_answer: Event (type, did, cid) = (%d, %d, %d)\n", event->type, event->did, event->cid);

		if (etype == EXOSIP_REGISTRATION_SUCCESS) {
          	//DEBUG_MESG("sip_regist_answer: Registration Success.\n");
			brk_flg = ON;
		}
		else if (etype == EXOSIP_REGISTRATION_FAILURE) {
            if (param->auth==ON) {
				err = sip_regist_resend(param);	// 認証を行う場合はもう一度送る
				param->auth = OFF;
				if (err<0) brk_flg = ON;
			}
			else {
				DEBUG_MESG("sip_regist_answer: Registration Failure.\n");
				brk_flg = ON;
			}
        }
		else {
        	DEBUG_MODE print_message("sip_regist_answer: Unexpected Event. event(type, did, cid) = (%d, %d, %d)\n", 
								event->type, event->did, event->cid);
		}

		param->type = etype;
		param->cid  = event->cid;
		param->did  = event->did;
      	eXosip_event_free(event);
		if (brk_flg==ON) break;
	}

	param->auth = ON;	// デフォルトはONという事で....
	return etype;
}





/**
int  sip_regist_thread(sip_param* param)

    機能：再レジスト用のスレッド生成を行う．

	スレッドの機能：
		  再レジストに失敗した場合，試行時間間隔を短縮(1/2)して再試行する．
		  最小の再レジスト間隔は 10s

	引数：param  SIP用パラメータ

　　戻り値： スレッド生成 pthread_create() の結果．pthread_create のマニュアルを見よ．

*/
int  sip_regist_thread(sip_param* param)
{
	int err;
	pthread_t reg_thread;

	err = pthread_create(&reg_thread, NULL, sip_regist_thread_loop, (void*)param);
	if (err!=0) {
		DEBUG_MODE print_message("sip_regist_thread: Thread Create Failure.\n");
	}

	return err;
}






/**
static void*  sip_regist_thread_loop(void* arg)

    機能：sip_regist_thread() から呼び出されるスレッド本体．
　　　　　通常は直接呼び出されることは無い．

		  再レジストに失敗した場合，試行時間間隔を短縮(1/2)して再試行する．
		  最小の再レジスト間隔は 10s

	引数：param  SIP用パラメータ void* へキャスト．

*/
static void*  sip_regist_thread_loop(void* arg)
{
	int err;
	int exp;
	sip_param *param = arg;

	exp = param->expires/2;


	Loop {
		sleep(exp);

		if (param->inExit==ON) return;		// 終了処理中なので中断してリターンする．
		param->inRegist = ON;
		while (param->inInvite==ON || param->inWait==ON) {	// 他の仕事が終わるまで待つ．
			DEBUG_MODE print_message("Sleeping in sip_regist_thred_loop\n");
			usleep(SIP_USLEEP_TIME);
		}

		err = sip_regist_resend(param);

		if (err==0) {
			err = sip_regist_answer(param);
			if (err==EXOSIP_REGISTRATION_SUCCESS) {
				DEBUG_MESG("sip_regist_thread_loop: Re-Registed.\n");
				exp = param->expires/2;
			}
			else {
				DEBUG_MESG("sip_regist_thread_loop: Re-Registration Failure.  %d\n", err);
				exp = exp/2;
				if (exp<10) exp = 10;
			}
		}
		else {
			DEBUG_MESG("sip_regist_thread_loop: Re-Registration Send error. %d\n", err);
			exp = exp/2;
			if (exp<10) exp = 10;
		}
		param->inRegist = OFF;
	}

	return;
}









/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// INVITE
//


/**
int sip_invite_send_and_answer(sip_param* param)

    機能：パラメータの内容に従って，インバイトを送信して返答を待つ．

	引数：param  SIP用パラメータ

　　戻り値： 0 以上:  インバイトに対する eXosipメッセージ
					  	see /usr/local/include/eXosip2/eXosip.h
		     0 未満:  エラー
*/
int sip_invite_send_and_answer(sip_param* param)
{
	int err;

	while (param->inRegist==ON) {
		DEBUG_MODE print_message("Sleeping in sip_invite_and_anser.\n");
		param->inInvite = OFF;	// Dead Lock 防止  Register優先．
		usleep(SIP_USLEEP_TIME);
	}
	param->inInvite = ON;

	err = sip_invite_send(param);
	if (err<0) {
		param->inInvite = OFF;
		return err;
	}
	param->iid = err;

	err = sip_invite_answer(param);
	param->inInvite = OFF;
	
	return err;
}





/**
int  sip_invite_send(sip_param* param)

    機能：パラメータの内容に従って，サーバへインバイトメッセージを送信する．

	引数：param  SIP用パラメータ

　　戻り値： 0以上： INVITE ID, 正常にインバイトメッセージをサーバに送信した． 
		     0未満： エラー
*/
int  sip_invite_send(sip_param* param)
{
	int err;
	
	eXosip_lock();
	err = eXosip_call_build_initial_invite(&(param->invt), param->to, param->from, NULL, NULL);
	if (err<0) {
		eXosip_unlock();
		return err;
	}

    err = eXosip_call_send_initial_invite(param->invt);
    eXosip_unlock ();

	return err;
}





/**
int  sip_invite_answer(sip_param* param)

    機能：インバイトに対するサーバからの返答を待つ．

	引数：param  SIP用パラメータ

　　戻り値： 0 以上:  インバイトに対する eXosipメッセージ
					  	see /usr/local/include/eXosip2/eXosip.h
		     0 未満:  エラー
*/
int  sip_invite_answer(sip_param* param)
{
	int  err, etype;
	int  brk_flg = OFF;
	int  suc_flg = OFF;

    eXosip_event_t *event;

	Loop {
      	event = eXosip_event_wait(0, 100);
      	if (event==NULL) continue;
		etype = event->type;
        DEBUG_MODE print_message("sip_invite_answer: Event (type, did, cid) = (%d, %d, %d)\n", event->type, event->did, event->cid);


		// 無視するイベント
	    if (etype==EXOSIP_CALL_INVITE) {				// a new call
        	DEBUG_MESG("sip_invite_answer: Call Invite.  %d\n", etype);
		}
    	else if (etype==EXOSIP_CALL_REINVITE) { 		// a new INVITE within call
        	DEBUG_MESG("sip_invite_answer: Call ReInvite.  %d\n", etype);
		}
 	   	else if (etype==EXOSIP_CALL_ACK) { 				// ACK received for 200ok to INVITE
        	DEBUG_MESG("sip_invite_answer: Call ACK.  %d\n", etype);
		}
		else if (etype==EXOSIP_CALL_PROCEEDING) {		// processing by a remote app  
        	DEBUG_MESG("sip_invite_answer: Remote App.  %d\n", etype);
		}
	    else if (etype==EXOSIP_CALL_RINGING) { 			// 呼び出し中．
        	DEBUG_MESG("sip_invite_answer: Call Ringing.  %d\n", etype);
		}


		// ループを終了するイベント（接続）
	    else if (etype==EXOSIP_CALL_ANSWERED) { 		// 通話開始
        	DEBUG_MESG("sip_invite_answer: Call Answer.  %d\n", etype);
			suc_flg = ON;
			brk_flg = ON;
		}


		// ループを終了するイベント（エラー）
	    else if (etype==EXOSIP_CALL_REQUESTFAILURE) { 	// 電話に出ない
        	DEBUG_MESG("sip_invite_answer: Request Failure.  %d\n", etype);
			brk_flg = ON;
		}
	    else if (etype==EXOSIP_CALL_NOANSWER) {			// no answer within the timeout
        	DEBUG_MESG("sip_invite_answer: Time out.  %d\n", etype);
			brk_flg = ON;
		}
	    else if (etype==EXOSIP_CALL_SERVERFAILURE) { 	// a server failure
        	DEBUG_MESG("sip_invite_answer: Server Failure.  %d\n", etype);
			brk_flg = ON;
		}
	    else if (etype==EXOSIP_CALL_GLOBALFAILURE) { 	// a global failure
        	DEBUG_MESG("sip_invite_answer: Global Failure.  %d\n", etype);
			brk_flg = ON;
		}
    	else if (etype==EXOSIP_CALL_CANCELLED) { 		// that call has been cancelled
        	DEBUG_MESG("sip_invite_answer: Call Cancelled.  %d\n", etype);
			brk_flg = ON;
		}
    	else if (etype==EXOSIP_CALL_CLOSED) { 			// a BYE was received for this call
        	DEBUG_MESG("sip_invite_answer: Call Closed.  %d\n", etype);
			brk_flg = ON;
		}
		else if (etype==EXOSIP_CALL_RELEASED) {			// call context is cleared. 
        	DEBUG_MESG("sip_invite_answer: Call Released.  %d\n", etype);
			brk_flg = ON;
		}


		// その他のイベントは無視する
		else {
        	DEBUG_MESG("sip_invite_answer: Continue Event. event(type, did, cid) = (%d, %d, %d)\n", 
							event->type, event->did, event->cid);
    	}


		param->type = etype;
		param->cid  = event->cid;
		param->did  = event->did;

      	eXosip_event_free(event);
		if (brk_flg==ON) break;
	}


	if (suc_flg==OFF) {
       	DEBUG_MESG("sip_invite_answer: END Event = %d.\n", etype);
	}

	return etype;
}








/**
eXosip_event_t*  sip_invite_wait(sip_param* param)

    機能：INVITEイベントを待つ

	引数：param  SIP用パラメータ

　　戻り値： 0： INVITEメッセージを受信．
		     0未満： エラー

//	戻り値： イベント変数(eXosip_event_t) へのポインタ．失敗した場合は NULL．  
//			 イベント変数はどこかでフリーしないといけない．eXosip_event_free()

*/
int  sip_invite_wait(sip_param* param)
{
	int  err, etype;
    eXosip_event_t *event;


	Loop {
		param->inWait = OFF;
		while (param->inRegist==ON) {
			DEBUG_MODE print_message("Sleeping in sip_invite_wait.\n");
			usleep(SIP_USLEEP_TIME);
		}
		param->inWait = ON;

      	event = eXosip_event_wait(0, 100);

      	if (event==NULL) {
			continue;
		}
		etype = event->type;
        DEBUG_MODE print_message("sip_invite_wait_event: Event (type, did, cid) = (%d, %d, %d)\n", event->type, event->did, event->cid);

		param->type = etype;
		param->tid  = event->tid;
		param->cid  = event->cid;
		param->did  = event->did;
      	eXosip_event_free(event);

		if (etype==EXOSIP_CALL_INVITE || etype==EXOSIP_CALL_REINVITE) {
			param->inInvite = ON;
			break;
		}
      	//eXosip_event_free(event);
	}

	param->inWait = OFF;
	return 0;
	//return event;
}




/**
int  sip_invite_accept(sip_param* param)

	機能：INVITE要求を処理する．

	引数：param  SIP用パラメータ

　　戻り値： 0以上 : 子プロセスのプロセスID. 正常終了
			 0未満 : エラー 

*/
int  sip_invite_accept(sip_param* param)
{
	int i, err, pid;


	// 100 Trying 送信
	eXosip_lock();
	eXosip_call_send_answer(param->tid, 100, NULL);
	eXosip_unlock();


	// 180 Ringing 送信
	eXosip_lock();
	eXosip_call_send_answer(param->tid, 180, NULL);
	eXosip_unlock();


	// プロセスの起動
	pid = sip_process_fork(param);
	if (pid<0) {
		DEBUG_MESG("sip_invite_accept: 子プロセスの起動に失敗.\n");
		sip_session_terminate(param);
		return -1;
	}
	param->cpid = pid;


	// 200 OK 送信
	eXosip_lock();
	err = eXosip_call_build_answer(param->tid, 200, &param->mssg);
	if (err==0) {
		sdp_set_body2(param, param->mssg);
		err = eXosip_call_send_answer(param->tid, 200, param->mssg);
		//osip_message_free(param->mssg);
		//param->mssg = NULL;
	}
	eXosip_unlock();
	param->inInvite = OFF;


	if (err!=0) {
		DEBUG_MESG("sip_invite_accept: 200 OK の送信に失敗.\n");
		sip_session_terminate(param);
		return -1;
	}
	else {
		param->inSession = ON;
	}

	return pid;
}






/////////////////////////////////////////////////////////////////////////////////////////
//
// SESSION
//


/**
int  sip_session_term_wait(sip_param* param)

	機能：切断要求を待つ．

	引数：param  SIP用パラメータ

　　戻り値： 0 以上:  インバイトに対する eXosipメッセージ
					  	see /usr/local/include/eXosip2/eXosip.h
		     0 未満:  エラー

*/
int  sip_session_term_wait(sip_param* param)
{
	int  err, etype;
    eXosip_event_t *event;

    //DEBUG_MESG("sip_session_term_wait: Wait termination\n");

	Loop {
		param->inSession = OFF;
		while (param->inRegist==ON) {
			DEBUG_MODE print_message("Sleeping in sip_session_term_wait.\n");
			usleep(SIP_USLEEP_TIME);
		}
		param->inSession = ON;

		if (param->inChild==OFF) break;	// 子プロセスは終了している．

      	event = eXosip_event_wait(0, 100);

      	if (event==NULL) {
			continue;
		}
		etype = event->type;
        DEBUG_MODE print_message("sip_session_term_wait: Event (type, did, cid) = (%d, %d, %d)\n", event->type, event->did, event->cid);

		param->type = etype;
		param->tid  = event->tid;
		param->cid  = event->cid;
		param->did  = event->did;
      	eXosip_event_free(event);

		if (etype==EXOSIP_CALL_CLOSED || etype==EXOSIP_CALL_CANCELLED || etype==EXOSIP_CALL_RELEASED) {
			break;
		}

	}

	param->inSession = OFF;
	return etype;
}
	 




/**
int  sip_session_terminate(sip_param* param)

	機能：接続を切断する．

	引数：param  SIP用パラメータ

　　戻り値： 0 : 正常終了
		     0未満 : エラー

*/
int  sip_session_terminate(sip_param* param)
{
	if (param->cpid>0) {
		param->inChild = OFF;
		kill(param->cpid, SIGTERM);
	}

	// BYE を送信
	eXosip_lock();
	eXosip_call_terminate(param->cid, param->did);
	eXosip_unlock();
	sleep(2);

	return 0;
}





/**
int  sip_session_start(sip_param* param)

    機能：セッションを開始する．
    引数：param  SIP用パラメータ

　　戻り値： 呼び出し（クライアント）の場合は 子プロセスの PID．サーバの場合は 0が返る．i
             0未満 : エラー

*/
int  sip_session_start(sip_param* param)
{
    int pid = 0;
    if (param->server==OFF) {
        pid = sip_process_fork(param);

        if (pid<0) {
            DEBUG_MESG("sip_session_start: 子プロセスの起動に失敗.\n");
            sip_session_terminate(param);
            return -1;
        }
        param->cpid = pid;
    }
	return pid;
}











/////////////////////////////////////////////////////////////////////////////////////////
//
//

/**
int  sip_send_ack(sip_param* param)


	戻り値： 0: 正常終了
	         0未満: エラー
*/
int  sip_send_ack(sip_param* param)
{
	int err;

    eXosip_lock();
    err = eXosip_call_build_ack(param->did, &param->mssg);
    if (err!=0) {
		sip_session_terminate(param);
	}
	else {
		err = eXosip_call_send_ack(param->did, param->mssg);
	}
    eXosip_unlock();

	return err;
}





/**
int  sip_event_get(sip_param* param)

    機能：

	引数：param  SIP用パラメータ

　　戻り値： 0 以上:  インバイトに対する eXosipメッセージ
						see. /usr/local/include/eXosip2/eXosip.h
		     0 未満:  エラー
*/
int  sip_event_get(sip_param* param)
{
	int  err, etype;
    eXosip_event_t *event;


	Loop {
		param->inWait = OFF;
		while (param->inRegist==ON) {
			DEBUG_MODE print_message("Sleeping in sip_get_event.\n");
			usleep(SIP_USLEEP_TIME);
		}
		param->inWait = ON;

      	event = eXosip_event_wait(0, 100);

      	if (event==NULL) {
			continue;
		}
		etype = event->type;
        DEBUG_MODE print_message("sip_event_get: Event (type, did, cid) = (%d, %d, %d)\n", event->type, event->did, event->cid);

		param->type = etype;
		param->tid  = event->tid;
		param->cid  = event->cid;
		param->did  = event->did;
      	eXosip_event_free(event);


		if (etype==EXOSIP_CALL_INVITE) {
			param->inInvite = ON;
			err = sip_invite_accept(param);
			param->inInvite = OFF;
		}

	}

	return etype;
}







int  sip_process_fork(sip_param* param)
{
	int pid;

	char* const cmdline[] ={
		"./openvpn_server.sh","8139","192.168.250.1","192.268.250.2",NULL
	};

	if (file_exist(param->prog)==FALSE) return -1;

	param->inChild = ON;

	pid = fork();
	if (pid==0) {
		//execve((const char*)(param->prog), param->argv, param->env);
		execve((const char*)(param->prog), cmdline, param->env);
        //fprintf(stderr, "RemoteIP:%s RemotePort:%s \n", param->remoteip, param->remoteport);
		print_message("sip_process_fork: 致命的：子プロセスを起動できない．%s %s %s\n", param->prog, param->argv, param->env);
		print_message("sip_process_fork: param->argv：%s %s %s %s %s\n", param->argv[0], param->argv[1], param->argv[2],param->argv[3],param->argv[4]);
		
		exit(1);
	}
	return pid;
}






