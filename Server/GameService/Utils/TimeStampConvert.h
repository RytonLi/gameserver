#pragma once

#include <time.h>

//null ���� 0
//���ڸ�ʽ: 2022/11/18
time_t ConvertStringToTimeStamp(const char* data);

const char* ConvertTimeStampToString(time_t timeStamp);