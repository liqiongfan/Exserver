/**
 * Copyright @2019 Exserver All rights reserved
 */

#ifndef EX_STRING_H
#define EX_STRING_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define EX_STRL(s)     (s), (sizeof(s) - 1)
#define ex_tolower(c)  (( c >= 'A' && c <= 'Z' ) ? ( c | (char)0x20 )  : c)
#define ex_toupper(c)  (( c >= 'a' && c <= 'z' ) ? ( c & ~(char)0x20 ) : c)
#define ex_isdigit(c)  ( c - '0' >= 0 && c - '0' <= 9 )
#define ex_memzero(s, n)  memset(s, 0, n)
#define ex_copymem(d, s, n) memcpy(d, s, n)
#define ex_memcmp memcmp
#define ex_memfree(src) free((src))
#define ex_memset(dest, c, n) memset(dest, c, n)

char *exsubstr(const char *source, long start, ssize_t length, int mode);
char *extrim(const char *source, int mode);

long ex_strrncasestr(const char *str, const char *res, int rl);
long ex_strirncasestr(const char *str, long sp, const char *res, int rl);
long ex_get_cl(const char *s, long i);

long ex_strstr(const char *source, const char *str, size_t n);
long ex_strncasestr(const char *source, const char *str, size_t n);

void _ex_strncat_(char **dest, const char *source, size_t *origin_size, size_t *used_num);

#endif




