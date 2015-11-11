
#include "vsock.h"
#include <WinSock2.h>
#include <iphlpapi.h>

fd_set g_readfds, g_writefds, g_exceptfds;
fd_set g_temp_readfds, g_temp_writefds, g_temp_exceptfds;
int g_nfds = 0;
DWORD g_last_tick;
struct sockaddr_in g_vsock_temp_sockAddr;

void add_socket_to_fdset(SOCKET s)
{
	FD_SET(s, &g_readfds);

	if (g_nfds <= s)
		g_nfds = s + 1;
}

void remove_socket_from_fdset(SOCKET s)
{
	FD_CLR(s, &g_readfds);
	FD_CLR(s, &g_writefds);
	FD_CLR(s, &g_exceptfds);
}

void add_socket_to_writefds(SOCKET s)
{
	FD_SET(s, &g_writefds);
}

void remove_socket_from_writefds(SOCKET s)
{
	FD_CLR(s, &g_writefds);
}

void add_socket_to_exceptfds(SOCKET s)
{
	FD_SET(s, &g_exceptfds);
}

void remove_socket_from_exceptfds(SOCKET s)
{
	FD_CLR(s, &g_exceptfds);
}

namespace VSOCK
{
int VSOCK_API startup()
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1);
	int nResult = WSAStartup(wVersionRequested, &wsaData);
	if (nResult != 0)
		return nResult;

	FD_ZERO(&g_readfds);
	FD_ZERO(&g_writefds);
	FD_ZERO(&g_exceptfds);
	g_last_tick = 0;

	int s = socket(); // create a anonymous socket to make the timer function in select() work
	add_socket_to_fdset(s);
	return 0;
}

int VSOCK_API shutdown()
{
	WSACleanup();

	return 0;
}

int VSOCK_API socket()
{
	int s = ::socket(AF_INET, SOCK_STREAM, 0);

	int optval = 1;
	::setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int));
	u_long ulValue = 1;
	int ret = ioctlsocket(s, FIONBIO, &ulValue);

	BOOL bReuseAddr = TRUE;
	::setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReuseAddr, sizeof(bReuseAddr));

	return s;
}

int VSOCK_API bind(int s, VSOCK_ADDR addr)
{
	return ::bind(s, (const struct sockaddr*)addr, sizeof(struct sockaddr_in));
}

