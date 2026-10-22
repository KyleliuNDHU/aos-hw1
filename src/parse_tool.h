#ifndef PARSE_TOOL_H
#define PARSE_TOOL_H

int parseInt(const char *s, int *out);
int parseLong(const char *s, long *out);
int parseDouble(const char *s, double *out);

int parseGen(const char *s, char **out);
int parseOut(const char *s, char **out);

#endif
