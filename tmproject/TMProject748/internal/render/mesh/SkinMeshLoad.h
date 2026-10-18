#pragma once

#include <cstdint>
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>

namespace skin_mesh_load
{
struct Header
{
    std::uint32_t parent, id, fvf, stride, influences, palette, vertices, indices;
};
static_assert(sizeof(Header) == 32, "skin mesh header contains eight DWORDs");

struct Payload
{
    std::uint32_t matrices = 0, names = 0, vertices = 0, indices = 0;
};

inline bool Plan(const Header& header, std::uint64_t fileBytes,
    std::size_t paletteCapacity, Payload& output)
{
    output = {};
    if (header.palette > paletteCapacity ||
        (header.palette && (header.influences < 1 || header.influences > 4)) ||
        header.stride < 3 * sizeof(float) || header.stride % sizeof(float) ||
        !header.vertices || !header.indices || header.indices % 3)
        return false;
    const std::uint64_t matrices = std::uint64_t(header.palette) * 64;
    const std::uint64_t names = std::uint64_t(header.palette) * 4;
    const std::uint64_t vertices = std::uint64_t(header.vertices) * header.stride;
    const std::uint64_t indices = std::uint64_t(header.indices) * 2;
    const auto limit = (std::numeric_limits<std::uint32_t>::max)();
    if (matrices > limit || names > limit || vertices > limit || indices > limit ||
        sizeof(Header) + matrices + names + vertices + indices > fileBytes)
        return false;
    output = {static_cast<std::uint32_t>(matrices), static_cast<std::uint32_t>(names),
        static_cast<std::uint32_t>(vertices), static_cast<std::uint32_t>(indices)};
    return true;
}

inline bool ReadExact(FILE* file, void* destination, std::size_t bytes)
{
    return file && (!bytes || (destination && std::fread(destination, 1, bytes, file) == bytes));
}

inline bool Positions(const void* data, const Header& header)
{
    const auto* vertices = static_cast<const float*>(data);
    if (!vertices) return false;
    const std::size_t stride = header.stride / sizeof(float);
    for (std::size_t i = 0; i < header.vertices; ++i)
        for (std::size_t axis = 0; axis < 3; ++axis)
            if (!std::isfinite(vertices[stride * i + axis])) return false;
    return true;
}

inline bool Indices(const void* data, const Header& header)
{
    const auto* indices = static_cast<const std::uint16_t*>(data);
    if (!indices) return false;
    for (std::size_t i = 0; i < header.indices; ++i)
        if (indices[i] >= header.vertices) return false;
    return true;
}

template<class Buffer> struct Release
{
    void operator()(Buffer* buffer) const { if (buffer) buffer->Release(); }
};

// Own the temporary COM reference until creation, read, validation and unlock succeed.
template<class Buffer, class Create, class Validate>
bool ReadBuffer(FILE* file, std::uint32_t bytes, Buffer*& output, Create create, Validate validate)
{
    if (!file || !bytes || output) return false;
    Buffer* raw = nullptr;
    const auto created = create(&raw);
    std::unique_ptr<Buffer, Release<Buffer>> pending(raw);
    if (created < 0 || !pending) return false;
    void* data = nullptr;
    if (pending->Lock(0, bytes, &data, 0) < 0) return false;
    const bool read = data && ReadExact(file, data, bytes) && validate(data);
    const auto unlocked = pending->Unlock();
    if (!read || unlocked < 0) return false;
    output = pending.release();
    return true;
}
}
