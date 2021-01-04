#ifndef _dbutil_h_
#define _dbutil_h_

#include "common.h"
#include <sqlite3.h>

/*
 * SQLITE3 접속
 */
int sh_dbcon(const char *db_name, sqlite3 **ppDb);

/*
 * SQLITE3 해제
 */
int sh_dbdiscon(sqlite3 *pDb);

/*
 * sqlite3_prepare() -> deprecated
 * sql_len : -1: sql 의 길이를 직접 찾는다. (NULL 이 나올때까지의 길이를 조회)
 */
int sh_stmt_open(sqlite3 *pDb, char *sql, const int sql_len, sqlite3_stmt **ppStmt);

/*
 * BIND: Integer
 */
int sh_db_bind_int(sqlite3_stmt *pStmt, int index, int val);

/*
 * BIND: Long
 */
int sh_db_bind_long(sqlite3_stmt *pStmt, int index, long val);

/*
 * BIND: Double
 */
int sh_db_bind_double(sqlite3_stmt *pStmt, int index, double val);

/*
 * BIND: String
 * sqlite3_bind_text: UTF-8
 * sqlite3_bind_text16: UTF-16
 * sqlite3_bind_text64: specify encoding type. no need.
 * SQLITE_STATIC : constant string
 * SQLITE_TRANSIENT : changable in the near future
 */

int sh_db_bind_string(sqlite3_stmt *pStmt, int index, const char *str, int str_len);
/*
 * CURSOR FETCH POSSIBLE CHECK
 * ret: 0: 데이터 있음. 1: 데이터 없음. 그외 오류
 */
int sh_cur_fetch(sqlite3_stmt *pStmt);

/*
 * CURSOR CLOSE
 */
int sh_stmt_close(sqlite3_stmt *pStmt);

/*
 * START POINT OF COMMIT;
 */
int sh_transaction_begin(sqlite3 *pDb);

/*
 * END POINT OF COMMIT;
 */
int sh_transaction_end(sqlite3 *pDb);

/*
 * INSERT INTO TB_PAYINFO_STRING
 */
int sh_ins_data_string(sqlite3 *pDb, const char *val);

/*
 * insert 실행
 */
int sh_db_exec(sqlite3_stmt *pStmt);

void sh_db_commit(sqlite3 *db);
void sh_db_rollback(sqlite3 *db);


#endif
