#ifndef	__LIB_FILE__
#define	__LIB_FILE__






#include "lib_type.h"






bool_t IsPathExist(char *path);
int GetFileSize(char *path);
int DeleteFile(char *path);
int GetDirectorySize(char *path);
bool_t MoveFile(char *old_path, char *new_path);











#endif//__LIB_FILE__


