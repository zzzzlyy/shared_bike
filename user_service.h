#ifndef BRKS_USER_SERVICE_H_
#define BRKS_USER_SERVICE_H_
#include"sqlconnection.h"
#include<memory>
class UserService
{
public:
	UserService(std::shared_ptr<MysqlConnection>sql_conn);
	bool exeist(const std::string& mobile);//判断手机是否存在
	bool insert(const std::string& mobile);//插入信息

private:
	std::shared_ptr<MysqlConnection> sqlcoon_;
};



#endif // !BRKS_USER_SERVICE_H_
