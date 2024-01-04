/**
  SDP用ツール　                          by Fumi.Iseki and Kikuchi
                                         (C) 2006 9/6  
*/



#include "sip_tools.h"
#include "sdp_tools.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SDP 
//			注意： tは必須．パラメータの順序も考慮する．m は一番最後．
//

/**
int  sdp_set_body(sip_param* param, osip_message_t* mesg)

	機能：param の変数に従って，mesg へ SDPボディを格納する．

	引数：param : SIP用パラメータ
		　mesg  : 転送用の SIPメッセージへのポインタ．

	戻り値：0： 正常終了．
			0未満：エラー．

*/
int  sdp_set_body(sip_param* param, osip_message_t* mesg)
{
	time_t tsec;
	char* buf;
	char  proto_fmtlst[LDATA];
	sdp_message_t* sdp = NULL;

	time(&tsec);					// 1900年からではない(1970)．要修正?．
	memset(proto_fmtlst, 0, LDATA);
	snprintf(proto_fmtlst, LDATA-1, "%s %d", param->protocol, 0);

	sdp_message_init(&sdp);					
	sdp_message_v_version_set(sdp, "0");
	sdp_message_o_origin_set(sdp, param->username, ultostr(tsec), ultostr(tsec), "IN", "IP4", param->localip);
	sdp_message_s_name_set(sdp, "-");
	sdp_message_t_time_descr_add(sdp, "0", "0");

	sdp_message_m_media_add(sdp, param->media, ltostr(param->localport), NULL, proto_fmtlst);
	sdp_message_k_key_set(sdp, 0, param->keytype, param->keydata);
	sdp_message_c_connection_add(sdp, 0, "IN", "IP4", ltostr(param->localport), NULL, NULL);
	sdp_message_a_attribute_add (sdp, 0, "IP4", param->localip);
	sdp_message_a_attribute_add (sdp, 0, "PORT", ltostr(param->localport));
	// added by Kikuchi
	sdp_message_a_attribute_add (sdp, 0, "VPN_LOCAL_ADDR",  param->vpn_localip);
	sdp_message_a_attribute_add (sdp, 0, "VPN_REMOTE_ADDR", param->vpn_remoteip);

	sdp_message_to_str(sdp, &buf);
	sdp_print_body(sdp);
	osip_message_set_body(mesg, buf, strlen(buf));
	osip_message_set_content_type(mesg, "application/sdp");

	free(buf);
	sdp_message_free(sdp);

	return 0;
}




/**
int  sdp_set_body2(sip_param* param, osip_message_t* mesg)

	機能：param の変数に従って，mesg へ SDPボディを格納する もう一つの方法．
　　　　　分かり易いがスマートではない．

	引数：param : SIP用パラメータ
		　mesg  : 転送用の SIPメッセージへのポインタ．

	戻り値：0： 正常終了．
			0未満：エラー．

*/
int  sdp_set_body2(sip_param* param, osip_message_t* mesg)
{
	time_t tsec;
  	char buf[BUFSZ];


	time(&tsec);					// 1900年からではない(1970)．要修正?．

	// added by Kikuchi: VPN_LOCAL_ADDR and VPN_REMOTE_ADDR
  	snprintf (buf, BUFSZ,
    			"v=0\r\n"
            	"o=%s %ld %ld IN IP4 %s\r\n"
            	"s=-\r\n"
	 			"t=0 0\r\n"
	 			"m=%s %d %s %s\r\n"
	 			"k=%s:%s\r\n" 
 				"c=IN IP4 %s\r\n"
 				"a=IP4:%s\r\n"
 				"a=PORT:%d\r\n"
 				"a=VPN_LOCAL_ADDR:%s\r\n"
 				"a=VPN_REMOTE_ADDR:%s\r\n",
				param->username, tsec, tsec, param->localip, 
			 	param->media, param->localport, param->protocol, param->fmtlist,
				param->keytype, param->keydata,
				param->localip, 
			 	param->localip, 
				param->localport,
				param->vpn_localip,
				param->vpn_remoteip);

	osip_message_set_body(mesg, buf, strlen(buf));
	osip_message_set_content_type(mesg, "application/sdp");

	return 0;
}






