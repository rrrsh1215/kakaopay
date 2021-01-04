#include <castutil.h>
#include <cfgutil.h>
#include <dbutil.h>
#include <seedutil.h>
#include <timeutil.h>
#include <json-c/json.h>

typedef struct _shryu_t
{
	int  timegap           ;    // 동시 결제(취소) 방지용 시간 차 (sec)
	char home     [128 + 1];
	char command  [  6 + 1];    // 요청 행위
	int  reqno             ;    // 요청 파일 번호
	char now_dttm [ 15 + 1];    // 현재 일시 yyyymmdd hhmiss

	char resbuf   [450 + 1];

	card_pay_info_t cpi;
} shryu_t;

int showResInfo();
int doPost();
int doDelete();
int doPut();
int doGet();
int getReqInfo();
int convJSONtoPAYINFO();
int getMngNo();
int encPAYINFO();
int makePAYINFO_ELSE();
int insReqInfoString();
int makeResInfo();
int decPAYINFO();
int getMngno();
int selPayinfo();
int doProcess();

void showCardPayInfo(card_pay_info_t *pCpi, const int line)
{
    printf("[%04d]---------------------------------------------\n", line);
    printf("ssl_len    [%d] \n", pCpi->ssl_len    );
    printf("req_cd     [%s] \n", pCpi->req_cd     );
    printf("mng_no     [%s] \n", pCpi->mng_no     );
    printf("card_no    [%ld]\n", pCpi->card_no    );
    printf("istm_cd    [%s] \n", pCpi->istm_cd    );
    printf("expr_mmyy  [%d] \n", pCpi->expr_mmyy  );
    printf("cvc        [%d] \n", pCpi->cvc        );
    printf("pay_amt    [%ld]\n", pCpi->pay_amt    );
    printf("vat        [%ld]\n", pCpi->vat        );
    printf("org_mng_no [%s] \n", pCpi->org_mng_no );
    printf("enc_info   [%s] \n", pCpi->enc_info   );
    printf("filler     [%s] \n", pCpi->filler     );
    printf("---------------------------------------------\n");

	return;
}

int checkParam(int argc, char *argv[], shryu_t *psh)
{
	if (argc < 3) {
		fprintf(stderr, "[ERROR]Usage) %s [GET|PUT|POST|DELETE] reqno. ex) %s GET 00\n", argv[0], argv[0]);
		return -1;
	}

	if (strcmp(argv[1], "GET") != 0 &&
		strcmp(argv[1], "PUT") != 0 &&
		strcmp(argv[1], "POST") != 0 &&
		strcmp(argv[1], "DELETE") != 0) {

		fprintf(stderr, "[ERROR]Usage) %s [GET|PUT|POST|DELETE] jsonfilename. ex) %s GET 00_req.json\n", argv[0], argv[0]);
		return -1;
	}
	printf("argc[%d] %s %s %s\n", argc, argv[0], argv[1], argv[2]);

	strcpy(psh->command, argv[1]);
	       psh->reqno = atoi(argv[2]);
	
	if (sh_getdatetime(psh->now_dttm, 0) < 0) {
		fprintf(stderr, "[ERROR]sh_getdatetime() fail.\n");
		return -1;
	}

	return 0;
}

/*
 * POST  : INSERT
 * DELETE: 전체취소
 * PUT   : 부분취소
 * GET   : 조회
 */
int main(int argc, char *argv[])
{
	char pathfile_db[128];

	shryu_t sh, *psh;
	psh = &sh;

	memset(pathfile_db, 0x00, sizeof(pathfile_db));
	memset(psh        , 0x00, sizeof(shryu_t    ));

	if (checkParam(argc, argv, psh) < 0) {
		return -1;
	}
	if (getConfig(psh->home, pathfile_db, &psh->timegap) < 0) {
		return -1;
	}

	printf("----------------------\n");
	printf("home       [%s]\n", psh->home    );
	printf("command    [%s]\n", psh->command );
	printf("reqno      [%d]\n", psh->reqno   );
	printf("timegap    [%d]\n", psh->timegap );
	printf("pathfile_db[%s]\n", pathfile_db  );
	printf("now        [%s]\n", psh->now_dttm);
	printf("----------------------\n");

	if (doProcess(pathfile_db, psh) < 0) {
		if (makeResInfo(psh, D_RETCD_FAIL) < 0) {
			return -1;
		}
	} else {
		if (makeResInfo(psh, D_RETCD_SUCC) < 0) {
			return -1;
		}
	}

	if (showResInfo(psh) < 0) {
		return -1;
	}

	printf("---> finished!!!  \n");

	return 0;
}

