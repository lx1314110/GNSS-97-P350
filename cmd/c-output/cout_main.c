#include "cout_parser.h"
#include "cout_global.h"



int main(int argc, char *argv[])
{
	return(argv_parser(argc, argv, &gOutCtx));
}


