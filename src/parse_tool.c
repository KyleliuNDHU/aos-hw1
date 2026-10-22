#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "parse_tool.h"

int parseLong(const char *s, long *out)
{
	char *end;
	errno = 0;

	long val = strtol(s, &end, 10);	// base 10; `end` points to the first char not consumed

	// 1. no digits at all (e.g. "" or "abc")
	if (end == s) 
	{
		fprintf(stderr, "parseLong error: 1. No digits\n");
		return 0;
	}
	
	// 2. trailing garbage (e.g. "12x")
	if(*end != '\0')
	{
		fprintf(stderr, "parseLong error: 2. Illegal characters\n");
		return 0;
	}
	
	// 3. overflow: strtol sets errno = ERANGE
	if (errno == ERANGE)
	{
		fprintf(stderr, "parseLong error: 3. Out of range\n");
		return 0;
	}

	*out = val;
	return 1;
}

int parseInt(const char *s, int *out) {
    char *end;
    errno = 0;

    long val = strtol(s, &end, 10);  // 嘗試轉換成 long

    // 1. 檢查是否有數字
    if (end == s) {
	fprintf(stderr, "parseInt error: 1. No digits\n");
        return 0; // 沒有數字
    }

    // 2. 檢查是否有非法字元
    if (*end != '\0') {
	fprintf(stderr, "parseInt error: 2. Illegal characters\n");
        return 0; // 字串後面還有非數字
    }

    // 3. 檢查範圍
    if (val > INT_MAX || val < INT_MIN) {
	fprintf(stderr, "parseInt error: 3. Out of range\n");
      	return 0; // 超出 int 範圍
    }

    *out = (int)val;
    return 1; // 成功
}

int parseDouble(const char *s, double *out) {
    char *end;

    double val = strtod(s, &end);

    // 1. 檢查是否有數字
    if (end == s) {
	fprintf(stderr, "parseDouble error: 1. No digits\n");
        return 0; // 沒有數字
    }

    // 2. 檢查是否有非法字元
    if (*end != '\0') {
	fprintf(stderr, "parseDouble error: 2. Illegal characters\n");
        return 0; // 字串後面還有非數字
    }

    // 3. 檢查範圍：pwrite 是機率，只接受 0.0 ~ 1.0
    if (val < 0.0 || val > 1.0) {
	fprintf(stderr, "parseDouble error: 3. Out of range (need 0 <= P <= 1)\n");
        return 0; // 不在 [0, 1]
    }

    *out = val;
    return 1; // 成功
}

/* --gen only accepts the three generator names; the string itself is kept
 * (points into argv, which lives for the whole program). */
int parseGen(const char *s, const char **out)
{
	if(strcmp(s, "random") && strcmp(s, "locality") && strcmp(s, "mine"))
	{
		fprintf(stderr, "argument error (gen): can't recognize this %s\n", s);
		return 0;
	}
        *out = s;
	return 1;
}

/* --out takes any path; the file is opened later in runExperiment. */
int parseOut(const char *s, const char **out)
{
	if (s == NULL){fprintf(stderr, "parseOut Error: no file to write in\n"); return 0;}
	*out = s;
	return 1;
}
