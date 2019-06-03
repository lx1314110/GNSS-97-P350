
#include <sys/select.h>
#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "d61850_protocol.h"
#include "alloc.h"
#include "lib_dbg.h"

#define MAX_ERR_NO			(3)
#define MAX_FRAME_LENGTH 	(259)
#define MAX_DATE_LENGTH 	(253)

#define FRAME_END			(0x3)
#define FRAME_ACK			(0x6)
#define FRAME_EAK			(0x15)

#define D61850_PROTOCOL_DEBUG			1
#define D61850_NOACK_CONFIG 	0
#define D61850_RECV_DEBUG		0
ssize_t
tread(int fd, void *buf, size_t nbytes, unsigned int timout);
int iec61850_recv_frame(int fd);

unsigned char  iec61850_date_crc(const char * data, int length)
{
	int i;

	unsigned char crc = 0;
	for (i = 0; i < length; i++){
		crc ^= *data;
		data++; 	
	}
	return crc;
}
/**
 * @brief [brief description]
 * @details [long description]
 * 
 * @param fd [description]
 * @param str [description]
 * 
 * @return -1 帧过长 -2发送失败 0 发送成功 
 */

int
iec61850_send(int fd ,const char * str)
{
		unsigned char  buf[MAX_FRAME_LENGTH];
		unsigned char *pos;
		int rt;
		unsigned int length;
		int frame_len;
		char err;			/*发送错误记数*/

		pos = buf;

		memset(buf,0,sizeof(buf));
		/*clac '/0'*/
		length = strlen(str) + 1;
		if (length > MAX_DATE_LENGTH){
			return -1;
		}

		
		/*length*/

		*pos++ = (length >> 8)&0xff;
		*pos++ = length &0xff;
		
		memcpy(pos,str,length);
		pos += length;

		*pos++ = iec61850_date_crc(str,length);
		*pos++ = 0x03;

		frame_len = pos - buf;

		/*start send*/

		err = 0;
		//printf("%s\n",str);
#if D61850_NOACK_CONFIG

		rt = write(fd,buf,frame_len);
		if (rt != frame_len){
			print(DBG_ERROR,"d61850 send err \n");
			return -2;
		}
#else
	while(1)
			{
				rt = write(fd,buf,frame_len);
				if (rt != frame_len){
					print(DBG_ERROR,"d61850 send err \n");
					return -2;
				}

				rt = iec61850_recv_frame(fd);
				//printf("recv:%x\n",rt);
				if (FRAME_ACK == rt){
					return 0;
				}else{
						err++;
						if (err >= MAX_ERR_NO){
							return -2;
						}
				}

			}
#endif
			return 0;
}


/**
 * @brief iec61850 收帧
 * @details [long description]
 * 
 * @param fd [description]
 * @return  ack:FRAME_ACK eak:FRAME_EAK  crc err:3  timeout:4  undefine frame : 5
 */
int iec61850_recv_frame(int fd)
{
	unsigned char  buf[MAX_FRAME_LENGTH];
	unsigned char *pos;
	unsigned short length;
	int rt;
	char ch;

	pos = buf;
	memset(buf,0,sizeof(buf));

	#if D61850_RECV_DEBUG
		printf("\n*********Recv**********\n");
	#endif
	do
	{
		rt = tread(fd,&ch,1,5);
		#if D61850_RECV_DEBUG
			printf("%x",ch);
		#endif
		if (-1 == rt){
			print(	DBG_ERROR, 
					"--timout err.");
			return 4;
		}
		*pos++ = ch;

		if ((pos - buf)>= MAX_FRAME_LENGTH)
		{
			return 3;
		}

	}while(FRAME_END != ch);
	#if D61850_RECV_DEBUG
		printf("\n***********************\n");
	#endif

	/*handle*/
	length = (buf[0] << 8) | buf[1];
	
	/*crc*/
	if ((length != 4) | (buf[2] != buf[3])){
		print(	DBG_ERROR, 
					"--RECV CRC is err.");
			return 3;
	}

	if (FRAME_ACK == buf[2]){
		return FRAME_ACK;

	}else if (FRAME_EAK == buf[2]){
			print(	DBG_WARNING, 
					"--RECV FRAME_EAK.");
			return FRAME_EAK;
	}
	else{
			return 5;
	}
}

/*
 * "Timed" read - timout specifies the # of seconds to wait before
 * giving up (5th argument to select controls how long to wait for
 * data to be readable).  Returns # of bytes read or -1 on error.
 *
 * LOCKING: none.
 */
ssize_t
tread(int fd, void *buf, size_t nbytes, unsigned int timout)
{
	int				nfds;
	fd_set			readfds;
	struct timeval	tv;

	tv.tv_sec = timout;
	tv.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	nfds = select(fd+1, &readfds, NULL, NULL, &tv);
	if (nfds <= 0) {
		if (nfds == 0)
			errno = ETIME;
		return(-1);
	}
	return(read(fd, buf, nbytes));
}

/*
 * "Timed" read - timout specifies the number of seconds to wait
 * per read call before giving up, but read exactly nbytes bytes.
 * Returns number of bytes read or -1 on error.
 *
 * LOCKING: none.
 */
ssize_t
treadn(int fd, void *buf, size_t nbytes, unsigned int timout)
{
	size_t	nleft;
	ssize_t	nread;

	nleft = nbytes;
	while (nleft > 0) {
		if ((nread = tread(fd, buf, nleft, timout)) < 0) {
			if (nleft == nbytes)
				return(-1); /* error, return -1 */
			else
				break;      /* error, return amount read so far */
		} else if (nread == 0) {
			break;          /* EOF */
		}
		nleft -= nread;
		buf += nread;
	}
	return(nbytes - nleft);      /* return >= 0 */
}
