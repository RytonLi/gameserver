#include "UVFile.h"
#include <string.h>

using namespace msm;

struct WriteFileReq {
	uv_fs_t req;
	uv_buf_t buf;
	UVFileCallback callback;
    UVFile* thisPtr;
};

WriteFileReq* _NewWriteFileReq(int size) {
	WriteFileReq* wr = nullptr;
	char* tmp = nullptr;

	tmp = (char*)malloc(size);
	if (!tmp) {
		goto Exit0;
	}

	wr = new WriteFileReq;

	wr->buf = uv_buf_init(tmp, size);

Exit0:
	return wr;
}

void _DeleteWriteFileReq(WriteFileReq* wr) {
	if (wr != nullptr) {
		free(wr->buf.base);
		delete wr;
	}
}

struct ReadFileReq {
	uv_fs_t req;
	uv_buf_t buf;
	UVFileCallback callback;
    UVFile* thisPtr;
};

bool UVFile::Init(uv_loop_t* loop, const std::string& path, my_fs_t* fd) {
    m_loop      = loop;
    m_path      = path;
    m_fd        = fd;
    m_isWriting = false;
    m_isReading = false;
    m_isClosing = false;
    return true;
}

bool UVFile::UnInit() {
    return true;
}

int UVFile::WriteSync(const char* data, int len, int offset) {
    int res = emFileIONull;
    WriteFileReq* wr = nullptr;

    if (m_isClosing) {
        res = -1;
        goto Exit0;
    }

    if (m_isWriting) {
        res = -2;
        goto Exit0;
    }

    wr = _NewWriteFileReq(len);
    if (!wr) {
        res = -3;
        goto Exit0;
    }

    memcpy(wr->buf.base, data, len);
    res = ::uv_fs_write(m_loop, (uv_fs_t*)wr, ((uv_fs_t*)m_fd)->result, &(wr->buf), 1, offset, NULL);
    _DeleteWriteFileReq(wr);

Exit0:
    return res;
}

int UVFile::ReadSync(char* buf, int len, int offset) {
    int res = emFileIONull;
    ReadFileReq* rd = nullptr;

    if (m_isClosing) {
        res = -1;
        goto Exit0;
    }

    if (m_isWriting) {
        res = -2;
        goto Exit0;
    }

    rd = new ReadFileReq;
    rd->buf = uv_buf_init(buf, len);
    res = uv_fs_read(m_loop, (uv_fs_t*)rd, ((uv_fs_t*)m_fd)->result, &(rd->buf), 1, offset, NULL);
    delete rd;

Exit0:
    return res;
}

FILE_IO_RESULT UVFile::WriteAsync(const char* data, int len, int offset, UVFileCallback callback) {
    FILE_IO_RESULT res = emFileIONull;
	WriteFileReq* wr = nullptr;
	int r;

    if (m_isClosing) {
        res = emFileIOIsClosing;
        goto Exit0;
    }

    if (m_isWriting) {
        res = emFileIOIsWriting;
        goto Exit0;
    }

	wr = _NewWriteFileReq(len);
	if (!wr) {
        res = emFileIOMallocFail;
		goto Exit0;
	}

	memcpy(wr->buf.base, data, len);
	wr->callback = callback;
    wr->thisPtr = this;
    m_isWriting = true;
    r = ::uv_fs_write(m_loop, (uv_fs_t*)wr, ((uv_fs_t*)m_fd)->result, &(wr->buf), 1, offset,
		[](uv_fs_t* req) {
			WriteFileReq* wr = (WriteFileReq*)req;
			wr->callback(req->result);
            wr->thisPtr->m_isWriting = false;
            wr->thisPtr->_AfterIOProcess();
            _DeleteWriteFileReq(wr);
		}
	);

	if (r) {
		_DeleteWriteFileReq(wr);
        m_isWriting = false;
        res = emFileIOWriteError;
		goto Exit0;
	}

    res = emFileIOSuccess;
Exit0:
	return res;
}

FILE_IO_RESULT UVFile::ReadAsync(char* buf, int len, int offset, UVFileCallback callback) {
    FILE_IO_RESULT res = emFileIONull;
    ReadFileReq* rd = nullptr;
    int r;

    if (m_isClosing) {
        res = emFileIOIsClosing;
        goto Exit0;
    }

    if (m_isWriting) {
        res = emFileIOIsWriting;
        goto Exit0;
    }

    rd = new ReadFileReq;
	rd->buf = uv_buf_init(buf, len);
	rd->callback = callback;
    rd->thisPtr = this;
    m_isReading = true;
    r = uv_fs_read(m_loop, (uv_fs_t*)rd, ((uv_fs_t*)m_fd)->result, &(rd->buf), 1, offset,
		[](uv_fs_t* req) {
			ReadFileReq* rd = (ReadFileReq*)req;
			rd->callback(req->result);
            rd->thisPtr->m_isReading = false;
            rd->thisPtr->_AfterIOProcess();
            delete rd;
        }
	);

    if (r) {
        delete rd;
        res = emFileIOReadError;
    }

    res = emFileIOSuccess;
Exit0:
    return res;
}

uint64_t UVFile::GetFileSize() {
    return m_fd->fd.statbuf.st_size;
}

void UVFile::Close(UVFileOnCloseCallback callback) {
    m_onCloseCallback = callback;
    if (m_isWriting || m_isReading) {
        m_isClosing = true;
    }
    else {
        _OnClose();
    }
}

my_fs_t* UVFile::Fd() {
    return m_fd;
}

void UVFile::_AfterIOProcess() {
    if (m_isClosing) {
        _OnClose();
    }
}

void UVFile::_OnClose() {
    if (m_onCloseCallback) {
        m_onCloseCallback(m_path);
    }
}