#include "dbutil.h"

/*
 * SQLITE3 접속
 */
int sh_dbcon(const char *pathfile_dbname, sqlite3 **ppDb)
{
	int ret;

	if ((ret = sqlite3_open(pathfile_dbname, ppDb)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_dbcon() fail. db_name[%s] (%d:%s)\n", pathfile_dbname, ret, sqlite3_errmsg(*ppDb));
		return ret;
	}

	printf("sh_dbcon() success. db_name[%s]\n", pathfile_dbname);
	return ret;
}

/*
 * SQLITE3 해제
 */
int sh_dbdiscon(sqlite3 *pDb)
{
	int ret;

	if ((ret = sqlite3_close(pDb)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_dbdiscon() fail. (%d:%s)\n", ret, sqlite3_errmsg(pDb));
		return ret;
	}

	printf("sh_dbdiscon() success.\n");
	return 0;
}

/*
 * sqlite3_prepare() -> deprecated
 * sql_len : -1: sql 의 길이를 직접 찾는다. (NULL 이 나올때까지의 길이를 조회)
 * 재사용하고자 할 경우 sqlite3_reset(stmt);  를 활용한다.
 */
int sh_stmt_open(sqlite3 *pDb, char *sql, const int sql_len, sqlite3_stmt **ppStmt)
{
	int ret;

	if ((ret = sqlite3_prepare_v2(pDb, sql, sql_len, ppStmt, NULL)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_cur_open() fail. sql[%d:%s] (%d:%s)\n", sql_len, sql, ret, sqlite3_errmsg(pDb));
		return ret;
	}

	printf("sh_stmt_open() success. sql[%s]\n", sql);
	return 0;
}

/*
 * BIND: Integer
 */
int sh_db_bind_int(sqlite3_stmt *pStmt, int index, int val)
{
	int ret;

	if ((ret = sqlite3_bind_int(pStmt, index, val)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_cur_bind_int() fail. index[%d] val[%d] (%d)\n", index, val, ret);
	}

	return ret;
}

/*
 * BIND: Long
 */
int sh_db_bind_long(sqlite3_stmt *pStmt, int index, long val)
{
	int ret;

	if ((ret = sqlite3_bind_int64(pStmt, index, val)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_cur_bind_long() fail. index[%d] val[%ld] (%d)\n", index, val, ret);
	}

	return ret;
}

/*
 * BIND: Double
 */
int sh_db_bind_double(sqlite3_stmt *pStmt, int index, double val)
{
	int ret;

	if ((ret = sqlite3_bind_int64(pStmt, index, val)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_cur_bind_double() fail. index[%d] val[%lf] (%d)\n", index, val, ret);
	}

	return ret;
}

/*
 * BIND: String
 * sqlite3_bind_text: UTF-8
 * sqlite3_bind_text16: UTF-16
 * sqlite3_bind_text64: specify encoding type. no need.
 * SQLITE_STATIC : constant string
 * SQLITE_TRANSIENT : changable in the near future
 */
int sh_db_bind_string(sqlite3_stmt *pStmt, int index, const char *str, int str_len)
{
	int ret;

	if ((ret = sqlite3_bind_text(pStmt, index, str, str_len, SQLITE_STATIC)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_cur_bind_string() fail. index[%d] str[%s] (%d)\n", index, str, ret);
	}

	return ret;
}
/*
 * CURSOR FETCH POSSIBLE CHECK
 * ret: SQLITE_ROW(100,데이터 있음), SQLITE_DONE(101,데이터 없음) SQLITE_BUSY(5,lock) SQLITE_ERROR(1), SQLITE_MISUSE(21)
 */
int sh_cur_fetch(sqlite3_stmt *pStmt)
{
	int ret;

	if ((ret = sqlite3_step(pStmt)) == SQLITE_ROW) {
		return 0;
	} else if (ret == SQLITE_DONE) {
		return 1;
	} else {
		fprintf(stderr, "[ERROR]sh_cur_fetch() fail. (%d)\n", ret);
		return -1;
	}

	return ret;
}

int sh_db_exec(sqlite3_stmt *pStmt)
{
	int ret;

	if ((ret = sqlite3_step(pStmt)) != SQLITE_DONE) {
		fprintf(stderr, "[ERROR]sh_db_exec() fail. (%d)\n", ret);
		return -1;
	}

	return 0;
}

/*
 * CURSOR CLOSE
 */
int sh_stmt_close(sqlite3_stmt *pStmt)
{
	return sqlite3_finalize(pStmt);
}

/*
 * START POINT OF COMMIT;
 */
int sh_transaction_begin(sqlite3 *pDb)
{
	int ret;
	char *err_msg;

	if ((ret = sqlite3_exec(pDb, "BEGIN TRANSACTION", NULL, NULL, &err_msg)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_transaction_begin() fail. (%d:%s)\n", ret, err_msg);
	}
	sqlite3_free(err_msg);
	return ret;
}

/*
 * END POINT OF COMMIT;
 */
int sh_transaction_end(sqlite3 *pDb)
{
	int ret;
	char *err_msg;

	if ((ret = sqlite3_exec(pDb, "END TRANSACTION", NULL, NULL, &err_msg)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sh_transaction_end() fail. (%d:%s)\n", ret, err_msg);
	}

	sqlite3_free(err_msg);
	return ret;
}

/*
 * INSERT INTO TB_PAYINFO_STRING
 */
int sh_ins_data_string(sqlite3 *db, const char *val)
{
	int ret;
	char *err_msg;

	char *sql;
	if ((sql = sqlite3_mprintf("INSERT INTO TB_PAYINFO_STRING VALUES ('%q')", val)) == NULL) {
		fprintf(stderr, "[ERROR]sqlite3_mprintf() fail. val[%s] (%s)\n", val, sqlite3_errmsg(db));
		return -1;
	}

	if ((ret = sqlite3_exec(db, sql, NULL, NULL, &err_msg)) != SQLITE_OK) {
		fprintf(stderr, "[ERROR]sqlite3_exec() fail. sql[%s] (%d:%s)\n", sql, ret, err_msg);
		return -1;
	}

	sqlite3_free(err_msg);
	sqlite3_free(sql);

	return 0;
}

void sh_db_commit(sqlite3 *db)
{
	sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
}
void sh_db_rollback(sqlite3 *db)
{
	sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
}

