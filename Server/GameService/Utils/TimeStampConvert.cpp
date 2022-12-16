#include "TimeStampConvert.h"
#include <stdio.h>

static char gs_timeBuff[25];

time_t ConvertStringToTimeStamp(const char* data) {
    if (data != nullptr) {
        int year;
        int month;
        int day;

        int res = sscanf(data, "%d/%d/%d", &year, &month, &day);

        if (res  < 3) {
            return 0;
        }
        else {
            struct tm timeInfo;
            timeInfo.tm_year = year - 1900;
            timeInfo.tm_mon = month - 1;
            timeInfo.tm_mday = day;
            timeInfo.tm_hour = 0;
            timeInfo.tm_min = 0;
            timeInfo.tm_sec = 0;
            return mktime(&timeInfo);
        }
    }
    else {
        return 0;
    }
    
}

const char* ConvertTimeStampToString(time_t timeStamp) {
    if (timeStamp != 0) {
        struct tm* timeInfo;
        timeInfo = localtime(&timeStamp);
        sprintf(gs_timeBuff, "%d/%d/%d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday);
        return gs_timeBuff;
    }
    else {
        return nullptr;
    }
}