int doProcess(char *pathfile_db, shryu_t *psh)
{
	sqlite3 *db;

	if (sh_dbcon(pathfile_db, &db) < 0) {
		return -1;
	}

	if (memcmp(psh->command, "POST"  , 4) == 0 ||
		memcmp(psh->command, "DELETE", 6) == 0 ||
		memcmp(psh->command, "PUT"   , 3) == 0) {

		// 요청정보 조회
		if (getReqInfo(db, psh) < 0) {
			sh_dbdiscon(db);
			return -1;
		}
		showCardPayInfo(&psh->cpi, __LINE__);

		// string 문자열 저장
		if (insReqInfoString(db, psh) < 0) {
			sh_db_rollback(db);
			sh_dbdiscon(db);
			return -1;
		}
	}

	if (memcmp(psh->command, "POST", 4) == 0) {
		if (doPost(db, psh) < 0) {
			sh_db_rollback(db);
			sh_dbdiscon(db);
			return -1;
		}
	} else if (memcmp(psh->command, "DELETE", 4) == 0) {
		if (doDelete(db, psh) < 0) {
			sh_db_rollback(db);
			sh_dbdiscon(db);
			return -1;
		}
	} else if (memcmp(psh->command, "PUT", 4) == 0) {
		if (doPut(db, psh) < 0) {
			sh_db_rollback(db);
			sh_dbdiscon(db);
			return -1;
		}
	} else if (memcmp(psh->command, "GET", 4) == 0) {
		if (doGet(db, psh) < 0) {
			sh_db_rollback(db);
			sh_dbdiscon(db);
			return -1;
		}
	}

	sh_db_commit(db);
	sh_dbdiscon(db);

	return 0;
}

int insReqInfoString(sqlite3 *db, shryu_t *psh)
{
	int reqlen = 0;

	char szReqbuf [450 + 1];

	char szSsl_len    [ D_SIZE_SSL_LEN    + 1];
	char szReq_cd     [ D_SIZE_REQ_CD     + 1];
	char szCard_no    [ D_SIZE_CARD_NO    + 1];
	char szExpr_mmyy  [ D_SIZE_EXPR_MMYY  + 1];
	char szCvc        [ D_SIZE_CVC        + 1];
	char szPay_amt    [ D_SIZE_PAY_AMT    + 1];
	char szVat        [ D_SIZE_VAT        + 1];
	char szOrg_mng_no [ D_SIZE_ORG_MNG_NO + 1];
	char szEnc_info   [ D_SIZE_ENC_INFO   + 1];

	card_pay_info_t *pCpi = &psh->cpi;

	memset(szReqbuf     , 0x00, sizeof(szReqbuf     ));

	memset(szSsl_len    , 0x00, sizeof(szSsl_len    ));
	memset(szReq_cd     , 0x00, sizeof(szReq_cd     ));
	memset(szCard_no    , 0x00, sizeof(szCard_no    ));
	memset(szExpr_mmyy  , 0x00, sizeof(szExpr_mmyy  ));
	memset(szCvc        , 0x00, sizeof(szCvc        ));
	memset(szPay_amt    , 0x00, sizeof(szPay_amt    ));
	memset(szVat        , 0x00, sizeof(szVat        ));
	memset(szOrg_mng_no , 0x00, sizeof(szOrg_mng_no ));
	memset(szEnc_info   , 0x00, sizeof(szEnc_info   ));

	memset(szOrg_mng_no , 0x20, D_SIZE_ORG_MNG_NO    );
	memcpy(szOrg_mng_no, pCpi->org_mng_no, strlen(pCpi->org_mng_no));

	itos_padding(pCpi->ssl_len , szSsl_len, 4, ' ', 'L');
	stos_padding(pCpi->req_cd  , strlen(pCpi->req_cd), szReq_cd, D_SIZE_REQ_CD, ' ');
	ltos_padding(pCpi->pay_amt , szPay_amt, 10, ' ', 'L');
	ltos_padding(pCpi->vat     , szVat    , 10, '0', 'L');
	if (memcmp(psh->command, "POST", 4) == 0) {
		ltos_padding(pCpi->card_no , szCard_no, D_SIZE_CARD_NO, ' ', 'R');
		stos_padding(pCpi->enc_info, strlen(pCpi->enc_info), szEnc_info, D_SIZE_ENC_INFO, ' ');
		sprintf(szExpr_mmyy, "%04d", pCpi->expr_mmyy);
		sprintf(szCvc      , "%03d", pCpi->cvc      );
	} else {
		memset(szCard_no  , 0x20, D_SIZE_CARD_NO  );
		memset(szExpr_mmyy, 0x20, D_SIZE_EXPR_MMYY);
		memset(szCvc      , 0x20, D_SIZE_CVC      );
		memset(szEnc_info , 0x20, D_SIZE_ENC_INFO );
	}

	sprintf(szReqbuf        , "%s"             , szSsl_len        ); reqlen += D_SIZE_SSL_LEN    ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szReq_cd         , D_SIZE_REQ_CD    ); reqlen += D_SIZE_REQ_CD     ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, pCpi->mng_no     , D_SIZE_MNG_NO    ); reqlen += D_SIZE_MNG_NO     ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szCard_no        , D_SIZE_CARD_NO   ); reqlen += D_SIZE_CARD_NO    ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, pCpi->istm_cd    , D_SIZE_ISTM_CD   ); reqlen += D_SIZE_ISTM_CD    ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szExpr_mmyy      , D_SIZE_EXPR_MMYY ); reqlen += D_SIZE_EXPR_MMYY  ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szCvc            , D_SIZE_CVC       ); reqlen += D_SIZE_CVC        ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szPay_amt        , D_SIZE_PAY_AMT   ); reqlen += D_SIZE_PAY_AMT    ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szVat            , D_SIZE_VAT       ); reqlen += D_SIZE_VAT        ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szOrg_mng_no     , D_SIZE_ORG_MNG_NO); reqlen += D_SIZE_ORG_MNG_NO ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memcpy(szReqbuf + reqlen, szEnc_info       , D_SIZE_ENC_INFO  ); reqlen += D_SIZE_ENC_INFO   ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);
	memset(szReqbuf + reqlen, 0x20             , D_SIZE_FILLER    ); reqlen += D_SIZE_FILLER     ; printf("szReqbuf[%03ld:%s]\n", strlen(szReqbuf), szReqbuf);

	// db write
	if (sh_ins_data_string(db, szReqbuf) < 0) {
		return -1;
	}

	memcpy(psh->resbuf, szReqbuf, 450);

	return 0;
}