/**
sdp_message_t*  sdp_get_body(sip_param* param)

	機能：相手の返答から SDPボディを取り出す．

	引数：param : SIP用パラメータ

	戻り値：SDPボディへのポインタ．
			エラーの場合は NULL．

*/
sdp_message_t*  sdp_get_body(sip_param* param)
{
    sdp_message_t* sdp = NULL;

    sdp = eXosip_get_remote_sdp(param->did);
	if (sdp==NULL) {
    	sdp = eXosip_get_remote_sdp_from_tid(param->tid);
	}

	return sdp;
}






/**
char*  sdp_get_media_keytype(sdp_message_t* sdp, int pos) 

	機能：SDPボディから media の keytype を取り出す．

	引数：sdp : SDPボディへのポインタ．
		  pos : 何番目のメディアかを指定する．通常は 0．

	戻り値：keytype を示す文字列．freeする必要あり．
			エラーの場合は NULL

*/
char*  sdp_get_media_keytype(sdp_message_t* sdp, int pos) 
{
	char* ret = NULL;
	sdp_media_t* med; 

	//if (sdp==NULL || sdp->m_medias==NULL) return NULL;
	if (sdp==NULL) return NULL;

	if (pos<0) pos = 0;
	med = (sdp_media_t*)osip_list_get(&sdp->m_medias, pos);
	if (med==NULL || med->k_key==NULL) return NULL;

	ret = dup_str(med->k_key->k_keytype);

	return ret;
}	




/**
char*  sdp_get_media_keytype(sdp_message_t* sdp, int pos) 

	機能：SDPボディから media の keydata を取り出す．

	引数：sdp : SDPボディへのポインタ．
		  pos : 何番目のメディアかを指定する．通常は 0．

	戻り値：keydata を示す文字列．freeする必要あり．
			エラーの場合は NULL

*/
char*  sdp_get_media_keydata(sdp_message_t* sdp, int pos) 
{
	char* ret = NULL;
	sdp_media_t* med; 

	//if (sdp==NULL || sdp->m_medias==NULL) return NULL;
	if (sdp==NULL) return NULL;

	if (pos<0) pos = 0;
	med = (sdp_media_t*)osip_list_get(&sdp->m_medias, pos);
	if (med==NULL || med->k_key==NULL) return NULL;

	ret = dup_str(med->k_key->k_keydata);

	return ret;
}	





/**
char*  sdp_get_media_attr(sdp_message_t* sdp, char* key, int pos) 

	機能：SDPボディから media の 属性の値を取り出す．

	引数：sdp : SDPボディへのポインタ．
		  key : 属性(attribute) を指定する．
		  pos : 何番目のメディアかを指定する．通常は 0．

	戻り値：属性の値を示す文字列．freeする必要あり．
			属性が無い．またはエラーの場合は NULL

*/
char*  sdp_get_media_attr(sdp_message_t* sdp, char* key, int pos) 
{
	int   i;
	char* ret = NULL;
	sdp_media_t* med; 

	//if (sdp==NULL || key==NULL || sdp->m_medias==NULL) return NULL;
	if (sdp==NULL || key==NULL) return NULL;

	if (pos<0) pos = 0;
	med = (sdp_media_t*)osip_list_get(&sdp->m_medias, pos);
	//if (med==NULL || med->a_attributes==NULL) return NULL;
	if (med==NULL) return NULL;

	i = 0;
	while(!osip_list_eol(&med->a_attributes, i)) {
		sdp_attribute_t* attr;
		attr = (sdp_attribute_t*)osip_list_get(&med->a_attributes, i);
		if (!strcmp(key, attr->a_att_field)) {
			ret = dup_str(attr->a_att_value);
			break;;
		}
		i++;
	}

	return ret;
}	

	





/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	for Debug
//

