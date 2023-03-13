#ifndef DATASTORE_MYSQL_CONNECTION_H_
#define	DATASTORE_MYSQL_CONNECTION_H_

#include<mysql/mysql.h>
#include<string>
#include<mysql/errmsg.h>
#include<assert.h>
#include"glo_def.h"

class  SqlRecordSet
{
public:
	SqlRecordSet() :m_pRes(nullptr)
	{

	}
	explicit SqlRecordSet(MYSQL_RES* pRes)
	{
		m_pRes = pRes;
	}
	~SqlRecordSet()
	{
		if (m_pRes)
		{
			mysql_free_result(m_pRes);
		}
	}
	//设置结果集
	inline void SetResult(MYSQL_RES* pRes)
	{
		assert(m_pRes == nullptr);//如果此时已经保存了结果集了，那么应该让程序抱错，防止内存泄露
		if (m_pRes)
		{
			LOG_WARN("the mysql_res has already stored result maybe will cause memory leak");
		}
		m_pRes = m_pRes;
	}
	inline MYSQL_RES* GetResult()
	{
		return m_pRes;
	}
	void FetchRow(MYSQL_ROW& row)
	{
		row = mysql_fetch_row(m_pRes);
	}
	inline i32 GetRowCount()
	{
		return m_pRes->row_count;
	}

private:
	MYSQL_RES* m_pRes;
};

class MysqlConnection
{
public:
	MysqlConnection();
	~MysqlConnection();
	MYSQL* Mysql()
	{
		return mysql_;
	}
	bool Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb);
	bool Execute(const char* szSql);
	bool Execute(const char* szSql, SqlRecordSet& recordSet);//mysql结果集 默认mysql结果集是MYSQL_RES*
	int EscapeString(const char* pSrc, int nSrcLen, char* pDest);//转义sql语句 因为sql语句可能含有特殊字符
	void close();
	const char* GetErrInfo();
	void Reconnect();
private:
	MYSQL* mysql_;
};










#endif // !DATASTORE_MYSQL_CONNECTION_H_