int getLastPayinfoByCard_no(sqlite3 *db, const long card_no, char *last_dttm)
{
	int ret;

	char sql[1024];
	char szCard_no[D_SIZE_CARD_NO + 1];

	sqlite3_stmt *stmt;

	memset(sql, 0x00, sizeof(sql));
	sprintf(szCard_no, "%ld", card_no);

	strcpy(sql, "SELECT SYSDTTM FROM TB_PAYINFO WHERE REQ_CD = 'PAYMENT' AND CARD_NO = ? ORDER BY SYSDTTM DESC");
	if (sh_stmt_open(db, sql, -1, &stmt) < 0) {
		return -1;
	}

	if (sh_db_bind_string(stmt, 1, szCard_no, strlen(szCard_no)) < 0) return -1;

	if ((ret = sh_cur_fetch(stmt)) < 0) {
		return -1;
	} else if (ret == 1) {
		printf("no data found TB_PAYINFO. card_no[%ld]\n", card_no);
		sh_stmt_close(stmt);
		return 0;
	}

	memcpy(last_dttm , (char *)sqlite3_column_text (stmt, 0), 15);

	sh_stmt_close(stmt);
	return 0;
}

int insPayinfo(sqlite3 *db, shryu_t *psh)
{
	char szCard_no[D_SIZE_CARD_NO + 1];

	card_pay_info_t *pCpi = &psh->cpi;

	sprintf(szCard_no, "%ld", pCpi->card_no);

	char *sql = "INSERT INTO TB_PAYINFO (MNG_NO, REQ_CD, CARD_NO, ISTM_CD, PAY_AMT, VAT, ORG_MNG_NO, ENC_INFO, SYSDTTM) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

	sqlite3_stmt *stmt;

	if (sh_stmt_open(db, sql, -1, &stmt) < 0) {
		return -1;
	}
	if (sh_db_bind_string(stmt, 1, pCpi->mng_no    , D_SIZE_MNG_NO        ) < 0) return -1;
	if (sh_db_bind_string(stmt, 2, pCpi->req_cd    , strlen(pCpi->req_cd )) < 0) return -1;
	if (sh_db_bind_string(stmt, 3, szCard_no       , strlen(szCard_no    )) < 0) return -1;
	if (sh_db_bind_string(stmt, 4, pCpi->istm_cd   , D_SIZE_ISTM_CD       ) < 0) return -1;
	if (sh_db_bind_long  (stmt, 5, pCpi->pay_amt                          ) < 0) return -1;
	if (sh_db_bind_long  (stmt, 6, pCpi->vat                              ) < 0) return -1;
	if (sh_db_bind_string(stmt, 7, pCpi->org_mng_no, D_SIZE_ORG_MNG_NO    ) < 0) return -1;
	if (sh_db_bind_string(stmt, 8, pCpi->enc_info  , D_SIZE_ENC_INFO      ) < 0) return -1;
	if (sh_db_bind_string(stmt, 9, psh->now_dttm   , 15                   ) < 0) return -1;

	if (sh_db_exec(stmt) < 0) {
		sh_stmt_close(stmt); 
		return -1;
	}

	sh_stmt_close(stmt); 
	return 0;
}

int doPost(sqlite3 *db, shryu_t *psh)
{
	char last_dttm[15 + 1];

	card_pay_info_t *pCpi = &psh->cpi;

	memset(last_dttm, 0x00, sizeof(last_dttm));

	// 해당 카드로 최근 결제내역 조회
	if (getLastPayinfoByCard_no(db, pCpi->card_no, last_dttm) < 0) {
		return -1;
	}

	// 최근 결제내역이 존재한다면
	if (last_dttm[0] != 0x00) {
		// 동시 결제 여부 확인
		if (TimeGapAfterYN(last_dttm, psh->now_dttm, psh->timegap) != 'Y') {
			fprintf(stderr, "[ERROR]동시결제입니다. bef[%s] after[%s] timegap[%d]\n", last_dttm, psh->now_dttm, psh->timegap);
			return -1;
		}
	}

	if (insPayinfo(db, psh) < 0) {
		return -1;
	}

	return 0;
}

