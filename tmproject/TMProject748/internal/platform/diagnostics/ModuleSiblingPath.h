#pragma once

#include <cstddef>
#include <cstring>

namespace diagnostics_path
{
    // Writes the directory of modulePath followed by fileName to out. Returns
    // false, with out empty, when modulePath has no directory or the result
    // does not fit capacity; no CRT invalid-parameter handler is reached.
    inline bool ReplaceFileName(const char* modulePath, const char* fileName,
        char* out, std::size_t capacity)
    {
        if (!out || capacity == 0)
            return false;
        out[0] = '\0';
        if (!modulePath || !fileName)
            return false;

        const char* backSlash = std::strrchr(modulePath, '\\');
        const char* forwardSlash = std::strrchr(modulePath, '/');
        const char* slash = !backSlash || (forwardSlash && forwardSlash > backSlash)
            ? forwardSlash : backSlash;
        if (!slash)
            return false;

        const std::size_t directory = static_cast<std::size_t>(slash - modulePath) + 1;
        const std::size_t name = std::strlen(fileName);
        if (directory >= capacity || name >= capacity - directory)
            return false;

        std::memcpy(out, modulePath, directory);
        std::memcpy(out + directory, fileName, name);
        out[directory + name] = '\0';
        return true;
    }
}
