#include "seedutil.h"

#include <openssl/seed.h>
#include <openssl/modes.h>

#define KEYLEN          16
#define MAX_BUF_SIZE    2048
#define SEEDKEY "/home/shryu/opensource/private/seedkey"

int makeSeedKeySchedule(unsigned char *mng_no_07, unsigned char *mng_no_13, SEED_KEY_SCHEDULE *sks);

int sh_encrypt(unsigned char *decbuf, const int declen, unsigned char *encbuf, int *enclen, card_pay_info_t *pCpi)
{
	int padlen;
	unsigned char padmsg[MAX_BUF_SIZE];
	unsigned char siVect[SEED_BLOCK_SIZE];
	unsigned char mng_no_07[07 + 1];
	unsigned char mng_no_13[13 + 1];
	 
	memset(padmsg   , 0x00, sizeof(padmsg   ));
	memset(siVect   , 0x00, sizeof(siVect   ));
	memset(mng_no_07, 0x00, sizeof(mng_no_07));
	memset(mng_no_13, 0x00, sizeof(mng_no_13));

	memcpy(mng_no_07, pCpi->mng_no + 00, 07);
	memcpy(mng_no_13, pCpi->mng_no + 07, 13);

    SEED_KEY_SCHEDULE sks;
    if (makeSeedKeySchedule(mng_no_07, mng_no_13, &sks) < 0) {
        fprintf(stderr, "makeSeedKeySchedule() failed! mng_no_07[%s] mng_no_13[%s]\n", mng_no_07, mng_no_13);
        return -1;
    }
    
    paddingproc(decbuf, declen, padmsg, &padlen, ENCRYPT_MSG);
    CRYPTO_cbc128_encrypt(padmsg, encbuf, padlen, &sks, siVect, (block128_f)SEED_encrypt);
    *enclen = padlen;

    if (*enclen > 300) {
        fprintf(stderr, "decbuf[%d:%s] enclen[%d] is too long. must be under 300\n", declen, decbuf, *enclen);
        return -1;
    }

	return 0;
}

int sh_decrypt(unsigned char *encbuf, const int enclen, unsigned char *decbuf, int *declen, card_pay_info_t *pCpi)
{
	unsigned char unpadmsg[MAX_BUF_SIZE];
	unsigned char siVect[SEED_BLOCK_SIZE];
	unsigned char mng_no_07[07 + 1];
	unsigned char mng_no_13[13 + 1];
	 
	memset(unpadmsg , 0x00, sizeof(unpadmsg ));
	memset(siVect   , 0x00, sizeof(siVect   ));
	memset(mng_no_07, 0x00, sizeof(mng_no_07));
	memset(mng_no_13, 0x00, sizeof(mng_no_13));

	memcpy(mng_no_07, pCpi->mng_no + 00, 07);
	memcpy(mng_no_13, pCpi->mng_no + 07, 13);

    SEED_KEY_SCHEDULE sks;
    if (makeSeedKeySchedule(mng_no_07, mng_no_13, &sks) < 0) {
        fprintf(stderr, "makeSeedKeySchedule() failed!\n");
        return -1;
    }
    
	CRYPTO_cbc128_decrypt(encbuf, unpadmsg, enclen, &sks, siVect, (block128_f)SEED_decrypt);
    paddingproc(unpadmsg, enclen, decbuf, declen, DECRYPT_MSG);
	
	return 0;
}

int sh_getSeedKey(unsigned char *seedkey)
{
	int len;
	FILE *fp;
	char buf[32] = {0x00, };
	if ((fp = fopen(SEEDKEY, "r")) == NULL) {
		fprintf(stderr, "fopen() failed! PATH[%s] (%m)\n", SEEDKEY);
		return -1;
	}
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fprintf(stderr, "fgets() failed! (%m)\n");
		fclose(fp);
		return -1;
	}
	fclose(fp);

	buf[strlen(buf) - 1] = 0x00;
	len = strlen(buf);
	memcpy(seedkey, buf, len);

	return 0;
}

int makeSeedKeySchedule(unsigned char *mng_no_07, unsigned char *mng_no_13, SEED_KEY_SCHEDULE *sks)
{
	unsigned char seedkey[KEYLEN];
	unsigned char CMK    [KEYLEN];
	unsigned char SSKey  [KEYLEN];

	memset(seedkey, 0x00, sizeof(seedkey));
	memset(CMK    , 0x00, sizeof(CMK    ));
	memset(SSKey  , 0x00, sizeof(SSKey  ));

	if (sh_getSeedKey(seedkey) < 0) {
		fprintf(stderr, "sh_getSeedKey() failed!\n");
		return -1;
	}
	if (getCMK(seedkey, mng_no_07, CMK) < 0) return -1;
	if (getSSK(CMK, mng_no_13, SSKey) < 0) return -1;

	SEED_set_key(SSKey, sks);

	return 0;
}

void synKey(unsigned char *originkey, unsigned char *addition, unsigned char *CMK)
{
	SEED_KEY_SCHEDULE sched;
	SEED_set_key(originkey, &sched);
	SEED_encrypt(addition, CMK, &sched);
	return;
}

int getCMK(unsigned char *seedkey, unsigned char *card_no, unsigned char *CMK)
{
	unsigned char addition[KEYLEN] = {0x00, };

	if (CMK == NULL) {
		fprintf(stderr, "CMK is NULL\n");
		return -1;
	}

	memcpy(addition, card_no, 10);
	addition[10] = 0x80;
	synKey(seedkey, addition, CMK);

	return 0;
}

int getSSK(unsigned char *CMK, unsigned char *expr_mmyy, unsigned char *SSK)
{
	int i;
	unsigned char addition[KEYLEN] = {0x00, };

	if (SSK == NULL) {
		fprintf(stderr, "SSK is NULL\n");
		return -1;
	}

	memcpy(addition, expr_mmyy, 4);
	for (i = 0; i < 4; i++) {
		addition[4 + i] = ~addition[i];
	}
	synKey(CMK, addition, SSK);

	return 0;
}

int paddingproc(unsigned char *orgbuf, int orglen, unsigned char *outcomebuf, int *outcomelen, int type)
{
    int i = 0;

	memcpy(outcomebuf, orgbuf, orglen);

    switch (type) {
    case ENCRYPT_MSG: 	// padding
        outcomebuf[orglen] = 0x80;
        *outcomelen = ((int)(orglen / SEED_BLOCK_SIZE) + 1) * SEED_BLOCK_SIZE;
        break;
    case DECRYPT_MSG:	// unpadding
        for (i = orglen -1; i > 0; i--) {
            if (outcomebuf[i] == 0x00) continue;
            else if (outcomebuf[i] == 0x80) {
                outcomebuf[i] = 0x00;
                *outcomelen = i;
                break;
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

