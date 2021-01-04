#ifndef _cfgutil_h_
#define _cfgutil_h_

#include "common.h"

int sh_get_mng_no(char *msg_no, char *home);
int getHomeDir(char *home);
int loadConfig(char *pathfile, char *field, void *value, const char type);
int getConfig(char *home, char *pathfile_db, int *timegap);

#endif