int VSOCK_API connect(int s, VSOCK_ADDR addr)
{
	struct sockaddr_in* sai = (struct sockaddr_in*)addr;

	int ret = ::connect(s, (const struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if (ret != 0 && WSAGetLastError() != WSAEWOULDBLOCK)
		return ret;
	
	add_socket_to_fdset(s);
	add_socket_to_writefds(s);
	add_socket_to_exceptfds(s);
	return 0;
}

int VSOCK_API send(int s, const char* buf, int len, int flags)
{
	int ret = ::send(s, buf, len, flags);
	if (ret < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
		ret = 0;

	if (ret < len && ret >= 0)
		add_socket_to_writefds(s);

	return ret;
}

int VSOCK_API recv(int s, char* buf, int len, int flags)
{
	int ret = ::recv(s, buf, len, flags);
	if (ret < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
		ret = 0;
	return ret;
}

int VSOCK_API close(int s)
{
	remove_socket_from_fdset(s);
	::shutdown(s, SD_BOTH);
	closesocket(s);

	return 0;
}

bool VSOCK_API select_event(DWORD internal_ms)
{
	memcpy(&g_temp_readfds, &g_readfds, sizeof(fd_set));
	memcpy(&g_temp_writefds, &g_writefds, sizeof(fd_set));
	memcpy(&g_temp_exceptfds, &g_exceptfds, sizeof(fd_set));

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	DWORD cur_tick = GetTickCount();
	if (g_last_tick)
	{
		g_last_tick += internal_ms;
		if (cur_tick < g_last_tick)
		{
			internal_ms = g_last_tick - cur_tick;
			timeout.tv_sec = internal_ms / 1000;
			timeout.tv_usec = (internal_ms % 1000) * 1000;
		}
	}
	else
		g_last_tick = cur_tick;

	int ret = select(g_nfds, (g_temp_readfds.fd_count?&g_temp_readfds:NULL), (g_temp_writefds.fd_count?&g_temp_writefds:NULL), (g_temp_exceptfds.fd_count?&g_temp_exceptfds:NULL), &timeout);
	//int ret = select(0, NULL, NULL, NULL, &timeout);
	if (ret < 0)
	{
		int err = WSAGetLastError();
		return false;
	}

	if (ret == 0)	// timeout and no socket is ready
		return false;

	return true;
}

void VSOCK_API check_event(int s, bool& bRead, bool& bWrite, bool& bExcept)
{
	bRead = FD_ISSET(s, &g_temp_readfds);
	bWrite = FD_ISSET(s, &g_temp_writefds);
	bExcept = FD_ISSET(s, &g_temp_exceptfds);

	if (bRead || bExcept)
	{
		DWORD avail = 0;
		if (::ioctlsocket(s, FIONREAD, &avail) < 0 || avail == 0)
		{
			remove_socket_from_fdset(s);
			bExcept = true;
			return;
		}
	}
	if (bWrite)
	{
		remove_socket_from_writefds(s);
		remove_socket_from_exceptfds(s);
	}
}

DWORD GetIPFromIFIndex(DWORD dwIndex)
{
	DWORD dwSize = 0;
	/*g_pfn*/GetIpAddrTable(NULL, &dwSize, FALSE);
	MIB_IPADDRTABLE* addr_table = (MIB_IPADDRTABLE*)malloc(dwSize);
	/*g_pfn*/GetIpAddrTable(addr_table, &dwSize, FALSE);

	MIB_IPADDRROW* pRow = addr_table->table;
	DWORD dwIP = 0;
	for (unsigned int nIndex = 0; nIndex < addr_table->dwNumEntries; nIndex++, pRow++)
	{
		if (pRow->dwIndex == dwIndex)
		{
			dwIP = pRow->dwAddr;
			break;
		}
	}

	free(addr_table);
	return dwIP;
}

DWORD VSOCK_API get_local_ip(DWORD dwServer)
{
	if (!dwServer)
		return 0;

	DWORD dwIPFIndex;
	GetBestInterface(dwServer, &dwIPFIndex);

	return htonl(GetIPFromIFIndex(dwIPFIndex));
}

VSOCK_ADDR VSOCK_API resolve_address(const char* lpszSocketAddress, WORD nSocketPort)
{
	memset(&g_vsock_temp_sockAddr, 0, sizeof(g_vsock_temp_sockAddr));

	LPSTR lpszAscii = (LPSTR)lpszSocketAddress;
	g_vsock_temp_sockAddr.sin_family = AF_INET;
	g_vsock_temp_sockAddr.sin_addr.s_addr = (lpszAscii ? inet_addr(lpszAscii) : INADDR_ANY);

	if (g_vsock_temp_sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		try {
			lphost = gethostbyname(lpszAscii);
		} catch (...)
		{
			return NULL;
		}
		g_vsock_temp_sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
	}

	g_vsock_temp_sockAddr.sin_port = htons((u_short)nSocketPort);
	return (VSOCK_ADDR)&g_vsock_temp_sockAddr;
}

VSOCK_ADDR VSOCK_API build_address(DWORD addr, WORD nSocketPort)
{
	memset(&g_vsock_temp_sockAddr, 0, sizeof(g_vsock_temp_sockAddr));

	g_vsock_temp_sockAddr.sin_family = AF_INET;
	g_vsock_temp_sockAddr.sin_addr.s_addr = htonl(addr);
	g_vsock_temp_sockAddr.sin_port = htons((u_short)nSocketPort);
	return (VSOCK_ADDR)&g_vsock_temp_sockAddr;
}

DWORD get_address_ip(VSOCK_ADDR lpAddress)
{
	if (!lpAddress)
		return 0;

	struct sockaddr_in* sockAddr = (struct sockaddr_in*)lpAddress;
	return ntohl(sockAddr->sin_addr.S_un.S_addr);
}

WORD VSOCK_API get_address_port(VSOCK_ADDR lpAddress)
{
	if (!lpAddress)
		return 0;

	struct sockaddr_in* sockAddr = (struct sockaddr_in*)lpAddress;
	return ntohs(sockAddr->sin_port);
}

VSOCK_ADDR VSOCK_API copy_address(VSOCK_ADDR lpAddress)
{
	void* lpAddress2 = malloc(sizeof(struct sockaddr_in));
	if (!lpAddress2 || !lpAddress)
		return NULL;

	memcpy(lpAddress2, lpAddress, sizeof(struct sockaddr_in));
	return lpAddress2;
}

void VSOCK_API delete_address(VSOCK_ADDR lpAddress)
{
	free(lpAddress);
}

int VSOCK_API compare_address(VSOCK_ADDR lpAddress, VSOCK_ADDR lpAddress2)
{
	return memcmp(lpAddress, lpAddress2, sizeof(struct sockaddr_in));
}

}

DWORD get_cur_tick()
{
	return GetTickCount();
}
