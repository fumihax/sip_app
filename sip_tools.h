/**
  SIP用ツール　                          by Fumi.Iseki
                                         (C) 2006 9/1  


	注意：
		eXosip_lock(), eXosip_unlock() の間で sleep() しないこと．
*/

#ifndef _SIP_TOOLS_H
#define _SIP_TOOLS_H


#include "common.h"
#include "tools.h"
#include "network.h"
#include "buffer.h"

#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <syslog.h>

#include <eXosip2/eXosip.h>


/**/

#define  SIP_USLEEP_TIME 10000


typedef struct _sip_param
{
	int tid;				    // ID for transactions (to be used for answers)
	int rid;				    // REGISTER ID
	int iid;				    // INVITE ID
	int cid;				    // CALL ID
	int did;				    // DIALOG ID

	int type;				    // 受信した Event Type
	int auth;				    // 認証を行うか？　デフォルトで ON
	int server;				    // サーバか？
	int ppid;				    // 親プロセス（メインプロセス）の PID
	int cpid;				    // 子プロセス（起動したプログラム）の PID

	char*  prog;			    // 起動するプログラムのパス
	char** argv;			    // 起動するプログラムへの引数
	char** env;				    // 起動するプログラムへの環境変数

	int inRegist;			    // （再）REGIST中
	int inInvite;			    // INVITE中
	int inWait;				    // 待機中
	int inSession;			    // セッション中
	int inExit;				    // 終了作業中
	int inChild;			    // 子プロセス作動中

	char* from;				    // SIP from
	char* to;				    // SIP to
	char* proxy;			    // SIP Proxy
	char* contact;			    // SIP contact
	int   expires;			    // SIP expires

	char*  localip;			    // 自分のIPアドレス
	char*  remoteip;		    // 相手のIPアドレス
	char*  vpn_localip;		    // 自分のVPN IPアドレス
	char*  vpn_remoteip;	    // 相手のVPN IPアドレス
	char*  username;		    // 自分のユーザ名（番号）
	char*  media;			    // SDPメディア情報
	char*  protocol;		    // SDPプロトコル
	char*  fmtlist;			    // SDPフォーマットリスト
	char*  keytype;			    // 暗号用キータイプ
	char*  keydata;			    // 暗号用キー
	int    localport;		    // 自分の子プロセスのポート番号．または自分のメインプロセスのポート番号．
	int    remoteport;		    // 相手の子プロセスのポート番号．または相手のメインプロセスのポート番号．

	osip_message_t* mssg;	    // 汎用メッセージ
	osip_message_t* rgst;	    // レジスト用メッセージ
	osip_message_t* invt;	    // インバイト用メッセージ

    struct eXosip_t* context;
} sip_param;


void sip_param_init(sip_param* param);
void sip_param_free(sip_param* param);
int  sip_terminate(sip_param* param);

int  sip_regist_send(sip_param* param);
int  sip_regist_resend(sip_param* param);
int  sip_regist_answer(sip_param* param);
int  sip_regist_send_and_answer(sip_param* param);
int  sip_regist_thread(sip_param* param);
static void sip_regist_thread_loop(void* arg);

int  sip_invite_send(sip_param* param);
int  sip_invite_answer(sip_param* param);
int  sip_invite_send_and_answer(sip_param* param);
int  sip_invite_wait(sip_param* param);
int  sip_invite_accept(sip_param* param);

int  sip_session_term_wait(sip_param* param);
int  sip_session_terminate(sip_param* param);
int  sip_session_start(sip_param* param);

int  sip_send_ack(sip_param* param);
int  sip_event_get(sip_param* param);
int  sip_process_fork(sip_param* param);


#endif
