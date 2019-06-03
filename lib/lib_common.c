#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "lib_common.h"




/*
  1	成功
  0	失败
*/
int notify(int pid, int sig)
{
	if(-1 == kill(pid, sig) )
	{
		perror("kill");
		return 0;
	}
	else
	{
		return 1;
	}
}




/*
  -1	Failed to find PID
   0	No such process
  >0	PID
*/
int find_pid_by_name( char *procName)
{
	DIR *dir;
	struct dirent *entry;
	FILE *fp;
	char *name;
	int tmp_pid,ret_pid = 0;
	char status[32];
	char buf[128];
	char *p1,*p2;

	dir = opendir("/proc");
	if(NULL == dir)
	{
		return -1;
	}

	for(;;)
	{
		errno = 0;
		if((entry = readdir(dir)) == NULL)
		{
			closedir(dir);
			dir = NULL;
			if(0 == errno)
			{
				return ret_pid;
			}
			else
			{
				return -1;
			}
		}

		name = entry->d_name;
		if(!(*name >= '0' && *name <= '9'))
			continue;

		tmp_pid = atoi(name);
		
		memset(status, 0, sizeof(status));
		sprintf(status, "/proc/%d/stat", tmp_pid);
		if((fp = fopen(status, "r")) == NULL)
		{
			closedir(dir);
			dir = NULL;
			return -1;
		}

		name = fgets(buf, sizeof(buf), fp);
		fclose(fp);
		if(NULL == name)
		{
			closedir(dir);
			dir = NULL;
			return -1;
		}

		p1 = strchr(buf, '(');
		p2 = strrchr(buf, ')');
		if(NULL == p1 || NULL == p2 || ' ' != p2[1])
		{
			closedir(dir);
			dir = NULL;
			return -1;
		}

		if(0 == memcmp(p1+1, procName, p2-p1-1))
		{
			if(0 == ret_pid)
			{
				ret_pid = tmp_pid;
			}
			else
			{
				if(tmp_pid < ret_pid)
				{
					ret_pid = tmp_pid;
				}
			}
		}
	}
}





/*
  字符串逆序
*/
void reverse(char *s)
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





/*
  整数转16进制字符串
*/
void itohexa(int n, char *s)
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






/* 字节异或 */
u8_t data_xor(u8_t *data, int len)
{
	int i;
	u8_t chksum = 0;

	for( i=0; i<len; i++ )
	{
		chksum = chksum ^ data[i];
	}

	return chksum;
}




int strlen_r( char *str , char end)
{
	int len = 0;

	while( (*str++) != end )
	{
		len++;
	}
	return len;
}




unsigned char bcd2decimal(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}




unsigned char decimal2bcd(unsigned char val)
{
	return ((val / 10) << 4) + val % 10;
}





