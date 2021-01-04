#include "castutil.h"

int sh_stoi(const char *str, const int len, int *num)
{
    int i, j;

    if (len < 1) {
        return -1;
    }

    *num = 0;
    for (i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;
        }
        if (str[i] != '0') break;
    }

    if (i < len) {
        *num = str[i] - '0';

        for (j = i + 1; j < len; j++) {
            if (str[i] < '0' || str[i] > '9') {
                return -1;
            }

            *num = *num * 10 + (str[j] - '0');
        }
    }

    return 0;
}

int sh_stol(const char *str, const int len, long *num)
{
    int i, j;

    if (len < 1) {
        return -1;
    }

    *num = 0;
    for (i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;
        }
        if (str[i] != '0') break;
    }

    if (i < len) {
        *num = str[i] - '0';

        for (j = i + 1; j < len; j++) {
            if (str[i] < '0' || str[i] > '9') {
                return -1;
            }

            *num = *num * 10 + (str[j] - '0');
        }
    }

    return 0;
}

/*
 * pos: L(left) R(right)
 */
void itos_padding(int org, char *dest, const int dest_len, const char ch, const char pos)
{
    int strtmp_len;
    char strtmp [dest_len + 1];

    memset(strtmp , 0x00, sizeof(strtmp));

    sprintf(strtmp, "%d", org);
    strtmp_len = strlen(strtmp);

    if (pos == 'R') {
        memset(strtmp + strtmp_len, ch, dest_len - strtmp_len);

        memcpy(dest, strtmp, dest_len);
    } else if (pos == 'L') {
        char strtmp2[dest_len + 1];
        memset(strtmp2, ch, sizeof(strtmp2));
        strtmp2[dest_len] = 0x00;

        memcpy(strtmp2 + dest_len - strtmp_len, strtmp, strtmp_len);

        memcpy(dest, strtmp2, dest_len);
    }

    return;
}

void ltos_padding(long org, char *dest, const int dest_len, const char ch, const char pos)
{
    int strtmp_len;
    char strtmp [dest_len + 1];

    memset(strtmp , 0x00, sizeof(strtmp));

    sprintf(strtmp, "%ld", org);
    strtmp_len = strlen(strtmp);

    if (pos == 'R') {
        memset(strtmp + strtmp_len, ch, dest_len - strtmp_len);

        memcpy(dest, strtmp, dest_len);
    } else if (pos == 'L') {
        char strtmp2[dest_len + 1];
        memset(strtmp2, ch, sizeof(strtmp2));
        strtmp2[dest_len] = 0x00;

        memcpy(strtmp2 + dest_len - strtmp_len, strtmp, strtmp_len);

        memcpy(dest, strtmp2, dest_len);
    }

    return;
}

void stos_padding(char *org, const int org_len, char *dest, const int dest_len, const char ch)
{
    if (org_len > dest_len) {
        memcpy(dest, org, org_len);
    } else {
        memset(dest, ch, dest_len);
        memcpy(dest, org, org_len);
    }
    return;
}

void stos_unpadding(const char *org, const int org_len, char *dest, const char ch, const char pos)
{
    int i;
    char strtmp[org_len + 1];

    memset(strtmp, 0x00, sizeof(strtmp));

    memcpy(strtmp, org, org_len);

    if (pos == 'L') {
        for (i = 0; i < org_len; i++) {
            if (strtmp[i] != ch) break;
        }

        memcpy(dest, strtmp + i, org_len - i);
    } else if (pos == 'R') {
        for (i = org_len - 1; i >= 0; i--) {
            if (strtmp[i] != ch) break;
        }

        memcpy(dest, strtmp, i + 1);
    }

    return;
}

void stol_unpadding(const char *org, const int org_len, long *dest, const char ch, const char pos)
{
    int i;
    char strtmp [org_len + 1];
    char strtmp2[org_len + 1];

    memset(strtmp , 0x00, sizeof(strtmp ));
    memset(strtmp2, 0x00, sizeof(strtmp2));

    memcpy(strtmp, org, org_len);

    if (pos == 'L') {
        for (i = 0; i < org_len; i++) {
            if (strtmp[i] != ch) break;
        }

        memcpy(strtmp2, strtmp + i, org_len - i);
        *dest = atol(strtmp2);
    } else if (pos == 'R') {
        for (i = org_len - 1; i >= 0; i--) {
            if (strtmp[i] != ch) break;
        }

        memcpy(strtmp2, strtmp, i + 1);
        *dest = atol(strtmp2);
    }

    return;
}
