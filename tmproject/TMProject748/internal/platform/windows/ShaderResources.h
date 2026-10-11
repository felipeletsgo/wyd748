#pragma once
#include <windows.h>
#include <cstddef>
#include <cstring>
#include <limits>

namespace shader_resources {
// Local ownership only; RenderDevice's public signatures and layout are unchanged.
template<class T>
class Owned {
public:
    T* value = nullptr;
    Owned() = default;
    Owned(const Owned&) = delete;
    Owned& operator=(const Owned&) = delete;
    ~Owned() { if (value) value->Release(); }
};

template<class T, std::size_t N>
class Batch {
    T* pending[N]{};
public:
    Batch() = default;
    Batch(const Batch&) = delete;
    Batch& operator=(const Batch&) = delete;
    ~Batch() { for (auto* value : pending) if (value) value->Release(); }
    T** Output(std::size_t index) { return &pending[index]; }
    static bool Complete(T* const (&current)[N]) {
        for (auto* value : current) if (!value) return false;
        return true;
    }
    bool Complete() const { return Complete(pending); }
    bool Publish(T* (&current)[N]) {
        if (!Complete()) return false;
        for (std::size_t i = 0; i < N; ++i) {
            if (current[i]) current[i]->Release();
            current[i] = pending[i];
            pending[i] = nullptr;
        }
        return true;
    }
};

// IO and allocation seams exercise the same read path without a graphics device.
// The envelope checks do not replace Direct3D's instruction/operand validation.
template<class IO, class Allocate, class Buffer>
HRESULT Read(IO& io, const char* path, DWORD stage, Allocate allocate, Buffer** output)
{
    if (!output) return E_POINTER;
    *output = nullptr;
    if (!path || !*path) return E_INVALIDARG;
    const int handle = io.Open(path);
    if (handle < 0) return E_FAIL;
    struct File {
        IO& io;
        int handle;
        ~File() { if (handle >= 0) io.Close(handle); }
    } file{io, handle};
    const auto length = io.Length(handle);
    if (length < 8 || length > (std::numeric_limits<int>::max)() || length % 4 != 0)
        return E_FAIL;
    const auto size = static_cast<UINT>(length);
    Owned<Buffer> code;
    const HRESULT allocated = allocate(size, &code.value);
    if (FAILED(allocated)) return allocated;
    if (!code.value || !code.value->GetBufferPointer() || code.value->GetBufferSize() != size)
        return E_FAIL;
    auto* bytes = static_cast<unsigned char*>(code.value->GetBufferPointer());
    UINT position = 0;
    while (position < size) {
        const int count = io.Read(handle, bytes + position, size - position);
        if (count <= 0 || static_cast<UINT>(count) > size - position) return E_FAIL;
        position += static_cast<UINT>(count);
    }
    unsigned char extra = 0;
    if (io.Read(handle, &extra, 1) != 0) return E_FAIL;
    const int closed = io.Close(handle);
    file.handle = -1;
    if (closed != 0) return E_FAIL;
    DWORD version = 0;
    DWORD end = 0;
    std::memcpy(&version, bytes, sizeof(version));
    std::memcpy(&end, bytes + size - sizeof(end), sizeof(end));
    if ((version & 0xFFFF0000UL) != stage || end != 0x0000FFFFUL) return E_FAIL;
    *output = code.value;
    code.value = nullptr;
    return S_OK;
}
}
