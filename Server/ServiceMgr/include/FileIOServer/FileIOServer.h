#pragma once

#include <unordered_map>

#include "UVFile.h"

namespace msm {

class FileIOServer {
public:
    FileIOServer() = default;
    virtual ~FileIOServer() = default;

    bool Init(uv_loop_t* loop);
    bool UnInit();

    FILE_IO_RESULT Access(const char* path);
    FILE_IO_RESULT Open(const char* path, int flags);
    FILE_IO_RESULT WriteAsync(const char* path, const char* data, int len, int offset, UVFileCallback callback);
    int WriteSync(const char* path, const char* data, int len, int offset);
    FILE_IO_RESULT ReadAsync(const char* path, char* buf, int size, int offset, UVFileCallback callback);
    int ReadSync(const char* path, char* buf, int size, int offset);
    FILE_IO_RESULT Close(const char* path);

    uint64_t GetFileSize(const char* path); //-1��ʾʧ��

private:
    void _OnClose(const char* path);

    uv_loop_t* m_loop;

    std::unordered_map<std::string, UVFile*> m_openFiles;
};

}