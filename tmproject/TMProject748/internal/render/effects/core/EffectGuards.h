#pragma once

// Header-only guards for effect code. They have no device or scene
// dependency, so the tests run them with fake buffers and meshes.

namespace effect_vertex_lock
{
// Locks a whole vertex buffer and calls write(vertices, count). A null
// buffer, a failed GetDesc or Lock and a null lock pointer skip the write.
// Only a successful Lock is unlocked. count is Size / sizeof(Vertex), so the
// write never passes the end of the buffer. The vertex format is not checked:
// callers keep their established layout.
template <typename Vertex, typename Descriptor, typename Buffer, typename Writer>
bool TryWrite(Buffer* buffer, Writer&& write)
{
    if (!buffer)
        return false;

    Descriptor desc{};
    if (buffer->GetDesc(&desc) < 0)
        return false;

    void* data = nullptr;
    if (buffer->Lock(0, 0, &data, 0) < 0)
        return false;
    if (!data)
    {
        buffer->Unlock();
        return false;
    }

    write(static_cast<Vertex*>(data), static_cast<unsigned int>(desc.Size / sizeof(Vertex)));
    return buffer->Unlock() >= 0;
}

// Sets the diffuse color of every vertex.
template <typename Vertex, typename Descriptor, typename Buffer>
bool TryFillDiffuse(Buffer* buffer, unsigned int color)
{
    return TryWrite<Vertex, Descriptor>(buffer, [color](Vertex* vertices, unsigned int count) {
        for (unsigned int i = 0; i < count; ++i)
            vertices[i].diffuse = color;
    });
}
}

namespace effect_bounds
{
// Returns index when it selects one of count table rows, otherwise fallback.
constexpr int RowOr(int index, int count, int fallback)
{
    return index >= 0 && index < count ? index : fallback;
}
}

namespace effect_spark
{
constexpr int kMaxSparks = 5;

// Clamps the requested spark count to the fixed child arrays.
constexpr int ClampCount(int requested)
{
    return requested < 0 ? 0 : (requested > kMaxSparks ? kMaxSparks : requested);
}

// The parent must expire no later than its children: it moves them every
// frame through raw pointers. Zero keeps both infinite.
constexpr unsigned int ParentLifetime(unsigned int childLifetime)
{
    return childLifetime > 100 ? childLifetime - 100 : childLifetime;
}
}

namespace shade_mask
{
constexpr int kSize = 128;

constexpr bool Contains(int x, int y)
{
    return x >= 0 && x < kSize && y >= 0 && y < kSize;
}

// Reads the terrain mask height under a shade owner. A high cell (>= 11)
// takes the first lower neighbor (< 10) in +y, -y, +x, -x order. Returns
// false for a sample outside this block; neighbors outside it are skipped.
inline bool Sample(const char (*mask)[kSize], int x, int y, int& height)
{
    if (!mask || !Contains(x, y))
        return false;

    int value = mask[y][x];
    if (value >= 11)
    {
        constexpr int dx[4] = { 0, 0, 1, -1 };
        constexpr int dy[4] = { 1, -1, 0, 0 };
        for (int i = 0; i < 4; ++i)
        {
            const int nx = x + dx[i];
            const int ny = y + dy[i];
            if (Contains(nx, ny) && mask[ny][nx] < 10)
            {
                value = mask[ny][nx];
                break;
            }
        }
    }
    height = value;
    return true;
}
}

namespace effect_mesh_state
{
// Applies an instance texture to a shared mesh for one draw and restores the
// mesh's own texture index when the scope ends.
template <typename Mesh>
class TextureOverride
{
public:
    TextureOverride(Mesh* mesh, bool apply, int texture)
        : m_mesh(apply ? mesh : nullptr), m_saved(0)
    {
        if (m_mesh)
        {
            m_saved = m_mesh->m_nTextureIndex[0];
            m_mesh->m_nTextureIndex[0] = texture;
        }
    }
    ~TextureOverride()
    {
        if (m_mesh)
            m_mesh->m_nTextureIndex[0] = m_saved;
    }
    TextureOverride(const TextureOverride&) = delete;
    TextureOverride& operator=(const TextureOverride&) = delete;

private:
    Mesh* m_mesh;
    int m_saved;
};
}
