
#include "sip_app.h"


extern int  DebugMode;


sip_param  SIP_param;

FILE*  Logf = NULL;



int  main(int argc, char** argv) 
{
	int  i, j, err, invite=OFF;
	struct sigaction ca, sa;
	char*  localip;
	char*  args[MAX_ARGS];
	
	int	mport=0;
	int	vport=0;
	Buffer progname;
	Buffer hostname;
	Buffer username;
	Buffer invtname;
	Buffer portnum;
	Buffer vpn_portnum;
	Buffer vpn_localip;
	Buffer vpn_remoteip;
	Buffer logfile;

	Buffer sip_to;
	Buffer sip_from;
	Buffer sip_contact;
	Buffer sip_proxy;



////////////////////////////////////////////////////////////////
// 	引数処理
//
	progname   = make_Buffer(LNAME);
	hostname   = make_Buffer(LNAME);
	username   = make_Buffer(LNAME);
	invtname   = make_Buffer(LNAME);
	portnum    = make_Buffer(LNAME);
	vpn_portnum  = make_Buffer(LNAME);
	vpn_localip  = make_Buffer(LNAME);
	vpn_remoteip = make_Buffer(LNAME);
	logfile	   = make_Buffer(LNAME);
	DebugMode  = OFF;

	for (i=1; i<argc; i++) {
		if	  	(!strcmp(argv[i],"-p")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &portnum);}
		else if (!strcmp(argv[i],"-s")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &hostname);}
		else if (!strcmp(argv[i],"-u")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &username);}
		else if (!strcmp(argv[i],"-t")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &invtname);}
		else if (!strcmp(argv[i],"-v")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &vpn_portnum);}
		else if (!strcmp(argv[i],"-l")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &vpn_localip);}
		else if (!strcmp(argv[i],"-r")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &vpn_remoteip);}
		else if (!strcmp(argv[i],"-f")) { if (i!=argc-1) copy_s2Buffer(argv[i+1], &logfile);}
		else if (!strcmp(argv[i],"-d")) { DebugMode = ON;}
		else if (!strcmp(argv[i],"-m")) { 
			if (i!=argc-1) copy_s2Buffer(argv[i+1], &progname);
			for (i=i+1, j=0; i<argc; i++, j++) {
				if (j>=MAX_ARGS) {
					print_message("main: Too many args.... Exit!!\n");
					exit(1);
				}
				args[j] = argv[i];
			}
			args[j] = NULL;
		}
	}
	if (hostname.buf[0]=='\0' || username.buf[0]=='\0' || progname.buf=='\0') {
		print_message("Usage... %s -s proxy[:port] -u user [-p port] [-t call_user] [-v vpn_wait_port] [-l vpn_localip] [-r vpn_remoteip] [-f log_file] [-d] -m program [args]\n", argv[0]);
		exit(1);
	}

	//ローカルIPアドレスの取得 get_myipaddr()は使わない
	localip = get_localip();
	if (localip==NULL) {
		print_message("main: Failure get my IP address!!\n");
		exit(1);
	}


	if (portnum.buf[0]!='\0') {
		mport = atoi((char*)portnum.buf);
	}
	if (mport==0) {
		mport = atoi(SIP_PORTNO);
		copy_s2Buffer(SIP_PORTNO, &portnum);
	}	

	if (vpn_portnum.buf[0]!='\0') {
		vport = atoi((char*)vpn_portnum.buf);
	}
	if (vport==0) {
		vport = atoi(VPN_PORTNO);
		copy_s2Buffer(VPN_PORTNO, &vpn_portnum);
	}	

	// ログファイル
	if (DebugMode==ON && logfile.buf[0]!='\0') {
		Logf = fopen((char*)logfile.buf, "w");
		TRACE_INITIALIZE(6, Logf);
	}
	free_Buffer(&logfile);



////////////////////////////////////////////////////////////////
// 	シグナルハンドリング
//
	// for Child Process
    ca.sa_handler = sip_child_term;
    ca.sa_flags   = SA_NOCLDSTOP | SA_RESTART;
    sigemptyset(&ca.sa_mask);
    sigaction(SIGCHLD, &ca, NULL);

	// for Parent Process
//	signal(SIGINT, sip_main_term);
	sa.sa_handler = sip_main_term;
	sa.sa_flags   = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGHUP,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);



////////////////////////////////////////////////////////////////
//	パラメータ設定
//
	sip_param_init(&SIP_param);

	sip_proxy   = make_Buffer_bystr("sip:");
	sip_to	    = make_Buffer_bystr("sip:");
	sip_from	= make_Buffer_bystr("sip:");
	sip_contact = make_Buffer_bystr("sip:");

	cat_Buffer(&hostname, &sip_proxy);	

	cat_Buffer(&username, &sip_from);	
	cat_s2Buffer("@", &sip_from);
	cat_Buffer(&hostname, &sip_from);	

	cat_Buffer(&username, &sip_contact);	
	cat_s2Buffer("@", &sip_contact);
	cat_s2Buffer(localip, &sip_contact);
	cat_s2Buffer(":", &sip_contact);
	cat_Buffer(&portnum, &sip_contact);
	
	if (invtname.buf[0]!='\0') {
		invite = ON;
		SIP_param.server = OFF;
		cat_Buffer(&invtname, &sip_to);	
		cat_s2Buffer("@", &sip_to);
		cat_Buffer(&hostname, &sip_to);	
	}

