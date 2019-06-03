#ifndef	__DWG_PARSER__
#define	__DWG_PARSER__






#include "dwg_context.h"

#define UART0   0
#define UART1   1
//#define BIT(nr)                 (1UL << (nr))
#define UART_RXFIFO_FULL_INT_ENA            (BIT(0))
#define UART_RXFIFO_TOUT_INT_ENA            (BIT(8))
#define ETS_UNCACHED_ADDR(addr) (addr)
#define ETS_CACHED_ADDR(addr) (addr)
#define READ_PERI_REG(addr) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr)))
#define WRITE_PERI_REG(addr, val) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr))) = (uint32_t)(val)
#define SET_PERI_REG_MASK(reg, mask)   WRITE_PERI_REG((reg), (READ_PERI_REG(reg)|(mask)))
#define REG_UART_BASE(i)                (0x60000000 + (i)*0xf00)
#define UART_INT_ENA(i)                 (REG_UART_BASE(i) + 0xC)


bool_t isRunning(struct wgCtx *ctx);

bool_t isNotify(struct wgCtx *ctx);

int sendmessage(char *data, struct wgCtx *ctx,char *MessageType);

void Init_buf();
void Clean_buf();
int CreateThread(struct wgCtx *ctx);

int CloseThread(struct wgCtx *ctx);

int ReadWgIp(struct wgCtx *ctx, char *pTbl ,char *ip);

int ReadMessageQueue(struct wgCtx *ctx);

int ReadAlmState(struct wgCtx *ctx);

int ReadClockState(struct wgCtx *ctx);

int ReadTimeSource(struct wgCtx *ctx);

void ReNew_value(struct wgCtx *ctx);



#endif