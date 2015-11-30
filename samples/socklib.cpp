#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>	// include TCP_NODELAY definition
#include <netdb.h>
#include <asm/errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "vsock.h"

#define ASOCK_STATE_IDLE		0
#define ASOCK_STATE_CREATED		1
#define ASOCK_STATE_LISTENING	2
#define ASOCK_STATE_CONNECTING	3
#define ASOCK_STATE_CONNECTED	4

#define ASOCK_FLAG_SOCK_TYPE	1
#define ASOCK_FLAG_TCPIP		0
#define ASOCK_FLAG_UDP			1

#define SOCK_TABLE_SIZE	64000

int g_sock_init = 0;
struct sockaddr_in g_sock_temp_sockAddr;
int g_epfd = -1;
struct epoll_event* g_events = NULL;
struct epoll_event* g_events2 = NULL;
int	g_epfd_thread;
u_short g_sock_count;

int stn_sock_setnonblock(int s)
{
	int optval;

	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

	if (true)
	{
		optval = 1;
		if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int)))
		{
			return -1;
		}
	}

	int reuse_addr = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int));

	int sndbuf_size = 64 * 1024;
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, sizeof(int));
	int rcvbuf_size = 64 * 1024;
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, sizeof(int));

	return 0;
}

int stn_sock_add_to_event(int epfd, int s)
{
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET;
	ev.data.fd = s;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, s, &ev) < 0)
	{
		int err = errno;
		return -1;
	}

	return 0;
}

int stn_sock_remove_from_event(int epfd, int s)
{
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, s, NULL) < 0)
	{
		int err = errno;
		return -1;
	}

	return 0;
}

DWORD get_cur_tick()
{
	uint64_t sec, msec;
	struct timeval tval;
	gettimeofday(&tval, NULL);
	sec = tval.tv_sec;
	msec = tval.tv_usec / 1000;

	return sec * 1000 + msec;
}

