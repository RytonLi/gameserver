#include "FileIOServer.h"
#include "Log/Log.h"

using namespace std;
using namespace msm;

bool FileIOServer::Init(uv_loop_t* loop) {
    m_loop = loop;
    m_openFiles.clear();
    return true;
}

bool FileIOServer::UnInit() {
    return true;
}

FILE_IO_RESULT FileIOServer::Access(const char* path) {
    FILE_IO_RESULT res = emFileIONull;
    my_fs_t req;
    int tmp;

    if (m_openFiles.count(path)) {
        res = emFileIOSuccess;
        goto Exit0;
    }

    tmp = ::uv_fs_access(m_loop, (uv_fs_t*)&req, path, 0, NULL);
    res = tmp ? emFileIONotFound : emFileIOSuccess;
Exit0:
    return res;
}

FILE_IO_RESULT FileIOServer::Open(const char* path, int flags) {
    FILE_IO_RESULT res    = emFileIONull;
    my_fs_t*       fd     = nullptr;
    UVFile*        uvFile = nullptr;
    string         localPath;

    if (m_openFiles.count(path)) {
        LOG_DEBUG("file: %s open failed, file is opened", path);
        res = emFileIOIsOpened;
        goto Exit0;
    }
    
    fd = new my_fs_t;
    if (flags & O_CREAT) {
        if (::uv_fs_open(m_loop, (uv_fs_t*)fd, path, flags, S_IREAD | S_IWRITE, NULL) < 0) {
            LOG_DEBUG("file: %s open failed, uv_fs_open fail", path);
            res = emFileIOOpenError;
            goto Exit0;
        }
    }
    else {
        if (::uv_fs_stat(m_loop, (uv_fs_t*)fd, path, NULL) < 0 ||
            ::uv_fs_open(m_loop, (uv_fs_t*)fd, path, flags, S_IREAD | S_IWRITE, NULL) < 0) {
            LOG_DEBUG("file: %s open failed, uv_fs_open fail", path);
            res = emFileIONotFound;
            goto Exit0;
        }
    }
    

    uvFile = new UVFile;
    localPath = path;
    uvFile->Init(m_loop, localPath, fd);
    m_openFiles.insert(pair<string, UVFile*>(localPath, uvFile));
    res = emFileIOSuccess;
    fd  = nullptr;
Exit0:
    if (fd) {
        delete fd;
    }
    return res;
}

FILE_IO_RESULT FileIOServer::WriteAsync(const char* path, const char* data, int len, int offset, UVFileCallback callback) {
    FILE_IO_RESULT res = emFileIONull;
    auto it = m_openFiles.find(path);
    if (it == m_openFiles.end()) {
        res = emFileIONotFound;
        goto Exit0;
    }

    res = it->second->WriteAsync(data, len, offset, callback);

Exit0:
    return res;
}

int FileIOServer::WriteSync(const char* path, const char* data, int len, int offset) {
    int res;
    auto it = m_openFiles.find(path);
    if (it == m_openFiles.end()) {
        res = -4;
        goto Exit0;
    }

    res = it->second->WriteSync(data, len, offset);

Exit0:
    return res;
}

FILE_IO_RESULT FileIOServer::ReadAsync(const char* path, char* buf, int size, int offset, UVFileCallback callback) {
    FILE_IO_RESULT res = emFileIONull;
    auto it = m_openFiles.find(path);
    if (it == m_openFiles.end()) {
        res = emFileIONotFound;
        goto Exit0;
    }

    res = it->second->ReadAsync(buf, size, offset, callback);

Exit0:
    return res;
}

int FileIOServer::ReadSync(const char* path, char* buf, int size, int offset) {
    int res;
    auto it = m_openFiles.find(path);
    if (it == m_openFiles.end()) {
        res = -3;
        goto Exit0;
    }

    res = it->second->ReadSync(buf, size, offset);

Exit0:
    return res;
}

uint64_t FileIOServer::GetFileSize(const char* path) {
    uint64_t res;
    auto it = m_openFiles.find(path);
    if (it != m_openFiles.end()) {
        res = it->second->GetFileSize();
    }
    else {
        my_fs_t req;
        if (::uv_fs_stat(m_loop, (uv_fs_t*)&req, path, NULL)) {
            res = -1;
        }
        else {
            res = req.fd.statbuf.st_size;
        }
    }

    return res;
}

FILE_IO_RESULT FileIOServer::Close(const char* path) {
    FILE_IO_RESULT res = emFileIONull;
    auto it = m_openFiles.find(path);
    if (it == m_openFiles.end()) {
        res = emFileIONotFound;
        goto Exit0;
    }

    it->second->Close(
        bind(&FileIOServer::_OnClose, this, placeholders::_1)
    );

Exit0:
    return res;
}

void FileIOServer::_OnClose(const std::string& path) {
    my_fs_t req;
    UVFile* uvFile = nullptr;
    my_fs_t* fd = nullptr;

    auto it = m_openFiles.find(path);
    if (it == m_openFiles.end()) {
        goto Exit0;
    }

    uvFile = it->second;
    fd = uvFile->Fd();

    if (::uv_fs_close(m_loop, (uv_fs_t*)&req, ((uv_fs_t*)fd)->result, NULL) < 0) {
        LOG_ERROR("on close file error, path: %s", path);
    }

    delete fd;
    delete uvFile;
    m_openFiles.erase(it);

Exit0:
    return;
}