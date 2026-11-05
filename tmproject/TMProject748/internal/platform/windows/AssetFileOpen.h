#pragma once
#include "../../application/MissingAssetCache.h"
#include "PerfTrace.h"
#include <windows.h>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <io.h>

namespace asset_file {
// Resource loading runs on the render thread. Cache missing paths only;
// format, permission, allocation, and lost-device failures remain retryable.
inline missing_asset::Cache& Misses() { static missing_asset::Cache cache; return cache; }
inline int Open(const char* kind, const char* path) {
    if (!path || !Misses().ShouldTry(path, GetTickCount64())) return -1;
    perf_trace::ScopedIO timing(kind, path);
    const int handle = _open(path, _O_BINARY | _O_RDONLY);
    const int error = errno;
    timing.Complete(handle != -1);
    if (handle == -1 && error == ENOENT) Misses().Missing(path, GetTickCount64());
    return handle;
}
inline FILE* OpenFile(const char* kind, const char* path) {
    if (!path || !Misses().ShouldTry(path, GetTickCount64())) return nullptr;
    FILE* file = nullptr;
    perf_trace::ScopedIO timing(kind, path);
    const auto error = fopen_s(&file, path, "rb");
    timing.Complete(file != nullptr);
    if (!file && error == ENOENT) Misses().Missing(path, GetTickCount64());
    return file;
}
}
