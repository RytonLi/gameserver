#pragma once

#include <time.h>

//null 返回 0
//日期格式: 2022/11/18
time_t ConvertStringToTimeStamp(const char* data);

const char* ConvertTimeStampToString(time_t timeStamp);