#include "user_event_hander.h"

#include<arpa/inet.h>
#include<assert.h>
#include<error.h>
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netdb.h>
#include<unistd.h>
#include"DispatchMsgService.h"
#include"sqlconnection.h"
#include"iniconfig.h"
#include"user_service.h"
UserEventHandler::UserEventHandler():iEventHandler("UserEventHandler") {
	//订阅事件的处理
	DispatchMsgService::getInstance()->subscribe(EEVENTID_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_LOGIN_REQ, this);
	thread_mutex_create(&pm_);

}
UserEventHandler::~UserEventHandler() {
	//退订事件的处理
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_LOGIN_REQ, this);
	thread_mutex_destroy(&pm_);

}

iEvent* UserEventHandler::handle(const iEvent* ev) {
	if (ev == NULL) {
		//LOG_ERROR("INPUT EV IS ERROR");
		printf("input ev is NULL");
	}
	u32 eid = ev->get_eid();

	if (eid == EEVENTID_GET_MOBILE_CODE_REQ) {
		return handle_mobile_code_req((MobileCodeReqEv*)ev);
	}
	else if (eid == EEVENTID_LOGIN_REQ) {
		return handler_login_req((LoginReqEv*) ev);
	}
	else if (eid == EEVENTID_RECHARGE_REQ) {
		//return hanle_recharge_req((RechargeEv*) ev);
	}
	else if (eid == EEVENTID_GET_ACCOUNT_BALANCE_REQ) {
		//return handle_get_account_balance_req((GetAccountBalanceEv*) ev);
	}
	else if (eid == EEVENTID_LIST_ACCOUNT_RECORDS_REQ) {
		//return handle_list_account_records_req((ListAccountRecordsReqEv)* ev);
	}
	return NULL;
}

MobileCodeRspEv* UserEventHandler::handle_mobile_code_req(MobileCodeReqEv* ev) {
	i32 icode = 0;
	std::string mobile_ = ev->get_mobile();
	//LOG_DEBUG("try to get mobile phone %s validate code.",mobile_c_str());
	printf("try to get mobile phone %s validate code.",mobile_.c_str());
	icode = code_gen();
	thread_mutex_lock(&pm_);
	m2c_[mobile_] = icode;
	thread_mutex_unlock(&pm_);
	printf("mobile:%s,code:%d\n", mobile_.c_str(), icode);

	return new MobileCodeRspEv(200, m2c_[mobile_]);
}

i32 UserEventHandler::code_gen(){
	i32 code = 0;
	srand((unsigned int)time(NULL));

	code = (unsigned int)(rand() % (999999 - 100000) + 100000);

	return code;
}

LoginResEv* UserEventHandler::handler_login_req(LoginReqEv*ev)
{
	LoginResEv* loginEv = nullptr;
	std::string mobile = ev->get_mobile();
	i32 icode = ev->get_icode();
	LOG_DEBUG("try to handll login ev, mobile = %s, icode = %d", mobile, icode);
	thread_mutex_lock(&pm_);
	auto it = m2c_.find(mobile);
	if (((it != m2c_.end()) && (it->second != icode)) || it == m2c_.end())
	{
		return new LoginResEv(ERRC_INVALID_DATA);

	}
	thread_mutex_unlock(&pm_);
	if (loginEv) return loginEv;
	//如果验证成功，则要判断用户在数据库中,不存在则插入用户记录
	//下面实现数据库连接
	std::shared_ptr<MysqlConnection>mysqlConn(new MysqlConnection);
	st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	if (!mysqlConn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("Database init failed, exit!\n");
		return new LoginResEv(ERRO_PROCCESS_FAILED);
	}
	UserService us(mysqlConn);
	bool result = false;
	if (!us.exeist(mobile))
	{
		result = us.insert(mobile);
		if (!result)
		{
			LOG_ERROR("insert user(%s) to db failed", mobile.c_str());
			return new LoginResEv(ERRO_PROCCESS_FAILED);
		}
	}
	return new LoginResEv(ERRC_SUCCESS);
}
