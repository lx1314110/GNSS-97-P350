#ifndef __LIB_COMMON__
#define __LIB_COMMON__




#include "lib_type.h"





int notify(int pid, int sig);
int find_pid_by_name( char *procName);

void reverse(char *s);
void itohexa(int n, char *s);

u8_t data_xor(u8_t *data, int len);
int strlen_r( char *str , char end);

unsigned char bcd2decimal(unsigned char val);
unsigned char decimal2bcd(unsigned char val);






#endif//__LIB_COMMON__


