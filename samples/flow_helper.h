#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "flow_impl.h"
#include "vsock.h"

class CFlowObject
{
public:
	CFlowObject();
	virtual ~CFlowObject();

	flow void wait(void** param = NULL);
	flow void time_wait(DWORD interval);
	void cancel_wait();

	void signal(void* param = NULL);

	FLOW_OBJECT* getObj() { return &m_obj; }

	static void check_timer();
	static void add_timer_to_map(DWORD ms, CFlowObject* pObj);
	static bool remove_timer_from_map(DWORD ms, CFlowObject* pObj);

	typedef std::multimap<DWORD, CFlowObject*> TimerMap;
	static TimerMap s_timer_map;

protected:
	FLOW_OBJECT	m_obj;
	DWORD		m_waiting_time;
};

class CFlowSocket
{
public:
	CFlowSocket();
	virtual ~CFlowSocket();

	int create();
	flow int connect(const char* url, WORD port);
	flow void send(const char* buf, int len);
	flow int recv(char* buf, int len);
	void close();

	CFlowObject* getExceptFobj() { return &m_except_obj; }

	static void event_loop();
	static void stop_event_loop();

protected:
	static std::map<int, CFlowSocket*>	s_socket_map;
	static bool		s_loop_running;

	int			m_s;
	CFlowObject	m_read_obj, m_write_obj, m_except_obj;
};

