#ifndef	__LIB_MSGQ__
#define	__LIB_MSGQ__



#include "lib_type.h"




#define MAX_MSGS		100		/* maximum number of messages in queue */  
#define	MAX_BYTES		512		/* maximum number of bytes on messages */



struct msgbuf {
	long mtype;
	char mdata[MAX_BYTES];
};


int MsgQ_Detect( int qid, long type );
int MsgQ_Msgs( int qid );

int MsgQ_Init( char *pathname, int proj_id );
bool_t MsgQ_Send( int qid, struct msgbuf *msg, int dlen );
int MsgQ_Rcv( int qid, struct msgbuf *msg );
bool_t MsgQ_Remove( int qid );


#endif//__LIB_MSGQ__