int getLastCancelinfoByMngno(sqlite3 *db, char *org_mng_no, char *last_dttm)
{
	int ret;

	char sql[1024];

	sqlite3_stmt *stmt;

	memset(sql, 0x00, sizeof(sql));

	strcpy(sql, "SELECT SYSDTTM FROM TB_PAYINFO WHERE ORG_MNG_NO = ? ORDER BY SYSDTTM DESC");
	if (sh_stmt_open(db, sql, -1, &stmt) < 0) {
		return -1;
	}

	if (sh_db_bind_string(stmt, 1, org_mng_no, D_SIZE_ORG_MNG_NO) < 0) return -1;

	if ((ret = sh_cur_fetch(stmt)) < 0) {
		return -1;
	} else if (ret == 1) {
		printf("no data found TB_PAYINFO. org_mng_no[%s]\n", org_mng_no);
		sh_stmt_close(stmt);
		return 0;
	}

	memcpy(last_dttm , (char *)sqlite3_column_text (stmt, 0), 15);

	sh_stmt_close(stmt);
	return 0;
}


int doDelete(sqlite3 *db, shryu_t *psh)
{
	char last_dttm[15 + 1];

	card_pay_info_t cpi_org;  // 잔존 결제 정보
	card_pay_info_t *pCpi = &psh->cpi;
	sqlite3_stmt *stmt;

	memset(last_dttm, 0x00, sizeof(last_dttm      ));
	memset(&cpi_org , 0x00, sizeof(card_pay_info_t));

	// 잔존금액 확인
	memcpy(cpi_org.mng_no, pCpi->org_mng_no, D_SIZE_MNG_NO);
	if (selPayinfo(db, &cpi_org) < 0) {
		return -1;
	}
	if (cpi_org.pay_amt <= 0) {
		fprintf(stderr, "[ERROR]잔존 결제금액이 존재하지 않습니다. mng_no[%s] pay_amt[%ld]\n", pCpi->org_mng_no, cpi_org.pay_amt);
		return -1;
	}
	if (cpi_org.pay_amt != pCpi->pay_amt) {
		fprintf(stderr, "[ERROR]잔존 결제금액[%ld] != 취소금액[%ld]\n", cpi_org.pay_amt, pCpi->pay_amt);
		return -1;
	}

	// 관리번호로 최근 취소내역 조회
	if (getLastCancelinfoByMngno(db, pCpi->org_mng_no, last_dttm) < 0) {
		return -1;
	}

	// 최근 결제내역이 존재한다면
	if (last_dttm[0] != 0x00) {
		// 동시 결제 여부 확인
		if (TimeGapAfterYN(last_dttm, psh->now_dttm, psh->timegap) != 'Y') {
			fprintf(stderr, "[ERROR]동시 취소요청입니다. bef[%s] after[%s] timegap[%d]\n", last_dttm, psh->now_dttm, psh->timegap);
			return -1;
		}
	}

	// 취소내역 저장
	if (insPayinfo(db, psh) < 0) {
		return -1;
	}

	// 취소처리
	char *sql = "UPDATE TB_PAYINFO SET PAY_AMT = 0, VAT = 0 WHERE MNG_NO = ?";
	if (sh_stmt_open(db, sql, -1, &stmt) < 0) {
		return -1;
	}
	if (sh_db_bind_string(stmt, 1, pCpi->org_mng_no, D_SIZE_ORG_MNG_NO) < 0) return -1;

	if (sh_db_exec(stmt) < 0) {
		sh_stmt_close(stmt); 
		return -1;
	}

	sh_stmt_close(stmt); 

	return 0;
}

int doPut(sqlite3 *db, shryu_t *psh)
{
	char last_dttm[15 + 1];

	card_pay_info_t cpi_org;  // 잔존 결제 정보
	card_pay_info_t *pCpi = &psh->cpi;
	sqlite3_stmt *stmt;

	memset(last_dttm, 0x00, sizeof(last_dttm      ));
	memset(&cpi_org , 0x00, sizeof(card_pay_info_t));

	// 잔존금액 확인
	memcpy(cpi_org.mng_no, pCpi->org_mng_no, D_SIZE_MNG_NO);
	if (selPayinfo(db, &cpi_org) < 0) {
		return -1;
	}
	if (cpi_org.pay_amt <= 0) {
		fprintf(stderr, "[ERROR]잔존 결제금액이 존재하지 않습니다. mng_no[%s] pay_amt[%ld]\n", pCpi->org_mng_no, cpi_org.pay_amt);
		return -1;
	}
	if (cpi_org.pay_amt < pCpi->pay_amt) {
		fprintf(stderr, "[ERROR]잔존 결제금액[%ld] < 취소금액[%ld]\n", cpi_org.pay_amt, pCpi->pay_amt);
		return -1;
	} else if (cpi_org.pay_amt == pCpi->pay_amt) {
		if (cpi_org.vat != pCpi->vat) {
			fprintf(stderr, "[ERROR]잔존 VAT[%ld] != 취소 VAT[%ld]. when pay_amt 는 동일\n", cpi_org.vat, pCpi->vat);
			return -1;
		}
	} else {
		if (cpi_org.vat < pCpi->vat) {
			fprintf(stderr, "[ERROR]잔존 VAT[%ld] < 취소 VAT[%ld].\n", cpi_org.vat, pCpi->vat);
			return -1;
		}
	}

	// 관리번호로 최근 취소내역 조회
	if (getLastCancelinfoByMngno(db, pCpi->org_mng_no, last_dttm) < 0) {
		return -1;
	}

	// 최근 결제내역이 존재한다면
	if (last_dttm[0] != 0x00) {
		// 동시 결제 여부 확인
		if (TimeGapAfterYN(last_dttm, psh->now_dttm, psh->timegap) != 'Y') {
			fprintf(stderr, "[ERROR]동시 취소요청입니다. bef[%s] after[%s] timegap[%d]\n", last_dttm, psh->now_dttm, psh->timegap);
			return -1;
		}
	}

	// 취소내역 저장
	if (insPayinfo(db, psh) < 0) {
		return -1;
	}

	// 취소처리
	char *sql = "UPDATE TB_PAYINFO SET PAY_AMT = ?, VAT = ? WHERE MNG_NO = ?";
	if (sh_stmt_open(db, sql, -1, &stmt) < 0) {
		return -1;
	}
	if (sh_db_bind_long  (stmt, 1, cpi_org.pay_amt - pCpi->pay_amt    ) < 0) return -1;
	if (sh_db_bind_long  (stmt, 2, cpi_org.vat     - pCpi->vat        ) < 0) return -1;
	if (sh_db_bind_string(stmt, 3, pCpi->org_mng_no, D_SIZE_ORG_MNG_NO) < 0) return -1;

	if (sh_db_exec(stmt) < 0) {
		sh_stmt_close(stmt); 
		return -1;
	}

	sh_stmt_close(stmt); 
	return 0;
}

