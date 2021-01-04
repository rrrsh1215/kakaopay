#include "cfgutil.h"

/*
 * 번호 증가 0~9, A-Z, a-z
 */
int increaseNo(char *mng_no, const int idx)
{
    if (idx == 0 && mng_no[idx] == 'z') {
        fprintf(stderr, "[ERROR] 관리번호를 더이상 증가할 수 없습니다.\n");
        return -1;
    }

    if (mng_no[idx] == '9') {
        mng_no[idx] = 'A';

    } else if (mng_no[idx] == 'Z') {
        mng_no[idx] = 'a';

    } else if (mng_no[idx] == 'z') {
        mng_no[idx] = '0';
        increaseNo(mng_no, idx - 1);

    } else {
        mng_no[idx] += 1;
    }

    return 0;
}

/*
 * 관리번호 생성
 */
int sh_get_mng_no(char *mng_no, char *home)
{
	char szMng_no[D_SIZE_MNG_NO + 2]; // '\n' + '\0'
	char pathfile_mng_no[128 + 1];

	FILE *fp;

	memset(szMng_no, 0x00, sizeof(szMng_no));

	// 마지막 mng_no 조회
	sprintf(pathfile_mng_no, "%s/cfg/mng_no.dat", home);

	if ((fp = fopen(pathfile_mng_no, "r")) == NULL) {
		fprintf(stderr, "[ERROR]fopen() fail. pathfile_mng_no[%s] (%d:%m)\n", pathfile_mng_no, errno);
		return -1;
	}
	if (fgets(szMng_no, D_SIZE_MNG_NO + 1, fp) == NULL) {
		fprintf(stderr, "fgets() fail. (%d:%m)\n", errno);
		fclose(fp);
		return -1;
	}

	fclose(fp);
	fp = NULL;

	if (increaseNo(szMng_no, D_SIZE_MNG_NO - 1) < 0) return -1;

	memcpy(mng_no, szMng_no, strlen(szMng_no));

	if ((fp = fopen(pathfile_mng_no, "w")) == NULL) {
		fprintf(stderr, "[ERROR]fopen() fail. pathfile_mng_no[%s] (%d:%m)\n", pathfile_mng_no, errno);
		return -1;
	}

	szMng_no[D_SIZE_MNG_NO] = '\n';
	if (fwrite(szMng_no, D_SIZE_MNG_NO + 1, 1, fp) != 1) {
		fprintf(stderr, "[ERROR]fwrite() fail. szMng_no[%s]\n", szMng_no);
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int getHomeDir(char *home)
{
	char *p;
	if ((p = getenv("SH_HOME")) == NULL) {
		fprintf(stderr, "getenv() fail. SH_HOME (%d:%m)\n", errno);
		return -1;
	}
	strcpy(home, p);

	return 0;
}

int loadConfig(char *pathfile, char *field, void *value, const char type)
{
	int len;
	char buf[1024];
	FILE *fp;

	char *f, *v; // field, value

	f = buf;

	if ((fp = fopen(pathfile, "r")) == NULL) {
		fprintf(stderr, "[ERROR]fopen() fail. pathfile[%s] (%d:%m)\n", pathfile, errno);
		return -1;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (*f == '#' || *f == 0x20 || *f == '\t' || *f == 0x00) continue;

		buf[strlen(buf) - 1] = 0x00;

		if ((v = strchr(buf, '=')) == NULL) continue;

		if (memcmp(f, field, v - f) != 0) continue;

		v++;
		len = strlen(v);

		if (type == INT || type == LONG) {
			*(int *)value = atoi(v);
		} else if (type == CHAR) {
			*(char *)value = *v;
		} else if (type == STR) {
			memcpy((char *)value, v, len);
		} else if (type == FLOAT) {
			*(float *)value = atof(v);
		} else if (type == DOUBLE) {
			*(double *)value = atof(v);
		} else if (type == HEXA) {
			sscanf(v, "%x", (int *)value);
		} else {
			fclose(fp);
			fprintf(stderr, "[ERROR]Invalid type[%c]\n", type);
			return -2;
		}
		fclose(fp);

		return len;
	}

	fclose(fp);

	fprintf(stderr, "Invalid field[%s] pathfile[%s]\n", field, pathfile);
	return -1;
}

int getConfig(char *home, char *pathfile_db, int *timegap)
{
    char pathfile_cfg [128];
    char db_name      [64];

	memset(db_name      , 0x00, sizeof(db_name      ));

    if (getHomeDir(home) < 0) {
        return -1;
    }
    sprintf(pathfile_cfg, "%s/cfg/config.cfg", home);

    if (loadConfig(pathfile_cfg, "DB_NAME", db_name, STR) < 0) {
        return -1;
    }
    if (loadConfig(pathfile_cfg, "TIMEGAP", timegap, INT) < 0) {
        return -1;
    }
    sprintf(pathfile_db, "%s/database/%s", home, db_name);

	return 0;
}

