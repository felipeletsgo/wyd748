#pragma once

namespace effect_vertex_color
{
// Buffer/Descriptor are injectable so device failures exercise the production guard.
template<typename Vertex, typename Descriptor, typename Buffer>
bool TrySetDiffuse(Buffer* buffer, unsigned int stride, unsigned int expectedFVF,
    unsigned int color)
{
    if (!buffer || stride != sizeof(Vertex))
        return false;

    Descriptor desc{};
    if (buffer->GetDesc(&desc) < 0 || desc.FVF != expectedFVF ||
        desc.Size == 0 || desc.Size % sizeof(Vertex) != 0)
        return false;

    void* data = nullptr;
    if (buffer->Lock(0, 0, &data, 0) < 0)
        return false;
    if (!data)
    {
        buffer->Unlock();
        return false;
    }

    auto* vertices = static_cast<Vertex*>(data);
    for (unsigned int i = 0; i < desc.Size / sizeof(Vertex); ++i)
        vertices[i].diffuse = color;

    return buffer->Unlock() >= 0;
}
}
