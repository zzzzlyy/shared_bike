#ifndef BRKS_COMMON_DATASERVER_H_
#define BRKS_COMMON_DATASERVER_H_
#include<memory>
#include"sqlconnection.h"
#include"glo_def.h"
class SqlTables
{
public:
	SqlTables(std::shared_ptr<MysqlConnection>sqlConn) :sqlconn_(sqlConn)
	{

	}
	bool CreateUserInfo()
	{
		const char* pUserInfoTable = "\
									CREATE TABLE IF NOT EXISTS userinfo(\
								    id			int(16)			NOT NULL primary key auto_increment,\
									mobile		varchar(16)		NOT NULL default	'13000000000',\
									username	varchar(128)	NOT NULL default '',\
									registertm	timestamp		NOT NULL default CURRENT_TIMESTAMP,\
									money		int(4)			NOT NULL default 0,\
									INDEX		mobile_index(mobile)\
									)";
										
		if (!sqlconn_->Execute(pUserInfoTable))
		{
			LOG_ERROR("creat table failed!, error msg: %s", sqlconn_->GetErrInfo());
			return false;
		}
		return true;
	}

	bool CreateBikeInfo()
	{
		const char* pBikeInfoTable = "\
									CREATE TABLE IF NOT EXISTS bikeinfo(\
								    id			int				NOT NULL primary key auto_increment,\
									devno		int				NOT NULL,\
									status		tinyint(1)		NOT NULL default 0,\
									trouble		int				NOT NULL default 0,\
									tmsg		varchar(256)	NOT NULL default	'',\
									latitude	double(10,6)	NOT NULL default 0,\
									longtitute	double(10,6)	NOT NULL default 0,\
									unique(devno)\
									)";

		if (!sqlconn_->Execute(pBikeInfoTable))
		{
			LOG_ERROR("creat table failed!, error msg: %s", sqlconn_->GetErrInfo());
			return false;
		}
		return true;
	}


private:
	std::shared_ptr<MysqlConnection>sqlconn_;

};

#endif // !BRKS_COMMON_DATASERVER_H_
