#ifndef __CSYS_PH_UPLOAD__
#define __CSYS_PH_UPLOAD__








#include "lib_type.h"

int ph_upload_argv_parser(int argc, char *argv[]);

#if 0
void ph_upload_helper(void);
int ph_upload_page_validity(char *, int);
int ph_upload_pages(sqlite3 *pDb, char *pTbl);
int ph_upload_nums(sqlite3 *pDb, char *pTbl);
int current_ph_upload_query(sqlite3 * pDb, char * pTbl, int page);
int history_ph_upload_query(sqlite3 * pDb, char * pTbl, int page);
#endif
#endif//__CSYS_PH_UPLOAD__



