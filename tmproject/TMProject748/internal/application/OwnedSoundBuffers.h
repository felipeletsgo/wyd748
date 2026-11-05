#pragma once
#include <vector>

// Own partially constructed COM buffers until CSound takes all references.
template<class Buffer> class OwnedSoundBuffers
{
public:
    explicit OwnedSoundBuffers(unsigned int count) : buffers(count, nullptr) {}
    ~OwnedSoundBuffers() { for (auto* buffer : buffers) if (buffer) buffer->Release(); }
    // A08-05: each COM reference is released once; a copy would release twice.
    OwnedSoundBuffers(const OwnedSoundBuffers&) = delete;
    OwnedSoundBuffers& operator=(const OwnedSoundBuffers&) = delete;
    OwnedSoundBuffers(OwnedSoundBuffers&&) = delete;
    OwnedSoundBuffers& operator=(OwnedSoundBuffers&&) = delete;
    Buffer** data() { return buffers.data(); }
    void transfer() { for (auto& buffer : buffers) buffer = nullptr; }
private:
    std::vector<Buffer*> buffers;
};
