#include "dntp_server.h"
#include "dntp_global.h"



int main(int argc, char *argv[])
{
	return(ProcNtp(&gNtpCtx));
}


