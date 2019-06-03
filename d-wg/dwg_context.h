#ifndef	__DWG_CONTEXT__
#define	__DWG_CONTEXT__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>

#include "alloc.h"
#include "addr.h"


struct UartBuffer{
    u32_t     UartBuffSize;
    u8_t    *pUartBuff;
    u8_t     *pInPos;
    u8_t     *pOutPos;
    u8_t  BuffState;
    u16_t    Space;  //remanent space of the buffer
    u8_t  TcpControl;
    struct  UartBuffer     *nextBuff;
};

struct wgCtx {
	bool_t loop_flag;
	
	bool_t notify_flag;
	
	int ne_changflag;
//database handle
	sqlite3 *pDb;
	//veth_info database
	sqlite3 *pVethDb;
	//wg socket
	int sendSock;
	//ipc
	struct ipcinfo ipc[IPC_CURSOR_ARRAY_SIZE];
	
	int fpga_fd;
	
	pthread_t thread_1,thread_2;
	
	int old_AlmState;
	int new_AlmState;
	
	int old_clkSta;
	int new_clkSta;
	
	char old_TimeSource[10];
	char new_TimeSource[10];
	
	char ip[16];
	
	int pid;
	int rec_pid;
	
	struct sockaddr_in sa_send;
	struct sockaddr_in sa_rec;
};


struct respond{
	
	char user[15];
	char password[8];
	char pid[6];
	char answer[15];
	
};


#endif