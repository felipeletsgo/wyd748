#include "../internal/render/mesh/EffectVertexColor.h"
#include <cstdio>
#include <cstring>

namespace
{
struct Vertex { float position[3]; unsigned int diffuse; float uv[2]; };
struct Descriptor { unsigned int Size = 0; unsigned int FVF = 0; };
struct Buffer
{
    Vertex vertices[2]{{{1, 2, 3}, 7, {4, 5}}, {{6, 7, 8}, 9, {10, 11}}};
    Descriptor desc{sizeof(vertices), 322};
    int getResult = 0, lockResult = 0, unlockResult = 0;
    int gets = 0, locks = 0, unlocks = 0;
    bool nullData = false;
    int GetDesc(Descriptor* out) { ++gets; *out = desc; return getResult; }
    int Lock(unsigned int, unsigned int, void** out, unsigned int)
    {
        ++locks;
        *out = nullData ? nullptr : vertices;
        return lockResult;
    }
    int Unlock() { ++unlocks; return unlockResult; }
};
bool Paint(Buffer* buffer, unsigned int stride = sizeof(Vertex))
{
    return effect_vertex_color::TrySetDiffuse<Vertex, Descriptor>(buffer, stride, 322, 0xFFAAAAAA);
}
}

int RunEffectVertexColorTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::printf("FAIL EffectVertexColor: %s\n", name); }
    };
    check(!Paint(nullptr), "null vertex buffer from crash is rejected");
    Buffer valid;
    Buffer original;
    check(Paint(&valid), "loaded buffer accepts color update");
    check(valid.gets == 1 && valid.locks == 1 && valid.unlocks == 1,
        "successful update balances lock exactly once");
    for (int i = 0; i < 2; ++i)
    {
        check(valid.vertices[i].diffuse == 0xFFAAAAAA, "all vertex colors updated");
        check(std::memcmp(valid.vertices[i].position, original.vertices[i].position,
            sizeof(Vertex::position)) == 0 &&
            std::memcmp(valid.vertices[i].uv, original.vertices[i].uv, sizeof(Vertex::uv)) == 0,
            "geometry and UVs preserved");
    }
    for (int fault = 0; fault < 8; ++fault)
    {
        Buffer buffer;
        unsigned int stride = sizeof(Vertex);
        switch (fault)
        {
        case 0: buffer.getResult = -1; break;
        case 1: buffer.desc.Size = 0; break;
        case 2: ++buffer.desc.Size; break;
        case 3: buffer.desc.FVF = 0; break;
        case 4: ++stride; break;
        case 5: buffer.lockResult = -1; break;
        case 6: buffer.nullData = true; break;
        case 7: buffer.unlockResult = -1; break;
        }
        check(!Paint(&buffer, stride), "device or layout failure reported");
        check(buffer.locks == (fault >= 5 ? 1 : 0), "invalid layout never locks");
        check(buffer.unlocks == (fault >= 6 ? 1 : 0), "only successful locks are unlocked");
        if (fault != 7)
            check(std::memcmp(buffer.vertices, original.vertices, sizeof(buffer.vertices)) == 0,
                "failure before write leaves buffer unchanged");
    }
    return failures;
}
