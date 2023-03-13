#include"NetworkInterface.h"
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"DispatchMsgService.h"
//�м�  ConnectSession������C���͵ĳ�Ա����
static ConnectSession* session_init(int fd, struct bufferevent* bev)
{
	ConnectSession* temp = nullptr;
	temp = new ConnectSession();
	if (!temp)
	{
		fprintf(stderr, "malloc failed, reason: %m\n");
		return nullptr;
	}
	memset(temp, '\0', sizeof(ConnectSession));
	temp->bev = bev;
	temp->fd = fd;
}

void session_free(ConnectSession* cs)
{
	if (cs)
	{
		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
		}
		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}
	}
	delete cs;
}
void session_reset(ConnectSession* cs)
{
	if (cs)
	{
		if (cs->read_buf)
		{
			delete[]cs->read_buf;
			cs->read_buf = nullptr;
		}
		if (cs->write_buf)
		{
			delete[]cs->write_buf;
			cs->write_buf = nullptr;
		}
		cs->session_stat = SESSION_STATUS::SS_REQUEST;
		cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;
		cs->message_len = 0;
		cs->read_message_len = 0;
		cs->read_header_len = 0;

	}
}

NetworkInterface::NetworkInterface()
{
	base_ = nullptr;
	listener_ == nullptr;
}

NetworkInterface::~NetworkInterface()
{
	close();
}

bool NetworkInterface::start(int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	base_ = event_base_new();
	//�������������ûص���NetworkInterface::listener_cb��
	listener_ = evconnlistener_new_bind(base_, NetworkInterface::listener_cb, base_,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		512, (struct sockaddr*)&sin,
		sizeof(struct sockaddr_in));
	return true;
}
void NetworkInterface::close()
{
	if (base_)
	{
		event_base_free(base_);
		base_ = nullptr;
	}
	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}
}

void NetworkInterface::listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
		struct sockaddr* sock, int socklen, void* arg)
{
	struct event_base* base = (struct event_base*)arg;
	LOG_DEBUG("accept a client %d\n", fd);

	//Ϊ����ͷ��˷���һ��buffevent
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	ConnectSession* cs = session_init(fd, bev);
	cs->session_stat = SESSION_STATUS::SS_REQUEST;
	cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;
	strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));
	LOG_DEBUG("remote is : %s\n", cs->remote_ip);
	bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);
	bufferevent_settimeout(bev, 10, 10);//��ʱֵӦ�����������ļ���
}

/*

			4�ֽ�     2���ֽ�   4���ֽ�          
�����ʽ		FBEB	  �¼�ID	   ���ݳ���N	   ��������

1.����ʶ��  ��ͷ���������ʶ, ������ʶ���Ŀ�ʼ
2.�¼����ͣ��¼�ID, �̶������ֽڱ�ʾ
2. ���ݳ��ȣ����ݰ��Ĵ�С, �̶�����4�ֽڡ�
3. �������ݣ���������, ����Ϊ����ͷ����ĳ��ȴ�С��

*/

void NetworkInterface::handle_request(struct bufferevent* bev, void* arg)
{
	ConnectSession* cs = (ConnectSession*)arg;
	if (cs->session_stat != SESSION_STATUS::SS_REQUEST)
	{
		LOG_WARN("NetworkInterface::handle_request - wrong session state[%d]", cs->session_stat);
		return;
	}
	if (cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER)//��ȡͷ������
	{
		i32 len = bufferevent_read(bev, cs->header + cs->read_header_len, MESSAGE_HEADER_LEN - cs->read_header_len);
		cs->read_header_len += len;

		cs->header[cs->read_header_len] = '\0';

		LOG_DEBUG("recv from client <<<< %s\n", cs->header);
		if (cs->read_header_len == MESSAGE_HEADER_LEN)
		{
			if (strncmp(cs->header, MESSAGE_HEADER_ID, strlen(MESSAGE_HEADER_ID)) == 0)
			{
				cs->eid = *((u16*)(cs->header + 4));
				cs->message_len = *((i32*)(cs->header + 6));
				LOG_DEBUG("read %d bytes in header", cs->read_header_len);

				if (cs->message_len < 1 || cs->message_len > MAX_MESSAGE_LEN)
				{
					LOG_ERROR("NetworkInterface::handle_request wrong message, len:%u", cs->message_len);
					bufferevent_free(bev);
					session_free(cs);
					return;
				}
				cs->read_buf = new char[cs->message_len];
				cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;
				cs->read_message_len = 0;
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request- Invalid request from %s\n", cs->remote_ip);
				//ֱ�ӹر����󣬲������κ���Ӧ����ֹ�ͷ��˶�����̽
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
	}
	
	if (cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE && evbuffer_get_length(bufferevent_get_input(bev)) > 0)
	{
		i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);
		cs->read_message_len += len;
		LOG_DEBUG("NetworkInterface::handle_request- %d bytes, message len %u read len: %u\n", len, cs->message_len, cs->read_message_len);
		if (cs->read_message_len == cs->message_len)
		{
			cs->session_stat = SESSION_STATUS::SS_RESPONSE;
			iEvent*ev = DispatchMsgService::getInstance()->parseEvent(cs->read_buf, cs->read_message_len, cs->eid);

			delete[]cs->read_buf;
			cs->read_buf = nullptr;
			cs->read_message_len = 0;
			if (ev)
			{
				ev->set_args(cs);
				DispatchMsgService::getInstance()->enqueue(ev);
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request EV IS NULL, remote ip %s\n", cs->remote_ip);
				//ֱ�ӹر����󣬲������κ���Ӧ����ֹ�ͷ��˶�����̽
				bufferevent_free(bev);
				session_free(cs);
				return;
			}

		}
	}
}

void NetworkInterface::handle_response(struct bufferevent* bev, void* arg)
{
	LOG_DEBUG("hello");
}
//��ʱ�����ӹرա���д������쳣���ָ���Ļص�����
void NetworkInterface::handle_error(struct bufferevent* bev, short event, void* arg)
{
	ConnectSession* cs = (ConnectSession*)arg;
	LOG_DEBUG("hello");
	//���ӹر�
	if (event && BEV_EVENT_EOF)
	{
		LOG_DEBUG("CONNECTION TIMEOUT");
	}
	else if ((event && BEV_EVENT_TIMEOUT) && (event && BEV_EVENT_READING))
	{
		LOG_WARN("NetworkInterface:: READING TIMEOUT....");
	}
	else if ((event && BEV_EVENT_TIMEOUT) && (event && BEV_EVENT_WRITING))
	{
		LOG_WARN("NetworkInterface:: WRITING TIMEOUT....");
	}
	else if (event && BEV_EVENT_ERROR)
	{
		LOG_ERROR("NetworkInterface::SOME OTHER ERROR....");
	}
	bufferevent_free(bev);
	session_free(cs);
}

void NetworkInterface::network_event_dispatch()
{ 
	//���������¼�
	event_base_loop(base_, EVLOOP_NONBLOCK);
	//������Ӧ�¼����ظ���Ӧ��Ϣ
	DispatchMsgService::getInstance()->handleAllResponseEvent(this);
}

void NetworkInterface::send_response_messege (ConnectSession* cs)
{
	if (cs->response == nullptr)
	{
		bufferevent_free(cs->bev);
		if (cs->request)
		{
			delete cs->request;
		}
		session_free(cs);
	}
	else
	{
		bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
		session_reset(cs);
	}
}