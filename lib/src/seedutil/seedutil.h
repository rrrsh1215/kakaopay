#ifndef _seedutil_h_
#define _seedutil_h_

#include "common.h"

#define KEYLEN          16

#define ENCRYPT_MSG     1
#define DECRYPT_MSG     2

int sh_encrypt(unsigned char *decbuf, const int declen, unsigned char *encbuf, int *enclen, card_pay_info_t *pCpi);
int sh_decrypt(unsigned char *encbuf, const int enclen, unsigned char *decbuf, int *declen, card_pay_info_t *pCpi);
int sh_getSeedKey(unsigned char *seedkey);
void synKey(unsigned char *originkey, unsigned char *addition, unsigned char *CMK);
int getCMK(unsigned char *seedkey, unsigned char *term_id, unsigned char *CMK);
int getSSK(unsigned char *CMK, unsigned char *randnum, unsigned char *SSK);
int paddingproc(unsigned char *orgbuf, int orglen, unsigned char *outcomebuf, int *outcomelen, int type);

#endif

