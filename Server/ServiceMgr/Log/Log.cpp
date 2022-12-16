#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <Windows.h>

using namespace std;

static char gs_buf[1024];
static int gs_len;

void LogLevel(int level) {
	gs_len = 0;
	switch (level)
	{
	case 0:
		
		gs_len = sprintf(gs_buf, "[debug]: ");
		break;
	case 1:
		gs_len = sprintf(gs_buf, "[erro]: ");
		break;
	default:
		gs_len = sprintf(gs_buf, "[info]: ");
		break;
	}
}

void LogTime() {
    SYSTEMTIME now;
    GetSystemTime(&now);

    gs_len += sprintf(gs_buf + gs_len, "%4d/%02d/%02d %02d:%02d:%02d:%03d, ",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);

    //timeb tb;
    //ftime(&tb);
    //struct tm now;
    //localtime_r(&tb.time, &now);
	
    //gs_len += sprintf(gs_buf + gs_len, "%4d/%02d/%02d %02d:%02d:%02d:%03d, ",
    //    now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, tb.millitm);
}

void LogLineAndFile(int lineno, const char* filename) {
	gs_len += sprintf(gs_buf + gs_len, "line: %d, file: %s, ", lineno, filename);
}


void Log(int level, int lineno, const char* filename, const char *msg, ...) {
	LogLevel(level);
	LogTime();
	LogLineAndFile(lineno, filename);

	va_list valst;
	va_start(valst, msg);

	gs_len += vsnprintf(gs_buf + gs_len, sizeof(gs_buf) - gs_len - 1, msg, valst);

	gs_buf[gs_len++] = '\n';
	gs_buf[gs_len++] = '\0';

	fprintf(stdout, gs_buf);

	va_end(valst);
}
