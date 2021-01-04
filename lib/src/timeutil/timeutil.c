#include <timeutil.h>
#include <castutil.h>
#include <time.h>
int sh_getdatetime(char *datetime, const int round)
{
	struct timespec ts;  // 
	struct tm tm;        //
	char point_number[round + 1];

	if (round < 0) {
		return -1;
	}
	memset(datetime, 0x00, 14 + round + 1);

	if (clock_gettime(CLOCK_REALTIME, &ts) == 1) {
		return -1;
	}

	if (localtime_r(&ts.tv_sec, &tm) == NULL) {
		return -1;
	}

	sprintf(datetime, "%4d%02d%02d %02d%02d%02d"
		, tm.tm_year + 1900
		, tm.tm_mon + 1
		, tm.tm_mday
		, tm.tm_hour
		, tm.tm_min
		, tm.tm_sec);

	if (round > 0) {
		// + 1 include null (automatically system add null);
		snprintf(point_number, round + 1, "%ld", ts.tv_nsec);
		strcat(datetime, point_number);
	}

	return 0;
}

char TimeGapAfterYN(char *bef, char *aft, const int gap)
{
	int bef_yyyy, bef_mm, bef_dd, bef_hh, bef_mi, bef_ss;
	int aft_yyyy, aft_mm, aft_dd, aft_hh, aft_mi, aft_ss;

	sh_stoi(bef +  0, 4, &bef_yyyy);
	sh_stoi(bef +  4, 2, &bef_mm  );
	sh_stoi(bef +  6, 2, &bef_dd  );
	sh_stoi(bef +  9, 2, &bef_hh  );
	sh_stoi(bef + 11, 2, &bef_mi  );
	sh_stoi(bef + 13, 2, &bef_ss  );

	sh_stoi(aft +  0, 4, &aft_yyyy);
	sh_stoi(aft +  4, 2, &aft_mm  );
	sh_stoi(aft +  6, 2, &aft_dd  );
	sh_stoi(aft +  9, 2, &aft_hh  );
	sh_stoi(aft + 11, 2, &aft_mi  );
	sh_stoi(aft + 13, 2, &aft_ss  );

	if (aft_yyyy*60*60*24*365 + aft_mm*60*60*24*12 + aft_dd*60*60*24 + aft_hh*60*60 + aft_mi*60 + aft_ss -
		(bef_yyyy*60*60*24*365 + bef_mm*60*60*24*12 + bef_dd*60*60*24 + bef_hh*60*60 + bef_mi*60 + bef_ss) > gap) {

		return 'Y';
	}

	return 'N';
}