namespace VSOCK
{
// public functions
int startup()
{
	if (g_sock_init)
		return 0;

	g_epfd_thread = epoll_create(1024);
	g_sock_count = 0;
	g_sock_init = 1;

	return 0;
}

int shutdown()
{
	int i;

	if (!g_sock_init)
		return 0;

	g_sock_init = 0;
	return 0;
}

int socket()
{
	int s;

	s = ::socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return 0;

	if (stn_sock_setnonblock(s))
	{
		::close(s);
		return 0;
	}

	return s;
}

int bind(int s, VSOCK_ADDR addr)
{
	int ret;

	if (::bind(s, (sockaddr*)addr, sizeof(sockaddr)) < 0)
		return -2;

	return 0;
}

int connect(int s, VSOCK_ADDR addr)
{
	if (::connect(s, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) < 0 
		&& !(errno == EWOULDBLOCK || errno == EINPROGRESS || errno == EALREADY))
	{
		return 1;
	}

	if (stn_sock_add_to_event(g_epfd_thread, s) < 0)
	{
		close(s);
		return 0;
	}

	return 0;
}

int close(int s)
{
	stn_sock_remove_from_event(g_epfd_thread, s);
	::close(s);
	return 0;
}

int send(int s, const char* buf, int len, int flags)
{
	int ret = ::send(s, buf, len, MSG_DONTWAIT | MSG_NOSIGNAL);
	if (ret < 0)
	{
		if (errno == EWOULDBLOCK)
			ret = 0;
	}

	return ret;
}

int recv(int s, char* buf, int len, int flags)
{
	int ret;

	ret = ::recv(s, buf, len, 0);
	return ret;
}

bool select_event(DWORD internal_ms)
{
	if (g_epfd == -1)
	{
		g_epfd = g_epfd_thread;
		g_events = (struct epoll_event*)malloc(SOCK_TABLE_SIZE * sizeof(struct epoll_event));
		g_events2 = (struct epoll_event*)malloc(SOCK_TABLE_SIZE * sizeof(struct epoll_event));
	}

	memset(g_events2, 0, SOCK_TABLE_SIZE * sizeof(struct epoll_event));
	int nfds = epoll_wait(g_epfd, g_events, SOCK_TABLE_SIZE, internal_ms);
	struct epoll_event *ep = g_events;
	for (int n = 0; n < nfds; ++n, ep++)
	{
		g_events2[ep->data.fd].events = ep->events;
	}

	return nfds != 0;
}

void check_event(int s, bool& bRead, bool& bWrite, bool& bExcept)
{
	struct epoll_event *ep = g_events2 + s;

	bRead = ep->events & EPOLLIN;
	bWrite = ep->events & EPOLLOUT;
	bExcept = ep->events & EPOLLERR;
}

DWORD get_local_ip()
{
	char achBuf[256];
	achBuf[0] = 0;

	if (gethostname(achBuf, sizeof(achBuf)) == 0)
	{
		struct hostent* pHost = gethostbyname(achBuf);

		if (pHost)
		{
			in_addr in;
			memcpy(&in.s_addr, pHost->h_addr, pHost->h_length);
			return ntohl(in.s_addr);
		}
	}
	return 0x7f000001;//"127.0.0.1";
}

DWORD get_local_ip(DWORD dwServer)
{
   return 0;
}

VSOCK_ADDR resolve_address(char* lpszSocketAddress, WORD nSocketPort)
{
	memset(&g_sock_temp_sockAddr, 0, sizeof(g_sock_temp_sockAddr));

	g_sock_temp_sockAddr.sin_family = AF_INET;
	g_sock_temp_sockAddr.sin_addr.s_addr = (lpszSocketAddress ? inet_addr(lpszSocketAddress) : INADDR_ANY);
	if (g_sock_temp_sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		struct hostent* lphost;
		lphost = gethostbyname(lpszSocketAddress);
		if (lphost != NULL)
			g_sock_temp_sockAddr.sin_addr.s_addr = ((struct in_addr*)lphost->h_addr)->s_addr;
		else
			return NULL;
	}
	g_sock_temp_sockAddr.sin_port = htons((u_short)nSocketPort);
	return (VSOCK_ADDR)&g_sock_temp_sockAddr;
}

VSOCK_ADDR build_address(DWORD addr, WORD nSocketPort)
{
	memset(&g_sock_temp_sockAddr, 0, sizeof(g_sock_temp_sockAddr));

	g_sock_temp_sockAddr.sin_family = AF_INET;
	g_sock_temp_sockAddr.sin_addr.s_addr = htonl(addr);
	g_sock_temp_sockAddr.sin_port = htons((u_short)nSocketPort);
	return (VSOCK_ADDR)&g_sock_temp_sockAddr;
}

DWORD get_address_ip(VSOCK_ADDR lpAddress)
{
	if (!lpAddress)
		return 0;
	
	struct sockaddr_in* sockAddr = (struct sockaddr_in*)lpAddress;
	return ntohl(sockAddr->sin_addr.s_addr);
}

WORD get_address_port(VSOCK_ADDR lpAddress)
{
	if (!lpAddress)
		return 0;
	
	struct sockaddr_in* sockAddr = (struct sockaddr_in*)lpAddress;
	return ntohs(sockAddr->sin_port);
}

VSOCK_ADDR copy_address(VSOCK_ADDR lpAddress)
{
	void* lpAddress2 = malloc(sizeof(struct sockaddr_in));
	memcpy(lpAddress2, lpAddress, sizeof(struct sockaddr_in));
	return lpAddress2;
}

void delete_address(VSOCK_ADDR lpAddress)
{
	free(lpAddress);
}

int compare_address(VSOCK_ADDR lpAddress, VSOCK_ADDR lpAddress2)
{
	return memcmp(lpAddress, lpAddress2, sizeof(struct sockaddr_in));
}

}
