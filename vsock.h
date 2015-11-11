#ifndef _VSOCK_H_
#define _VSOCK_H_

//#ifdef _WIN32
//	#ifdef VSOCK_EXPORTS
//		#define VSOCK_API __declspec(dllexport)
//	#else
//		#define VSOCK_API __declspec(dllimport)
//	#endif
//#else
	#define VSOCK_API
//#endif

typedef unsigned long	DWORD;
typedef unsigned short	WORD;
typedef void* VSOCK_ADDR;
typedef	int (*vsock_callback_t)(void* user_data);

namespace VSOCK
{
	int VSOCK_API startup();
	int VSOCK_API shutdown();

	int VSOCK_API socket();
	int VSOCK_API bind(int s, VSOCK_ADDR addr);
	int VSOCK_API connect(int s, VSOCK_ADDR addr);
	int VSOCK_API send(int s, const char* buf, int len, int flags = 0);
	int VSOCK_API recv(int s, char* buf, int len, int flags = 0);
	int VSOCK_API close(int s);

	bool select_event(DWORD internal_ms); // return true when any event found
	void check_event(int s, bool& bRead, bool& bWrite, bool& bExcept);

	DWORD VSOCK_API get_local_ip();
	DWORD VSOCK_API get_local_ip(DWORD dwServer);

	VSOCK_ADDR VSOCK_API resolve_address(const char* lpszSocketAddress, WORD nSocketPort);
	VSOCK_ADDR VSOCK_API build_address(DWORD addr, WORD nSocketPort);
	DWORD VSOCK_API get_address_ip(VSOCK_ADDR lpAddress);
	WORD VSOCK_API get_address_port(VSOCK_ADDR lpAddress);
	VSOCK_ADDR VSOCK_API copy_address(VSOCK_ADDR lpAddress);
	void VSOCK_API delete_address(VSOCK_ADDR lpAddress);
	int VSOCK_API compare_address(VSOCK_ADDR lpAddress, VSOCK_ADDR lpAddress2);
};

DWORD get_cur_tick();

#endif
