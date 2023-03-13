/**********
负责分发消息服务模块，其实就是把外部收到的消息，转化成内部事件，也就是data->msg->event的解码过程，
然后再把事件投递至线程池的消息队列，由线程池调用其process 方法对事件进行处理，最终调用每个event的
handler方法来处理event，此时每个event handler需要subscribe该	event后才会被调用到
**********/
#ifndef  BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_
#define  BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_


#include<vector>
#include<map>
#include<queue>
#include"ievent.h"
#include"eventtype.h"
#include"iEventHandler.h"
#include"threadpool/thread_pool.h"
#include "NetworkInterface.h"

class DispatchMsgService
{
protected:
	DispatchMsgService();
public:
	virtual ~DispatchMsgService();

	virtual BOOL open();
	virtual void close();

	virtual void subscribe(u32 eid, iEventHandler* handler);
	virtual void unsubscribe(u32 eid, iEventHandler* handler);

	//把时间投递到线程池中进行处理
	virtual i32 enqueue(iEvent* ev);
	
	//线程池回调函数
	static void svc(void* arg);

	//对具体事件进行分发处理
	virtual iEvent* process(const iEvent* ev);

	//单例模式
	static DispatchMsgService* getInstance();

	iEvent* parseEvent(const char* msg, u32 len, u32 eid);

	void handleAllResponseEvent(NetworkInterface* interface);

protected:

		thread_pool_t *tp;
		static DispatchMsgService* DMS_;//单例模式中指向自己的指针

		typedef std::vector < iEventHandler*> T_EventHandelers;//
		typedef std::map<u32, T_EventHandelers> T_EventHandelersMap;//事件对应的处理函数
		T_EventHandelersMap subscribers_;//绑定事件与处理者

		bool svr_exit_;// 服务是否退出标志
		static std::queue<iEvent*>response_events;
		static pthread_mutex_t queue_mutex;

};
#endif // ! BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_