//	SIP_param.expires   = 60;
	SIP_param.proxy     = dup_str((char*)sip_proxy.buf);
	SIP_param.from	    = dup_str((char*)sip_from.buf);
	SIP_param.contact   = dup_str((char*)sip_contact.buf);
	SIP_param.to        = dup_str((char*)sip_to.buf);
	SIP_param.prog      = dup_str((char*)progname.buf);
	SIP_param.argv 	    = args;
	SIP_param.username  = dup_str((char*)username.buf);
	SIP_param.localip   = localip;
	SIP_param.localport = vport;

	SIP_param.media	    = dup_str("application/vpn");
	SIP_param.protocol  = dup_str("OpenVPN");
	SIP_param.fmtlist   = dup_str("0");

	// for Server
	if (SIP_param.server==ON) {	
		SIP_param.localport = vport;
		SIP_param.keytype   = dup_str("DH");
		SIP_param.keydata   = dup_str("crypt code");

		SIP_param.vpn_localip  = dup_str((char*)vpn_localip.buf);
		SIP_param.vpn_remoteip = dup_str((char*)vpn_remoteip.buf);
	}
	

	if (file_exist(SIP_param.prog)==FALSE) {
		print_message("main: プログラムファイルが存在しない．\n");
		exit(1);
	}

	SIP_param.ppid = getpid();


//	不要変数の開放
	free_Buffer(&progname);
	free_Buffer(&hostname);
	free_Buffer(&username);
	free_Buffer(&invtname);
	free_Buffer(&portnum);
	free_Buffer(&vpn_localip);
	free_Buffer(&vpn_remoteip);
	free_Buffer(&vpn_portnum);
	free_Buffer(&sip_proxy);
	free_Buffer(&sip_to);
	free_Buffer(&sip_from);
	free_Buffer(&sip_contact);
	
	DEBUG_MODE {
		print_message("PROXY      %s\n", SIP_param.proxy);
		print_message("FROM       %s\n", SIP_param.from);
		print_message("CONTACT    %s\n", SIP_param.contact);
		print_message("TO         %s\n", SIP_param.to);
		print_message("PROGRAM    %s\n", SIP_param.prog);
		print_message("VPN_LOCAL  %s\n", SIP_param.vpn_localip);
		print_message("VPN_REMOTE %s\n", SIP_param.vpn_remoteip);
		if (SIP_param.server==ON) {
			print_message("VPN_PORT   %s\n", ltostr(SIP_param.localport));
		}
	}


////////////////////////////////////////////////////////////////
//	eXosip 初期化
//
	err = eXosip_init();
	if (err!=0) {
		if (Logf!=NULL) fclose(Logf);
		print_message("main: eXosip_init: ERROR!!\n");
		exit(1);
	}

	err = eXosip_listen_addr(IPPROTO_UDP, NULL, mport, AF_INET, 0);
	if (err!=0) {
		print_message("main: eXosip_listen_addr: ERROR!! mport = %d\n", mport);
		eXosip_quit();
		exit(1);
	}

	eXosip_set_user_agent("SIP for APP b1 rev.27");
	eXosip_add_authentication_info("infosys", "infosys", "infosysipass", NULL, NULL);



////////////////////////////////////////////////////////////////
//	SIP
//
	// REGISTER
	do {
		err = send_regist(&SIP_param);		
	} while(err!=EXOSIP_REGISTRATION_SUCCESS);


	// INVITE 送信
	if (invite==ON) {
		err = send_invite(&SIP_param);
		if (err==0) {
			err = session_start(&SIP_param);	// ここで子プロセスを起動
			if (err>0) {
				session_term_wait(&SIP_param);
				session_terminate(&SIP_param);
			}
		}
		else {
			print_message("main: 相手が呼び出しに応じません．INVITE 失敗!!\n");
		}
	}


	// INVITE 待ち
	else {
		Loop {
			err = recive_invite(&SIP_param);	// ここで子プロセスを起動
			if (err>0) {
				err = session_start(&SIP_param);
				if (err==0) {
					session_term_wait(&SIP_param);
					session_terminate(&SIP_param);
				}
			}
		}
	}


	sip_main_term(0);
}
	








//////////////////////////////////////////////////////////////////////////
// プログラムの終了
//

//
// for メインプロセス
//
void  sip_main_term(int signal)
{  
	int err;

	err = sip_terminate(&SIP_param); 
	if (err==1) return;		// スレッドはそのままリターンさせる．
	
	if (Logf!=NULL) {
		fclose(Logf);
		Logf = NULL;
	}
	exit(signal);
}




//
// for 子プロセス
//
void  sip_child_term(int signal)
{
	pid_t pid = 0;
    int ret;
       
    do {          
        pid = waitpid(-1, &ret, WNOHANG);
    } while(pid>0);
	
	SIP_param.inChild = OFF;
}





