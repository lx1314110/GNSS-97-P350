#ifndef	__DNTP_ALLOC__
#define	__DNTP_ALLOC__


#include "dwg_context.h"

#define SERVER_IP "192.168.22.66"
#define	WG_SEND_PORT		5002
#define	WG_REC_PORT		    5001
#define USERNAME "DaTang"
#define PASSWORD "334455"
#define CRC_FAILURE "CRC_FAILURE"

#define BUFFER_SIZE 	1024*4 
#define UART_FIFO_LEN  	128 

/* probably should not put STATUS here */
/*
typedef enum {
    OK = 0,
    FAIL,
    PENDING,
    BUSY,
    CANCEL
}STATUS;
*/
int initializePriority(int prio);

int initializeNotifySignal(void);

int initializeFpga(struct wgCtx *ctx);

int cleanFpga(struct wgCtx *ctx);

int initializeDatabase(sqlite3 **pDb);
int initializeVethDatabase(sqlite3 **pDb);

void initializeContext(struct wgCtx *ctx);

int initializeExitSignal(void);

int initializeShareMemory(struct ipcinfo *ipc);

int initializeMessageQueue(struct ipcinfo *ipc);

int initializeSemaphoreSet(struct ipcinfo *ipc);

int writePid(struct ipcinfo *ipc);

bool_t send_ne_timer(struct wgCtx *ctx);

int ReadipFromTable(struct wgCtx *ctx, char *pTbl);

int write_virtual_eth(struct wgCtx *ctx, char *tbl_sys,struct veth *netp);

int cleanDatabase(sqlite3 *pDb);
int cleanVethDatabase(sqlite3 *pDb);

#endif