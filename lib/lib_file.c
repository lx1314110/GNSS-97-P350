#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib_file.h"






/*
  1		路径存在
  0		路径不存在
*/
bool_t IsPathExist(char *path)
{
	if( 0 == access(path, F_OK) )
	{
		//exist
		return(true);
	}
	else
	{
		//not exist
		return(false);
	}
}





/*
  (< 0)		失败
  (>=0)		占用空间
*/
int GetFileSize(char *path)
{
	struct stat buf;

	if( IsPathExist(path) )
	{
		//exist
		memset(&buf, 0, sizeof(struct stat));
		if( 0 == stat(path, &buf) )
		{
			return(buf.st_size);
		}
		else
		{
			return(-1);
		}
	}
	else
	{
		//not exist
		return(-2);
	}
}







/*
  (<=0)		失败
  (  1)		成功
*/
int DeleteFile(char *path)
{
	if( IsPathExist(path) )
	{
		//exist
		if( -1 == unlink(path) )
		{
			//delete failure
			return(0);
		}
		else
		{
			//delete success
			if( IsPathExist(path) )
			{
				//exist after delete
				return(0);
			}
			else
			{
				//not exist after delete
				return(1);
			}
		}
	}
	else
	{
		//not exist
		return(-1);
	}
}






/*
  (<0)	失败
  (=0)	成功
  遍历目录，获取占用空间
*/
int traversal(char *path, int *size)
{
	DIR *dir;
	struct dirent *entry;
	struct stat buf;
	char next_path[512];
	
	if( IsPathExist(path) )
	{
		if( 0 == stat(path, &buf) )
		{
			if( !S_ISDIR(buf.st_mode) )
			{
				return(-3);
			}
			else
			{
				dir = opendir(path);
				if( NULL == dir )
				{
					return(-4);
				}
				else
				{
					errno = 0;
					while( (entry = readdir(dir)) != NULL )
					{
						if( (0 == strcmp(entry->d_name, "."))||
							(0 == strcmp(entry->d_name, "..")) )
						{
							continue;
						}
						else
						{
							if(entry->d_type == DT_DIR)
							{
								sprintf(next_path, "%s/%s", path, entry->d_name);
								traversal(next_path, size);
							}
							else
							{
								sprintf(next_path, "%s/%s", path, entry->d_name);
								if( 0 == stat(next_path, &buf) )
								{
									*size += buf.st_size;
								}
								else
								{
									return(-6);
								}
							}
						}
						errno = 0;
					}
					if( 0 == errno )
					{
						return(0);
					}
					else
					{
						return(-5);
					}
				}
			}
		}
		else
		{
			return(-2);
		}
	}
	else
	{
		return(-1);
	}
}






/*
  ( -1)		失败
  (>=0)		占用空间
*/
int GetDirectorySize(char *path)
{
	int dir_size = 0;
	
	if(0 == traversal(path, &dir_size))
	{
		return(dir_size);
	}
	else
	{
		return(-1);
	}
}






/*
  1		成功
  0		失败
*/
bool_t MoveFile(char *old_path, char *new_path)
{
	if( 0 == rename(old_path, new_path) )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}





