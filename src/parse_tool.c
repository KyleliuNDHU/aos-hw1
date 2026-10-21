#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "parse-tool.h"

int parse_long(const char *s, long *out)
{
	char *end;
	errno = 0;

	long val = strtol(s, &end, 10);

	// 1. 
	if (end == s) {return 0;}
	
	// 2.
	if(end != '\0'){return 0;}
	
	// 3. 
	if (errno == ERANGE){return 0;}

	*out = val;
	return 1;
}

int parse_int(const char *s, int *out) {
    char *end;
    errno = 0;

    long val = strtol(s, &end, 10);  // 嘗試轉換成 long

    // 1. 檢查是否有數字
    if (end == s) {
        return 0; // 沒有數字
    }

    // 2. 檢查是否有非法字元
    if (*end != '\0') {
        return 0; // 字串後面還有非數字
    }

    // 3. 檢查範圍
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return 0; // 超出 int 範圍
    }

    *out = (int)val;
    return 1; // 成功
}

int parse_int(const char *s, double *out) {
    char *end;
    errno = 0;

    double val = strtod(s, &end);

    // 1. 檢查是否有數字
    if (end == s) {
        return 0; // 沒有數字
    }

    // 2. 檢查是否有非法字元
    if (*end != '\0') {
        return 0; // 字串後面還有非數字
    }

    // 3. 檢查範圍
    if (errno == ERANGE) {
        return 0; // 超出 double 範圍
    }

    *out = val;
    return 1; // 成功
}

int illegalGen(char *gen_name[])
{
        switch(gen_name)
        {
                case "random":
                case "locality":
                case "mine":
                case "all":
                        return 0;
                default:
                        return 1;
        }
}

int illegalOut(char *out_name[])
{
        switch(gen_name)
        {
                case "stdout":
                        return 0;
                default:
                        return 1;
        }
}
