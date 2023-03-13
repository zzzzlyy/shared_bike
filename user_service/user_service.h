#ifndef BRKS_USER_SERVICE_H_
#define BRKS_USER_SERVICE_H_
#include"sqlconnection.h"
#include<memory>
class UserService
{
public:
	UserService(std::shared_ptr<MysqlConnection>sql_conn);
	bool exeist(const std::string& mobile);//�ж��ֻ��Ƿ����
	bool insert(const std::string& mobile);//������Ϣ

private:
	std::shared_ptr<MysqlConnection> sqlcoon_;
};



#endif // !BRKS_USER_SERVICE_H_