int doGet(sqlite3 *db, shryu_t *psh)
{
	if (getMngno(db, psh) < 0) {
		return -1;
	}

	if (selPayinfo(db, &psh->cpi) < 0) {
		return -1;
	}

	return 0;
}

int selPayinfo(sqlite3 *db, card_pay_info_t *pCpi)
{
	int ret;

	char sql[1024];
	char enc_info[D_SIZE_ENC_INFO + 1];

	sqlite3_stmt *stmt;

	memset(sql     , 0x00, sizeof(sql     ));
	memset(enc_info, 0x00, sizeof(enc_info));

	strcpy(sql, "SELECT ISTM_CD, PAY_AMT, VAT, ENC_INFO FROM TB_PAYINFO WHERE MNG_NO = ?");
	
	if (sh_stmt_open(db, sql, -1, &stmt) < 0) {
		return -1;
	}

	if (sh_db_bind_string(stmt, 1, pCpi->mng_no, D_SIZE_MNG_NO) < 0) return -1;

	if ((ret = sh_cur_fetch(stmt)) < 0) {
		return -1;
	} else if (ret == 1) {
		fprintf(stderr, "[ERROR]no data found TB_PAYINFO_STRING. MNG_NO[%s]\n", pCpi->mng_no);
		sh_stmt_close(stmt);
		return -1;
	}

	strcpy(pCpi->istm_cd  , (char *)sqlite3_column_text (stmt, 0));
	       pCpi->pay_amt  =         sqlite3_column_int64(stmt, 1) ;
	       pCpi->vat      =         sqlite3_column_int64(stmt, 2) ;
	strcpy(enc_info       , (char *)sqlite3_column_text (stmt, 3));

	stos_unpadding(enc_info, strlen(enc_info), pCpi->enc_info, ' ', 'R');

	sh_stmt_close(stmt);
	return 0;
}

int getMngno(sqlite3 *db, shryu_t *psh)
{
	int str_len;
	char pathfile_req[1024 + 1];

	card_pay_info_t *pCpi = &psh->cpi;

	sprintf(pathfile_req, "%s/jsonfile/%02d_req.json", psh->home, psh->reqno);
	fprintf(stderr, ">>>pathfile_req[%s]\n", pathfile_req);

	json_object *obj = NULL, *obj_val = NULL;

	if ((obj = json_object_from_file(pathfile_req)) == NULL) {
		fprintf(stderr, "no data found. pathfile_req[%s](%s)\n", pathfile_req, json_util_get_last_err());
		return -1;
	}

	// mng_no
	if ((obj_val = json_object_object_get(obj, "Mng_no")) == NULL) {
		fprintf(stderr, "[ERROR]json_object_object_get() fail. key[Mng_no] (%s)\n", json_util_get_last_err());
		return -1;
	}
	str_len = strlen(json_object_get_string(obj_val));
	if (str_len != 20) {
		fprintf(stderr, "[ERROR] Invalid length[%d] of mng_no.\n", str_len);
		return -1;
	}
	memcpy(pCpi->mng_no, json_object_get_string(obj_val), str_len);

	// 자원 해제
	json_object_put(obj);

	return 0;
}

