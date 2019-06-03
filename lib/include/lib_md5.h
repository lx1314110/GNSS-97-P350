#ifndef	__LIB_MD5__
#define	__LIB_MD5__


#include "lib_type.h"


/* MD5 context */
typedef struct  
{
        u32_t state[4];		/* state (ABCD)                           		*/
        u32_t count[2];    	/* number of bits, modulo 2^64 (lsb first)		*/
        u8_t  buffer[64];  	/* input buffer                            		*/
 } md5_ctx;


// Constants for Transform routine.
#define S11    7
#define S12   12
#define S13   17
#define S14   22
#define S21    5
#define S22    9
#define S23   14
#define S24   20
#define S31    4
#define S32   11
#define S33   16
#define S34   23
#define S41    6
#define S42   10
#define S43   15
#define S44   21


void md5_transform(u32_t state[4], u8_t block[64]);


void md5_init(md5_ctx *context);
void md5_update(md5_ctx *context, u8_t *buffer, u32_t length);
void md5_final(u8_t result[16], md5_ctx *context);


#endif//__LIB_MD5__


