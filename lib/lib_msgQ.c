#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#include "lib_msgQ.h"





/*
  创建消息队列
   -1	失败
  >=0	成功
*/
int MsgQ_Init( char *pathname, int proj_id )
{
	key_t key;
 	int msgQ_id;
    
	/* Create unique key via call to ftok() */
	key = ftok( pathname, proj_id );
	if ( -1 == key )
	{
		return(-1);
	}

	/* Open the queue,create if necessary */
	msgQ_id = msgget( key, IPC_CREAT|0660 );
	if( -1 == msgQ_id )
	{
		return (-1);
	}
	return (msgQ_id);
}


/*
  判断指定类型的消息是否存在
   1	存在
   0	不存在
  -1	无法判断指定类型的消息是否存在
*/
int MsgQ_Detect( int qid, long type )
{
	if( -1 == msgrcv( qid, NULL, 0, type, IPC_NOWAIT) )
	{
		if( E2BIG == errno )
		{
			return(1);   
		}
		else
		{
			return(0);
		}
	}
	else
	{
		return(-1);
	}
}


/*
  获得当前消息队列中的消息数量
   -1	失败
  >=0	成功
*/
int MsgQ_Msgs( int qid )
{
	struct msqid_ds qds;
    
	/* Get current info */
	if ( 0 == msgctl( qid, IPC_STAT, &qds ) )
	{
		return(qds.msg_qnum);
	}
	return (-1);
}


/*
  发送消息
  0	失败
  1	成功
*/
bool_t MsgQ_Send( int qid, struct msgbuf *msg, int dlen )
{
	if ( MsgQ_Msgs( qid ) >= MAX_MSGS )
	{
		return (false);
	}
	
	if( -1 == msgsnd( qid, msg, dlen, IPC_NOWAIT) )
	{
		return (false);
	} 
	return (true);
}


/*
  接收消息
  -1	失败
   0	无消息
  >0	有消息
*/
int MsgQ_Rcv( int qid, struct msgbuf *msg )
{
	/* Read a msgbuf from the queue */
	int ret = -1;
	
	ret = msgrcv( qid, msg, sizeof(struct msgbuf)-sizeof(long), 0, IPC_NOWAIT );
	if ( -1 != ret )
	{
		return (ret);
	}
	else
	{
		if(ENOMSG == errno)
		{
			return (0);
		}
		else
		{
			return (-1);
		}
	}
}


/*
  删除消息队列
  0	失败
  1	成功
*/
bool_t MsgQ_Remove( int qid )
{
	if ( -1 == msgctl(qid, IPC_RMID, 0) )
	{
		return (false);
	}
	return (true);
}


