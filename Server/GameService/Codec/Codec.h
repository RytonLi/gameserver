#pragma once

#include "Packet.h"

GAME_SERVICE_NS_BEGIN

int check_pack(const char* data, int len);
// ���ﲻ�ٿ��ǰ�ȫ�����⣬ǰ��ĺ�����Ҫ���ϰ�ȫ
bool decode(Packet* pack, const char* data, int len);
// ����Ϣ���б��룬���ر����ĳ���
int encode(char* buff, uint16_t cmd, const char* data, int len);

GAME_SERVICE_NS_END