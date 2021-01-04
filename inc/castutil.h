#include "common.h"

int sh_stoi(const char *str, const int len, int *num);
int sh_stol(const char *str, const int len, long *num);

/*
 * pos: L(left) R(right)
 */
void itos_padding(int org, char *dest, const int dest_len, const char ch, const char pos);
void ltos_padding(long org, char *dest, const int dest_len, const char ch, const char pos);
void stos_padding(char *org, const int org_len, char *dest, const int dest_len, const char ch);

void stos_unpadding(const char *org, const int org_len, char *dest, const char ch, const char pos);
void stol_unpadding(const char *org, const int org_len, long *dest, const char ch, const char pos);
