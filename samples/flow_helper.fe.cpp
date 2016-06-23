#include "flow_helper.h"

CFlowObject::CFlowObject()
{
	flow_init(&m_obj);
	m_waiting_time = (DWORD)-1;
}

CFlowObject::~CFlowObject()
{
	cancel_wait();
}

flow void CFlowObject::wait(void** param)
{
	flow_wait(&m_obj, param);
}

flow void CFlowObject::time_wait(DWORD interval)
{
	if (m_waiting_time != (DWORD)-1)
		remove_timer_from_map(m_waiting_time, this);

	m_waiting_time = get_cur_tick() + interval;
	add_timer_to_map(m_waiting_time, this);
	flow_wait(&m_obj, NULL);
}

void CFlowObject::cancel_wait()
{
	if (m_waiting_time != (DWORD)-1)
		remove_timer_from_map(m_waiting_time, this);

	flow_cancel_wait(&m_obj);
}

void CFlowObject::signal(void* param)
{
	flow_signal(&m_obj, param);
}

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

CFlowSocket::CFlowSocket()
{
	m_s = -1;
}

CFlowSocket::~CFlowSocket()
{
	printf("CFlowSocket::~CFlowSocket\n");
	close();
}

int CFlowSocket::create()
{
	if (m_s >= 0)
		return -1;

	m_s = VSOCK::socket();
	s_socket_map[m_s] = this;
	return m_s;
}

flow int CFlowSocket::connect(const char* url, WORD port)
{
	int ret = VSOCK::connect(m_s, VSOCK::resolve_address(url, port));
	if (ret < 0)
		return ret;

	m_write_obj.wait();
	return 0;
}

flow void CFlowSocket::send(const char* buf, int len)
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

flow int CFlowSocket::recv(char* buf, int len)
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

void CFlowSocket::close()
{
	if (m_s < 0)
		return;
	printf("CFlowSocket::close\n");
	VSOCK::close(m_s);
	s_socket_map.erase(m_s);
	m_s = -1;
}

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

void CFlowSocket::stop_event_loop()
{
	s_loop_running = false;
}