int makeResInfo(shryu_t *psh, const char *ret_cd)
{
	char pathfile_res[1024 + 1];
	char card_no_masking[D_SIZE_CARD_NO + 1];

	json_object *obj;
	card_pay_info_t *pCpi = &psh->cpi;

	sprintf(pathfile_res, "%s/jsonfile/%02d_res.json", psh->home, psh->reqno);

	if ((obj = json_object_new_object()) == NULL) {
		fprintf(stderr, "[ERROR]json_object_new_object() fail. (%s)\n", json_util_get_last_err());
		return -1;
	}

	if (memcmp(psh->command, "POST"  , 4) == 0 ||
		memcmp(psh->command, "DELETE", 6) == 0 ||
		memcmp(psh->command, "PUT"   , 3) == 0) {

		json_object_object_add(obj, "Mng_no" , json_object_new_string(pCpi->mng_no));
		json_object_object_add(obj, "Res_buf", json_object_new_string(psh->resbuf ));
	
	} else if (memcmp(psh->command, "GET", 3) == 0) {
		if (memcmp(ret_cd, D_RETCD_SUCC, D_SIZE_RETCD) == 0) {
			// 복호화
			if (decPAYINFO(pCpi) < 0) {
				json_object_put(obj);
				return -1;
			}
			printf("decrypt(): card_no[%ld] expr_mmyy[%d] cvc[%d]\n", pCpi->card_no, pCpi->expr_mmyy, pCpi->cvc);

			// 마스킹처리
			sprintf(card_no_masking, "%ld", pCpi->card_no);
			memset(card_no_masking + 6, '*', strlen(card_no_masking) - 9);
		}

		json_object_object_add(obj, "Mng_no"    , json_object_new_string(pCpi->mng_no    ));
		json_object_object_add(obj, "Card_no"   , json_object_new_string(card_no_masking ));
		json_object_object_add(obj, "Expr_mmyy" , json_object_new_int   (pCpi->expr_mmyy ));
		json_object_object_add(obj, "Cvc"       , json_object_new_int   (pCpi->cvc       ));
		json_object_object_add(obj, "Pay_amt"   , json_object_new_int64 (pCpi->pay_amt   ));
		json_object_object_add(obj, "Vat"       , json_object_new_int64 (pCpi->vat       ));
	}

	json_object_object_add(obj, "Ret_cd", json_object_new_string(ret_cd));
	if (memcmp(ret_cd, D_RETCD_FAIL, 2) == 0) {
		json_object_object_add(obj, "Ret_msg"    , json_object_new_string("fail. call & ask."));
	} else {
		json_object_object_add(obj, "Ret_msg"    , json_object_new_string("success!"));
	}

	if (json_object_to_file(pathfile_res, obj) < 0) {
		fprintf(stderr, "[ERROR]json_object_to_file() fail. pathfile_res[%s] (%s)\n", pathfile_res, json_util_get_last_err());
		return -1;
	}

	// 자원 해제
	json_object_put(obj);

	return(0);
}

int showResInfo(shryu_t *psh)
{
	int str_len;
    char pathfile_res[1024 + 1];

	char mng_no [D_SIZE_MNG_NO + 1];
	char ret_cd [D_SIZE_RETCD  + 1];
	char ret_msg[128           + 1];
	char res_buf[D_SIZE_TOTAL  + 1];

	memset(mng_no , 0x00, sizeof(mng_no ));
	memset(ret_cd , 0x00, sizeof(ret_cd ));
	memset(ret_msg, 0x00, sizeof(ret_msg));
	memset(res_buf, 0x00, sizeof(res_buf));

    json_object *obj = NULL, *obj_val = NULL;
    //card_pay_info_t *pCpi = &psh->cpi;

    sprintf(pathfile_res, "%s/jsonfile/%02d_res.json", psh->home, psh->reqno);
    printf(">>>pathfile_res[%s]\n", pathfile_res);

    if ((obj = json_object_from_file(pathfile_res)) == NULL) {
        fprintf(stderr, "[ERROR]no data found. pathfile_res[%s](%s)\n", pathfile_res, json_util_get_last_err());
        return -1;
    }

	// mng_no
	if ((obj_val = json_object_object_get(obj, "Mng_no")) != NULL) {
		str_len = strlen(json_object_get_string(obj_val));
		if (str_len != D_SIZE_MNG_NO) {
			memset(mng_no, 0x20, D_SIZE_MNG_NO);
		} else {
			memcpy(mng_no, json_object_get_string(obj_val), str_len);
		}
	}

	// ret_cd
	if ((obj_val = json_object_object_get(obj, "Ret_cd")) != NULL) {
		str_len = strlen(json_object_get_string(obj_val));
		if (str_len != D_SIZE_RETCD) {
			memset(ret_cd, 0x20, D_SIZE_RETCD);
		} else {
			memcpy(ret_cd, json_object_get_string(obj_val), str_len);
		}
	}

	// ret_msg
	if ((obj_val = json_object_object_get(obj, "Ret_msg")) != NULL) {
		str_len = strlen(json_object_get_string(obj_val));
		memcpy(ret_msg, json_object_get_string(obj_val), str_len);
	}

	if (memcmp(psh->command, "POST"  , 4) == 0 ||
		memcmp(psh->command, "DELETE", 6) == 0 ||
		memcmp(psh->command, "PUT"   , 3) == 0) {

		// res_buf
		if ((obj_val = json_object_object_get(obj, "Res_buf")) != NULL) {
			str_len = strlen(json_object_get_string(obj_val));
			memcpy(res_buf, json_object_get_string(obj_val), str_len);
		}

		printf("------------------- res ----------------------------\n");
		printf("mng_no [%s]\n", mng_no );
		printf("ret_cd [%s]\n", ret_cd );
		printf("ret_msg[%s]\n", ret_msg);
		printf("res_buf[%s]\n", res_buf);
		printf("----------------------------------------------------\n");

	} else if (memcmp(psh->command, "GET", 3) == 0) {
		char card_no   [D_SIZE_CARD_NO + 1];
		int  expr_mmyy = 0;
		int  cvc       = 0;
		long pay_amt   = 0;
		long vat       = 0;

		memset(card_no, 0x00, sizeof(card_no));

		// res_buf
		if ((obj_val = json_object_object_get(obj, "Card_no")) != NULL) {
			str_len = strlen(json_object_get_string(obj_val));
			if (str_len < 10 || str_len > 20) {
				memset(card_no, 0x20, D_SIZE_CARD_NO);
			} else {
				memcpy(card_no, (char *)json_object_get_string(obj_val), str_len);
			}
		}

		// expr_mmyy
		if ((obj_val = json_object_object_get(obj, "Card_no")) != NULL) {
			expr_mmyy = json_object_get_int(obj_val);
		}

		// cvc 
		if ((obj_val = json_object_object_get(obj, "Cvc")) != NULL) {
			cvc = json_object_get_int(obj_val);
		}

		// pay_amt 
		if ((obj_val = json_object_object_get(obj, "Pay_amt")) != NULL) {
			pay_amt = json_object_get_int64(obj_val);
		}

		// vat 
		if ((obj_val = json_object_object_get(obj, "Vat")) != NULL) {
			vat = json_object_get_int64(obj_val);
		}

		printf("------------------- res ----------------------------\n");
		printf("mng_no    [%s] \n", mng_no    );
		printf("card_no   [%s] \n", card_no   );
		printf("expr_mmyy [%d] \n", expr_mmyy );
		printf("cvc       [%d] \n", cvc       );
		printf("pay_amt   [%ld]\n", pay_amt   );
		printf("vat       [%ld]\n", vat       );
		printf("ret_cd    [%s] \n", ret_cd    );
		printf("ret_msg   [%s] \n", ret_msg   );
		printf("----------------------------------------------------\n");

	}

    // 자원 해제
    json_object_put(obj);

	return 0;
}

