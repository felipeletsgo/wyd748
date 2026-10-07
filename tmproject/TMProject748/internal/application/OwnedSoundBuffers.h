#pragma once
#include <vector>

// Own partially constructed COM buffers until CSound takes all references.
template<class Buffer> class OwnedSoundBuffers
{
public:
    explicit OwnedSoundBuffers(unsigned int count) : buffers(count, nullptr) {}
    ~OwnedSoundBuffers() { for (auto* buffer : buffers) if (buffer) buffer->Release(); }
    Buffer** data() { return buffers.data(); }
    void transfer() { for (auto& buffer : buffers) buffer = nullptr; }
private:
    std::vector<Buffer*> buffers;
};
