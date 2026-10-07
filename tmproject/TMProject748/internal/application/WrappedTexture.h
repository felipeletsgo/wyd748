#pragma once

#include <cstddef>
#include <cstring>
#include <io.h>
#include <memory>
#include <new>

namespace wrapped_texture
{
inline bool ValidIndex(int index, int capacity) { return index >= 0 && index < capacity; }

inline bool PayloadLength(long length, bool dds, int& payload)
{
    // Keep allocation bounded before subtracting the wrapper. DDS needs its
    // full 128-byte header; TGA needs its 18-byte header before the footer.
    const int prefix = dds ? 1 : 4;
    if (length < prefix + (dds ? 128 : 18) || length > 64L * 1024 * 1024)
        return false;
    payload = static_cast<int>(length) - prefix;
    return true;
}

// Consumes and closes the descriptor on every path, including allocation and
// short-read failures. Successful data keeps the established wrapper decoding.
inline char* Read(int descriptor, bool dds, const char* footer, int& payload)
{
    struct Close { int fd; ~Close() { _close(fd); } } close{descriptor};
    if (!PayloadLength(_filelength(descriptor), dds, payload)) return nullptr;
    const int suffix = dds ? 0 : 18;
    std::unique_ptr<char[]> data(new (std::nothrow) char[payload + suffix]);
    if (!data) return nullptr;
    char prefix[4]{};
    const int prefixSize = dds ? 1 : 4;
    if (_read(descriptor, prefix, prefixSize) != prefixSize ||
        _read(descriptor, data.get(), payload) != payload) return nullptr;
    if (dds)
    {
        std::memcpy(data.get(), "DDS", 3);
        std::memcpy(data.get() + 84, data[84] == '2' ? "DXT1" : "DXT3", 4);
    }
    else std::memcpy(data.get() + payload, footer, suffix);
    return data.release();
}
}
