#pragma once

#include <functional>
#include <string.h>

#include "Utils/Define.h"
#include "UVInclude/uv.h"

MY_SERVICE_MGR_NS_BEGIN

struct my_fs_t {
    uv_fs_t fd;

    my_fs_t() {
        memset(&fd, 0, sizeof(uv_fs_t));
    }
    ~my_fs_t() {
        ::uv_fs_req_cleanup(&fd);
    }
};

using UVFileCallback = std::function<void(ssize_t)>;
using UVFileOnCloseCallback = std::function<void(const std::string&)>;

enum FILE_IO_RESULT : uint8_t {
    emFileIONull       = 0,
    emFileIOSuccess    = 1,
    emFileIONotFound   = 2,
    emFileIOIsOpened   = 3,
    emFileIOIsWriting  = 4,
    emFileIOMallocFail = 5,
    emFileIOWriteError = 6,
    emFileIOReadError  = 7,
    emFileIOIsClosing  = 8,
    emFileIOOpenError  = 9,
};

class UVFile {
public:
    UVFile() = default;
    virtual ~UVFile() = default;

    bool Init(uv_loop_t* loop, const std::string& path, my_fs_t* fd);
    bool UnInit();

    int WriteSync(const char* data, int len, int offset);
    int ReadSync(char* buf, int len, int offset);

    FILE_IO_RESULT WriteAsync(const char* data, int len, int offset, UVFileCallback callback = nullptr);
    FILE_IO_RESULT ReadAsync(char* buf, int len, int offset, UVFileCallback callback = nullptr);

    uint64_t GetFileSize();

    void Close(UVFileOnCloseCallback callback);

    my_fs_t* Fd();

private:
    void _AfterIOProcess();
    void _OnClose();

    uv_loop_t*  m_loop;
    std::string m_path;
    my_fs_t*    m_fd;
    bool        m_isWriting;
    bool        m_isReading;
    bool        m_isClosing;

    UVFileOnCloseCallback m_onCloseCallback;
};

MY_SERVICE_MGR_NS_END
