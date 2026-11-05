#pragma once

#include <cstddef>
#include <cstdint>

// Structural limits of the static .msa mesh format. The loader checks every
// count, size and range against them before it creates or locks a buffer.
// All 1,869 shipped files referenced by Mesh\MeshList.txt satisfy them.
namespace msa_load
{
constexpr std::uint32_t MaxAttributes = 32;
constexpr std::uint32_t ColorFvf = 322;        // XYZ | DIFFUSE | TEX1, loaded as stored
constexpr std::uint32_t AddedUvFvf = 256;      // other formats gain one UV set
constexpr std::uint32_t UvAnimatedFvf = 530;   // XYZ | NORMAL | TEX2 after loading
constexpr std::uint32_t AddedUvBytes = 8;
constexpr std::uint32_t UvAnimatedStride = 40; // RDVERTEX2
constexpr std::uint32_t MaxStride = 256;
constexpr std::size_t TextureFieldBytes = 11;

struct Range
{
    std::uint32_t attribId, faceStart, faceCount, vertexStart, vertexCount;
};
static_assert(sizeof(Range) == 20, "D3DXATTRIBUTERANGE layout");

inline std::uint32_t LoadedFvf(std::uint32_t storedFvf)
{
    return storedFvf == ColorFvf ? storedFvf : storedFvf + AddedUvFvf;
}

inline bool ValidHeader(std::uint32_t storedFvf, std::uint32_t stride, std::uint32_t attributes)
{
    if (attributes == 0 || attributes > MaxAttributes) return false;
    if (stride < 3 * sizeof(float) || stride % sizeof(float) || stride > MaxStride) return false;
    // UV animation rewrites RDVERTEX2 fields; that layout must be exact.
    if (LoadedFvf(storedFvf) == UvAnimatedFvf && stride + AddedUvBytes != UvAnimatedStride) return false;
    return true;
}

inline bool ValidIndexBytes(std::uint32_t bytes, std::uint64_t remainingFileBytes)
{
    return bytes != 0 && bytes % sizeof(std::uint16_t) == 0 && bytes <= remainingFileBytes;
}

struct VertexPlan
{
    std::uint32_t vertices = 0;    // complete stored vertices
    std::uint32_t readBytes = 0;   // bytes read from the file
    std::uint32_t bufferBytes = 0; // vertex-buffer size
    std::uint32_t loadedStride = 0;
    bool addUv = false;
};

// The bounds scan seeds from the second vertex, so a mesh needs two vertices.
inline bool PlanVertices(std::uint32_t storedFvf, std::uint32_t stride, std::uint32_t storedBytes,
    std::uint64_t remainingFileBytes, VertexPlan& plan)
{
    plan = {};
    if (stride == 0) return false;
    const std::uint32_t vertices = storedBytes / stride;
    if (vertices < 2) return false;
    const bool addUv = storedFvf != ColorFvf;
    const std::uint64_t readBytes = addUv ? std::uint64_t(vertices) * stride : storedBytes;
    const std::uint64_t bufferBytes = addUv
        ? std::uint64_t(storedBytes) + std::uint64_t(AddedUvBytes) * vertices
        : storedBytes;
    if (readBytes > remainingFileBytes || bufferBytes > 0xFFFFFFFFull) return false;
    plan.vertices = vertices;
    plan.readBytes = static_cast<std::uint32_t>(readBytes);
    plan.bufferBytes = static_cast<std::uint32_t>(bufferBytes);
    plan.loadedStride = addUv ? stride + AddedUvBytes : stride;
    plan.addUv = addUv;
    return true;
}

// Every attribute range must address loaded vertices and indices only.
inline bool RangesFit(const Range* ranges, std::uint32_t count, std::uint32_t vertices, std::uint32_t indices)
{
    if (!ranges || count > MaxAttributes) return false;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        const Range& range = ranges[i];
        if (std::uint64_t(range.vertexStart) + range.vertexCount > vertices) return false;
        if ((std::uint64_t(range.faceStart) + range.faceCount) * 3 > indices) return false;
    }
    return true;
}

// Copies source up to its first '.', or to its end, into a terminated
// destination. Returns false when the stem does not fit.
inline bool CopyStem(char* destination, std::size_t capacity, const char* source, std::size_t sourceBytes)
{
    if (!destination || capacity == 0 || !source) return false;
    std::size_t i = 0;
    for (; i < sourceBytes && source[i] != '\0' && source[i] != '.'; ++i)
    {
        if (i + 1 >= capacity)
        {
            destination[0] = '\0';
            return false;
        }
        destination[i] = source[i];
    }
    destination[i] = '\0';
    return true;
}

// UV animation writes tu2/tv2 of every vertex in every attribute range of a
// shared mesh. Only a loaded RDVERTEX2 mesh with a vertex buffer qualifies.
inline bool UvAnimatable(bool hasVertexBuffer, std::uint32_t loadedFvf, std::uint32_t loadedStride,
    std::uint32_t attributes)
{
    return hasVertexBuffer && loadedFvf == UvAnimatedFvf && loadedStride == UvAnimatedStride
        && attributes <= MaxAttributes;
}
}
