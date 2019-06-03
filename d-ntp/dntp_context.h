#ifndef	__DNTP_CONTEXT__
#define	__DNTP_CONTEXT__






#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "dntp_type.h"
#include "dntp_threadpool.h"
#include "alloc.h"
#include "dntp_socket.h"





//struct itimerval {
//	struct timeval it_interval; /* next value */
//	struct timeval it_value;    /* current value */
//};
//struct timeval {
//	long tv_sec;                /* seconds */
//	long tv_usec;               /* microseconds */
//};






struct ntpreq {
	struct timeval recvTime;
	struct recvaddr_t recvAddr;
	struct ntpkt recvPkt;
};







struct ntpCtx {
	//loop flag
	bool_t loop_flag;

	//notify flag
	bool_t notify_flag;

	//database handle
	sqlite3 *pDb;
	//veth_info database
	sqlite3 *pVethDb;

	//ntp socket
	int ntpSock;

	int fpga_fd;

    pthread_t thread_id;
	//ipc
	struct ipcinfo ipc;

	//thread pool
	struct threadpool pool;

	struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	struct ntpsta ntpSta;
	struct leapinfo lp;
};











#endif//__DNTP_CONTEXT__


