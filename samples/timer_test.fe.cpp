#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "flow_impl.h"
#include "vsock.h"
//#include "skip_flow.h"

class CFlowObject
{
public:
	CFlowObject()
	{
		flow_init(&m_obj);
		m_waiting_time = (DWORD)-1;
	}

	virtual ~CFlowObject()
	{
		cancel_wait();
	}

	flow void wait(void** param = NULL)
	{
		flow_wait(&m_obj, param);
	}

	flow void time_wait(DWORD interval)
	{
		if (m_waiting_time != (DWORD)-1)
			remove_timer_from_map(m_waiting_time, this);

		m_waiting_time = get_cur_tick() + interval;
		add_timer_to_map(m_waiting_time, this);
		flow_wait(&m_obj, NULL);
	}

	void cancel_wait()
	{
		if (m_waiting_time != (DWORD)-1)
			remove_timer_from_map(m_waiting_time, this);

		flow_cancel_wait(&m_obj);
	}

	void signal(void* param = NULL)
	{
		flow_signal(&m_obj, param);
	}

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

CFlowObject::TimerMap CFlowObject::s_timer_map;

void CFlowObject::check_timer()
{
	DWORD cur_tick = get_cur_tick();
	TimerMap::iterator it = s_timer_map.begin(), next_it;
    while (it != s_timer_map.end())
	{
		next_it = it;
		next_it++;
		if (it->first > cur_tick)
			break;
		if (it->second)
			it->second->signal();
		s_timer_map.erase(it);
		it = next_it;
	}
}

void CFlowObject::add_timer_to_map(DWORD ms, CFlowObject* pObj)
{
	//printf("add timer, ms=%lu, obj=%lx\n", ms, pObj);
	s_timer_map.insert(std::pair<DWORD, CFlowObject*>(ms, pObj));
}

bool CFlowObject::remove_timer_from_map(DWORD ms, CFlowObject* pObj)
{
    std::pair<TimerMap::iterator, TimerMap::iterator> range = s_timer_map.equal_range(ms);
    for (TimerMap::iterator it = range.first; it != range.second; ++it)
	{
		if (it->second == pObj)
		{
			//printf("remove timer, ms=%lu, obj=%lx\n", ms, pObj);
			it->second = NULL;
			return true;
		}
	}

	//printf("remove timer, not found\n");
	return false;
}

class CFlowSocket
{
public:
	CFlowSocket()
	{
		m_s = -1;
	}

	virtual ~CFlowSocket()
	{
		printf("CFlowSocket::~CFlowSocket\n");
		close();
	}

	int create()
	{
		if (m_s >= 0)
			return -1;

		m_s = VSOCK::socket();
		s_socket_map[m_s] = this;
		return m_s;
	}

	flow int connect(const char* url, WORD port)
	{
		int ret = VSOCK::connect(m_s, VSOCK::resolve_address(url, port));
		if (ret < 0)
			return ret;

		m_write_obj.wait();
		return 0;
	}

	flow void send(const char* buf, int len)
	{
		int t = 0;
		while (t < len)
		{
			t += VSOCK::send(m_s, buf + t, len - t);
			if (t == len)
				break;
			m_write_obj.wait();
		}
	}

	flow int recv(char* buf, int len)
	{
		int ret = 0;
		while (true)
		{
			ret = VSOCK::recv(m_s, buf, len);
			if (ret > 0)
				break;
			m_read_obj.wait();
		}

		return ret;
	}

	void close()
	{
		if (m_s < 0)
			return;
		printf("CFlowSocket::close\n");
		VSOCK::close(m_s);
		s_socket_map.erase(m_s);
		m_s = -1;
	}

	CFlowObject* getExceptFobj() { return &m_except_obj; }

	static void event_loop();

	static void stop_event_loop()
	{
		s_loop_running = false;
	}

protected:
	static std::map<int, CFlowSocket*>	s_socket_map;
	static bool		s_loop_running;

	int			m_s;
	CFlowObject	m_read_obj, m_write_obj, m_except_obj;
};

std::map<int, CFlowSocket*> CFlowSocket::s_socket_map;
bool CFlowSocket::s_loop_running = true;

void CFlowSocket::event_loop()
{
	while (s_loop_running)
	{
		if (!VSOCK::select_event(100))
		{
			CFlowObject::check_timer();
			continue;
		}
		std::map<int, CFlowSocket*>::iterator it, next_it;
		for (it = s_socket_map.begin(); it != s_socket_map.end(); it = next_it)
		{
			next_it = it;
			next_it++;
			bool bRead, bWrite, bExcept;
			VSOCK::check_event(it->first, bRead, bWrite, bExcept);

			CFlowSocket* pSocket = it->second;
			if (bExcept)
				pSocket->m_except_obj.signal();
			else if (bWrite)
				pSocket->m_write_obj.signal();
			else if (bRead)
				pSocket->m_read_obj.signal();
		}
	}
}

flow void start_timer(int interval, FLOW_OBJECT* pObj)
{
	flow_fork
	{
	    CFlowObject fo;
        fo.time_wait(interval);
		flow_signal(pObj, NULL);
	}
}

flow void heartbeat(int col, int max, int interval)
{
    CFlowObject fo;
    for (int i = 0; i < max; i += interval)
    {
		for (int j = 0; j < col; j++)
			printf("\t");
        printf("%d\n", i);
        fo.time_wait(interval * 1000);
    }
}

flow_root int main(int argc, char** argv)
{
	VSOCK::startup();

	flow_fork
	{
		heartbeat(0, 10, 2);
	}
	flow_fork
	{
		heartbeat(1, 10, 1);
		CFlowSocket::stop_event_loop();
	}

    CFlowSocket::event_loop();
    return 0;
}