/**
void  sdp_show_body(sdp_message_t* sdp)

	機能：SDPボディの内容の一部を取り出してプリントする．デバッグ用．
		  取り出し方法の確認用．

	引数：sdp : SDPボディへのポインタ．

		see /usr/local/include/osipparser2/sdp_message.h 

*/
void  sdp_show_body(sdp_message_t* sdp)
{
	int i, j;

	if (sdp==NULL) return;

	print_message("\n-------------------------------------------\n");
	print_message("v_version  -> %s\n", sdp->v_version);
	print_message("o_username -> %s\n", sdp->o_username);
	print_message("o_sess_id  -> %s\n", sdp->o_sess_id);
	print_message("o_sess_ver -> %s\n", sdp->o_sess_version);
	print_message("o_nettype  -> %s\n", sdp->o_nettype);
	print_message("o_addrtype -> %s\n", sdp->o_addrtype);
	print_message("o_addr     -> %s\n", sdp->o_addr);
	print_message("s_name     -> %s\n", sdp->s_name);
	print_message("i_info     -> %s\n", sdp->i_info);
	print_message("u_uri      -> %s\n", sdp->u_uri);
	print_message("z_adjust   -> %s\n", sdp->z_adjustments);

	if (sdp->c_connection!=NULL) {
		print_message("c_nettype  -> %s\n", sdp->c_connection->c_nettype);
		print_message("c_addrtype -> %s\n", sdp->c_connection->c_addrtype);
		print_message("c_addr     -> %s\n", sdp->c_connection->c_addr);
	}


	// Media
	i = 0;
	while(!osip_list_eol(&sdp->m_medias, i)) {
		sdp_media_t*  med; 
		med = (sdp_media_t*)osip_list_get(&sdp->m_medias, i);

		print_message("\nMedia %d\n", i);
		print_message("m_media    -> %s\n", med->m_media);
		print_message("m_port     -> %s\n", med->m_port);
		print_message("m_num_port -> %s\n", med->m_number_of_port);
		print_message("m_protocol -> %s\n", med->m_proto);

		if (med->k_key!=NULL) {
			print_message("m_keytype  -> %s\n", med->k_key->k_keytype);
			print_message("m_keydata  -> %s\n", med->k_key->k_keydata);
		}

		j = 0;
		while(!osip_list_eol(&med->c_connections, j)) {
			sdp_connection_t* con;
			con = (sdp_connection_t*)osip_list_get(&med->c_connections, j);
			print_message("m_nettype  -> %s\n", con->c_nettype);
			print_message("m_addrtype -> %s\n", con->c_addrtype);
			print_message("m_addr     -> %s\n", con->c_addr);
			j++;
		}

		j = 0;
		while(!osip_list_eol(&med->a_attributes, j)) {
			sdp_attribute_t* buf;
			buf = (sdp_attribute_t*)osip_list_get(&med->a_attributes, j);
			print_message("m_att_field -> %s\n", buf->a_att_field);
			print_message("m_att_value -> %s\n", buf->a_att_value);
			j++;
		}

		i++;
	}

	print_message("-------------------------------------------\n\n");

	return;
}	





/**
int  sdp_print_body(sdp_message_t* sdp)

	機能：SDPボディの内容をプリントする．デバッグ用．
		  sdp_show_body() と違い，ただ垂れ流すだけ．

	引数：sdp : SDPボディへのポインタ．

		see /usr/local/include/osipparser2/sdp_message.h 

*/
void  sdp_print_body(sdp_message_t* sdp)
{
	char* str = NULL;

	if (sdp==NULL) return;
	sdp_message_to_str(sdp, &str);
	
	if (str==NULL) return;
	print_message("\n-------------------------------------------\n");
	print_message(str);
	print_message("-------------------------------------------\n\n");
	free(str);
	
	return; 
}








//////////////////////////////////////////////////////////////////////////////////////////
//
//	Junk Code
//
#if 0

	char* buf;
	sdp_message_t* sdp = NULL;

	sdp_message_init(&sdp);
	sdp_message_v_version_set(sdp, "0");
	sdp_message_o_origin_set(sdp, "9000", "1111", "1111", "IN", "IP4", "202.26.158.1");
	sdp_message_s_name_set(sdp, "-");
	sdp_message_t_time_descr_add(sdp, "0", "0");
	sdp_message_m_media_add(sdp, "application", "5060", NULL, "APP 0");
	sdp_message_k_key_set(sdp, 0, "-", "-");
	sdp_message_c_connection_add(sdp, 0, "IN", "IP4", "202.26.158.1", NULL, NULL);
	sdp_message_a_attribute_add(sdp, 0, "IP", "202.26.158.1");
	sdp_message_a_attribute_add(sdp, 0, "PORT", "5011");

	sdp_message_to_str(sdp, &buf);
	sdp_print_body(sdp);
*/



/*
	if (sdp->k_key!=NULL) {
		print_message("k_keytype  -> %s\n", sdp->k_key->k_keytype);
		print_message("k_keydata  -> %s\n", sdp->k_key->k_keydata);
	}
*/











#endif
