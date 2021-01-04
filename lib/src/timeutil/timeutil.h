#include "common.h"

int sh_getdatetime(char *datetime, const int round);

/*
 * Y: 동시결제 X
 * N: 동시결제 O
 */
char TimeGapAfterYN(char *bef, char *aft, const int gap);
