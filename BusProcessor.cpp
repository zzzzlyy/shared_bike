#include"BusProcessor.h"
#include"SqlTables.h"
BusinessProcessor::BusinessProcessor(std::shared_ptr<MysqlConnection> conn)
	:mysqlconn_(conn), ueh_(new UserEventHandler())
{

}
bool BusinessProcessor::init()//��ʼ����
{
	SqlTables tables(mysqlconn_);
	tables.CreateUserInfo();
	tables.CreateBikeInfo();
	return true;
}

BusinessProcessor::~BusinessProcessor()
{
	ueh_.reset();

}