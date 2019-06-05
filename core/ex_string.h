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
#define ex_str1cmp(d, s)   ((d)[0] == s[0])
#define ex_str2cmp(d, s)   ((d)[0] == s[0] && (d)[1] == s[1])
#define ex_str3cmp(d, s)   ((d)[0] == s[0] && (d)[1] == s[1] && (d)[2] == s[2])
#define ex_str4cmp(d, s)   ((d)[0] == s[0] && (d)[1] == s[1] && (d)[2] == s[2] && (d)[3] == s[3])
#define ex_str5cmp(d, s)   ((d)[0] == s[0] && (d)[1] == s[1] && (d)[2] == s[2] && (d)[3] == s[3] && (d)[4] == s[4])
#define ex_str6cmp(d, s)   ((d)[0] == s[0] && (d)[1] == s[1] && (d)[2] == s[2] && (d)[3] == s[3] && (d)[4] == s[4] && (d)[5] == s[5])
#define ex_str7cmp(d, s)   ((d)[0] == s[0] && (d)[1] == s[1] && (d)[2] == s[2] && (d)[3] == s[3] && (d)[4] == s[4] && (d)[5] == s[5] && (d)[6] == s[6])

#define ex_str8ncmp(d, s) ( ex_tolower((d)[0]) == s[0] && ex_tolower((d)[1]) == s[1] && ex_tolower((d)[2]) == s[2] && ex_tolower((d)[3]) == s[3] &&\
							 ex_tolower((d)[4]) == s[4] && ex_tolower((d)[5]) == s[5] && ex_tolower((d)[6]) == s[6] && ex_tolower((d)[7]) == s[7] )

#define ex_str9ncmp(d, s) ( ex_tolower((d)[0]) == s[0] && ex_tolower((d)[1]) == s[1] && ex_tolower((d)[2]) == s[2] && ex_tolower((d)[3]) == s[3] &&\
							 ex_tolower((d)[4]) == s[4] && ex_tolower((d)[5]) == s[5] && ex_tolower((d)[6]) == s[6] && ex_tolower((d)[7]) == s[7] && \
							 ex_tolower((d)[8]) == s[8] )

#define ex_str10ncmp(d, s) ( ex_tolower((d)[0]) == s[0] && ex_tolower((d)[1]) == s[1] && ex_tolower((d)[2]) == s[2] && ex_tolower((d)[3]) == s[3] &&\
							 ex_tolower((d)[4]) == s[4] && ex_tolower((d)[5]) == s[5] && ex_tolower((d)[6]) == s[6] && ex_tolower((d)[7]) == s[7] && \
							 ex_tolower((d)[8]) == s[8] && ex_tolower((d)[9]) == s[9] )

#define ex_str11ncmp(d, s) ( ex_tolower((d)[0]) == s[0] && ex_tolower((d)[1]) == s[1] && ex_tolower((d)[2]) == s[2] && ex_tolower((d)[3]) == s[3] &&\
							 ex_tolower((d)[4]) == s[4] && ex_tolower((d)[5]) == s[5] && ex_tolower((d)[6]) == s[6] && ex_tolower((d)[7]) == s[7] && \
							 ex_tolower((d)[8]) == s[8] && ex_tolower((d)[9]) == s[9] && ex_tolower((d)[10]) == s[10] )

#define ex_str12ncmp(d, s) ( ex_tolower((d)[0]) == s[0] && ex_tolower((d)[1]) == s[1] && ex_tolower((d)[2]) == s[2] && ex_tolower((d)[3]) == s[3] &&\
							 ex_tolower((d)[4]) == s[4] && ex_tolower((d)[5]) == s[5] && ex_tolower((d)[6]) == s[6] && ex_tolower((d)[7]) == s[7] && \
							 ex_tolower((d)[8]) == s[8] && ex_tolower((d)[9]) == s[9] && ex_tolower((d)[10]) == s[10] && ex_tolower((d)[11]) == s[11] )

#define ex_str13ncmp(d, s) ( ex_tolower((d)[0]) == s[0] && ex_tolower((d)[1]) == s[1] && ex_tolower((d)[2]) == s[2] && ex_tolower((d)[3]) == s[3] &&\
							 ex_tolower((d)[4]) == s[4] && ex_tolower((d)[5]) == s[5] && ex_tolower((d)[6]) == s[6] && ex_tolower((d)[7]) == s[7] && \
							 ex_tolower((d)[8]) == s[8] && ex_tolower((d)[9]) == s[9] && ex_tolower((d)[10]) == s[10] && ex_tolower((d)[11]) == s[11] && \
							 ex_tolower((d)[12]) == s[12] )

char *exsubstr(const char *source, long start, ssize_t length, int mode);
char *extrim(const char *source, int mode);

long ex_strrncasestr(const char *str, const char *res, int rl);
long ex_strirncasestr(const char *str, long sp, const char *res, int rl);
long ex_get_cl(const char *s, long i);

long ex_strstr(const char *source, const char *str, size_t n);
long ex_strncasestr(const char *source, const char *str, size_t n);

void _ex_strncat_(char **dest, const char *source, size_t *origin_size, size_t *used_num);

#endif




