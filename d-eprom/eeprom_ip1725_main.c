#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "addr.h"
#include "lib_fpga.h"
#include "lib_ip1725.h"

//#define EEPROM_READ (0)
//#define EEPROM_WRITE (1)
char *progname = NULL;

#if 0
static void reverse(char *s)
{
    int c;
    int i,j;

    for(i=0,j=(strlen(s)-1); i<j; i++,j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
#endif

#if 0
static void itohexa(int n, char *s)
{
    char *t = s;
    int temp;
  
    do
    {
        temp = n%16;
        if( (temp >= 0) &&(temp <= 9) )
        {
            *s++ = temp + 0x30;
        }
        else
        {
            *s++ = temp + 0x37;
        }
    } 
    while((n/=16) >0);
  
    *s = '\0';
    reverse(t);
}
#endif

static unsigned char Char2Hex(const char C)
{
    unsigned char Data = 0;
    if (C >= '0' && C <= '9')
    {
        Data = C - '0';
    }
    else if (C >= 'A' && C <= 'F')
    {
        Data = C - 'A' + 10;
    }
    else if (C >= 'a' && C <= 'f')
    {
        Data = C - 'a' + 10;
    }
    return Data;
}

unsigned short StrToHex(unsigned char * str)
{
	unsigned char OneData = 0;
	unsigned short result_hex = 0;
	unsigned char * tstr = str;
	int i = 0;

	//*hex = 0;
	if(tstr == NULL)
		return -1;
	 if(tstr[0] == '0' && (tstr[1] == 'x' || tstr[1] == 'X'))
	 	tstr = &str[2];
	 //reverse(tstr);	 
	 for (i = 0; i < strlen((char *)tstr); i++)
	 {		  
		 OneData = Char2Hex(tstr[i]);
		 //result_hex += OneData*16^i; //<==>
		 result_hex += OneData;
		 if(i < strlen((char *)tstr) -1)
		 	result_hex <<= 4;
	 }
	//*hex = result_hex;
  return result_hex;
}


static void usage(char *argv)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "    %s read|write reg_addr write_value\n\n", argv);

    fprintf(stderr, "    %s read reg_addr[hex] \n",argv);
    fprintf(stderr, "    %s write reg_addr[hex] write_value[hex]\n",argv);

}
static void Close_FpgaDev(int fpga_fd)
{
	if(!FpgaClose(fpga_fd))
	{
		printf("Failed to close FPGA device.");
		return;
	}  
}
int main(int argc, char * argv [])
{
	unsigned char reg_addr = 0;
	unsigned short value = 0;
	int fpga_fd;
	int ret = -1;
		
	if (argc < 3)
	{
		usage(argv[0]);
		return -1;
	}

	//init
    fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == fpga_fd)
	{
		printf("Failed to open FPGA device.");
		return -1;
	}

	if (!strncmp("read", argv[1], 4))
	{
	    //status = EEPROM_READ;
		reg_addr = (unsigned char) StrToHex((u8_t *)argv[2]);
		SwitchRegRead(fpga_fd,reg_addr, &value);
		printf("[read] addr:0x%02x value:0x%04x\n", reg_addr, value);
		ret = 0;
	}
	else if(!strncmp("write", argv[1], 5))
	{
	    //status = EEPROM_WRITE;
    	if (argc < 4){
			usage(argv[0]);
			return -1;
		}
		reg_addr = StrToHex((u8_t *)argv[2]);
		value = StrToHex((u8_t *)argv[3]);
		if(reg_addr < 0)
			goto usag;

		SwitchRegWrite(fpga_fd,reg_addr, value);
		printf("[write] addr:0x%02x value:0x%04x\n", reg_addr, value);
		ret = 0;
			
	}else{
	            goto usag;
	}

	Close_FpgaDev(fpga_fd);

	return ret;
	usag:
		Close_FpgaDev(fpga_fd);
		usage(argv[0]);
		return -1;

}