int getReqInfo(sqlite3 *db, shryu_t *psh)
{
	char pathfile_req[1024 + 1];

	card_pay_info_t *pCpi = &psh->cpi;

	sprintf(pathfile_req, "%s/jsonfile/%02d_req.json", psh->home, psh->reqno);
	fprintf(stderr, ">>>pathfile_req[%s]\n", pathfile_req);

	json_object *obj = NULL;

	if ((obj = json_object_from_file(pathfile_req)) == NULL) {
		fprintf(stderr, "[ERROR]no data found. pathfile_req[%s](%s)\n", pathfile_req, json_util_get_last_err());
		return -1;
	}

	if (convJSONtoPAYINFO(psh, obj) < 0) {
		json_object_put(obj);
		return -1;
	}

	if (makePAYINFO_ELSE(db, psh) < 0) {
		json_object_put(obj);
		return -1;
	}

	if (memcmp(psh->command, "POST", 4) == 0) {
		if (encPAYINFO(pCpi) < 0) {
			json_object_put(obj);
			return -1;
		}
	}

	// 자원 해제
	json_object_put(obj);

	return 0;
}

int encPAYINFO(card_pay_info_t *pCpi)
{
	int declen, enclen;
	unsigned char decbuf[1024 + 1];
	unsigned char encbuf[1024 + 1];

	char szCardno[D_SIZE_CARD_NO + 1];

	memset(decbuf, 0x00, sizeof(decbuf));
	memset(encbuf, 0x00, sizeof(encbuf));

	memset(szCardno, 0x00, sizeof(szCardno));

	ltos_padding(pCpi->card_no, szCardno, D_SIZE_CARD_NO, ' ', 'R');

	// enc_info
	sprintf((char *)decbuf, "%s|%04d|%03d", szCardno, pCpi->expr_mmyy, pCpi->cvc);
	declen = strlen((char *)decbuf);

	if (sh_encrypt(decbuf, declen, encbuf, &enclen, pCpi) < 0) {
		return -1;
	}
	memcpy(pCpi->enc_info, encbuf, enclen);
	fprintf(stderr, "decbuf[%d:%s] encbuf[%d:%s]\n", declen, decbuf, enclen, encbuf);

	memcpy(pCpi->enc_info, encbuf, enclen);

	return 0;
}

int decPAYINFO(card_pay_info_t *pCpi)
{
	int declen, enclen;

	unsigned char decbuf[1024 + 1];
	unsigned char encbuf[1024 + 1];

	char szCard_no[D_SIZE_CARD_NO + 1];

	memset(decbuf   , 0x00, sizeof(decbuf   ));
	memset(encbuf   , 0x00, sizeof(encbuf   ));
	memset(szCard_no, 0x00, sizeof(szCard_no));

	enclen = strlen(pCpi->enc_info);
	memcpy(encbuf, pCpi->enc_info, enclen);

	if (sh_decrypt(encbuf, enclen, decbuf, &declen, pCpi) < 0) {
		return -1;
	}

	memcpy(szCard_no, (char *)decbuf, D_SIZE_CARD_NO);
	stol_unpadding(szCard_no, D_SIZE_CARD_NO, &pCpi->card_no, ' ', 'R');

	sh_stoi((char *)decbuf + 21,  4, &pCpi->expr_mmyy);
	sh_stoi((char *)decbuf + 26,  3, &pCpi->cvc      );
	
	return 0;
}

