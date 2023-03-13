#include"user_service.h"
UserService::UserService(std::shared_ptr<MysqlConnection>sql_conn):sqlcoon_(sql_conn)
{

}
bool UserService::exeist(const std::string& mobile)
{//判断手机是否存在

	char sql_content[1024] = { 0 };
	sprintf(sql_content, \
		"select * from userinfo where mobile = %s", \
		mobile.c_str());
	SqlRecordSet record_set;
	if (!sqlcoon_->Execute(sql_content, record_set))
	{
		return false;
	}
	return (record_set.GetRowCount() != 0);
}
bool UserService::insert(const std::string& mobile)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, \
		"insert into userinfo (mobile) values (%s)", \
		mobile.c_str());
	return sqlcoon_->Execute(sql_content);
}