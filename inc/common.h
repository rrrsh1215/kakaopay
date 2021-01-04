#ifndef _common_h_
#define _common_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#define D_SIZE_TOTAL       450
#define D_SIZE_SSL_LEN		 4
#define D_SIZE_REQ_CD		10
#define D_SIZE_MNG_NO		20
#define D_SIZE_CARD_NO		20
#define D_SIZE_ISTM_CD		 2
#define D_SIZE_EXPR_MMYY     4
#define D_SIZE_CVC           3
#define D_SIZE_PAY_AMT		10
#define D_SIZE_VAT			10
#define D_SIZE_ORG_MNG_NO	20
#define D_SIZE_ENC_INFO		300
#define D_SIZE_FILLER	 	47	

// type
#define INT     0x01
#define LONG    0x02
#define CHAR    0x03
#define STR     0x04
#define FLOAT   0x05
#define DOUBLE  0x06
#define HEXA    0x07

// return Code
#define D_SIZE_RETCD     2
#define D_RETCD_SUCC   "00"
#define D_RETCD_FAIL   "01"

typedef struct _card_pay_info_t
{
    int  ssl_len                             ;
    char req_cd      [ D_SIZE_REQ_CD     + 1];
    char mng_no      [ D_SIZE_MNG_NO     + 1];
    long card_no                             ;
    char istm_cd     [ D_SIZE_ISTM_CD    + 1];
    int  expr_mmyy                           ;
	int  cvc                                 ;
    long pay_amt                             ;
    long vat                                 ;
    char org_mng_no  [ D_SIZE_ORG_MNG_NO + 1];
    char enc_info    [ D_SIZE_ENC_INFO   + 1];
    char filler      [ D_SIZE_FILLER     + 1];
} card_pay_info_t;

#endif