int makePAYINFO_ELSE(sqlite3 *db, shryu_t *psh)
{
	card_pay_info_t *pCpi = &psh->cpi;

	// ssl_len
	pCpi->ssl_len = 446;

	// req_cd
	if (memcmp(psh->command, "POST", 4) == 0) {
		memcpy(pCpi->req_cd, "PAYMENT", 7);
	
	} else if (memcmp(psh->command, "DELETE", 6) == 0 || memcmp(psh->command, "PUT", 3) == 0) {
		memcpy(pCpi->req_cd, "CANCEL", 6);
	}

	// mng_no
	if (sh_get_mng_no(pCpi->mng_no, psh->home) < 0) {
		return -1;
	}

	return 0;
}

int convJSONtoPAYINFO(shryu_t *psh, json_object *pObj)
{
	int str_len = 0;

	card_pay_info_t *pCpi = &psh->cpi;
	json_object *obj_val;

	// pay_amt 
	if ((obj_val = json_object_object_get(pObj, "Pay_amt")) == NULL) {
		fprintf(stderr, "[ERROR]json_object_object_get() fail. key[Pay_amt] (%s)\n", json_util_get_last_err());
		return -1;
	}
	pCpi->pay_amt = json_object_get_int64(obj_val);
	if (pCpi->pay_amt < 100 || pCpi->pay_amt > 9999999999) {
		fprintf(stderr, "[ERROR] Invalid pay_amt[%ld]\n", pCpi->pay_amt);
		return -1;
	}

	// vat 
	if ((obj_val = json_object_object_get(pObj, "Vat")) == NULL) {
		pCpi->vat = round((double)pCpi->pay_amt / 11);

	} else {
		pCpi->vat = json_object_get_int64(obj_val);
		if (pCpi->vat > pCpi->pay_amt) {
			fprintf(stderr, "[ERROR] Invalid vat[%ld] > pCpi->pay_amt[%ld]\n", pCpi->vat, pCpi->pay_amt);
			return -1;
		}
	}

	if (memcmp(psh->command, "POST", 4) == 0) {
		// card_no
		if ((obj_val = json_object_object_get(pObj, "Card_no")) == NULL) {
			fprintf(stderr, "[ERROR]json_object_object_get() fail. key[Card_no] (%s)\n", json_util_get_last_err());
			return -1;
		}
		pCpi->card_no = json_object_get_int64(obj_val);
		if (pCpi->card_no < 1000000000) {
			fprintf(stderr, "[ERROR] Invalid card_no[%ld]\n", pCpi->card_no);
			return -1;
		}

		// installment code
		if ((obj_val = json_object_object_get(pObj, "Istm_cd")) == NULL) {
			fprintf(stderr, "[ERROR]json_object_object_get() fail. key[Istm_cd] (%s)\n", json_util_get_last_err());
			return -1;
		}
		str_len = strlen(json_object_get_string(obj_val));
		if (str_len != 2) {
			fprintf(stderr, "[ERROR] Invalid length[%d] of istm_cd.\n", str_len);
			return -1;
		}
		memcpy(pCpi->istm_cd, json_object_get_string(obj_val), str_len);

		// expr_mmyy
		if ((obj_val = json_object_object_get(pObj, "Expr_mmyy")) == NULL) {
			fprintf(stderr, "[ERROR]json_object_object_get() fail. key[Expr_mmyy] (%s)\n", json_util_get_last_err());
			return -1;
		}
		pCpi->expr_mmyy = json_object_get_int(obj_val);
		if (pCpi->expr_mmyy < 100 || pCpi->expr_mmyy > 1299) {
			fprintf(stderr, "[ERROR] Invalid expr_mmyy[%d]\n", pCpi->expr_mmyy);
			return -1;
		}

		// cvc
		if ((obj_val = json_object_object_get(pObj, "Cvc")) == NULL) {
			fprintf(stderr, "[ERROR]json_object_object_get() fail. key[Cvc] (%s)\n", json_util_get_last_err());
			return -1;
		}
		pCpi->cvc = json_object_get_int(obj_val);
		if (pCpi->cvc > 999) {
			fprintf(stderr, "[ERROR] Invalid cvc[%d]\n", pCpi->cvc);
			return -1;
		}
	} else if (memcmp(psh->command, "DELETE", 6) == 0 || memcmp(psh->command, "PUT", 3) == 0) {
		memcpy(pCpi->istm_cd, "00", 2);
	}

	// org_mng_no
	if (memcmp(psh->command, "DELETE", 6) == 0 || memcmp(psh->command, "PUT", 3) == 0) {
		if ((obj_val = json_object_object_get(pObj, "Org_mng_no")) == NULL) {
			fprintf(stderr, "[ERROR]json_object_object_get() fail. key[Org_mng_no] (%s)\n", json_util_get_last_err());
			return -1;
		}

		str_len = strlen(json_object_get_string(obj_val));
		if (str_len != 20) {
			fprintf(stderr, "[ERROR] Invalid length[%d] of org_mng_no.\n", str_len);
			return -1;
		}
		memcpy(pCpi->org_mng_no, json_object_get_string(obj_val), str_len);
	}

	return 0;
}

