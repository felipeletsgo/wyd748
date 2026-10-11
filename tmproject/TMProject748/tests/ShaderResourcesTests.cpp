#include "../internal/platform/windows/ShaderResources.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace {
struct Resource {
    int& live;
    explicit Resource(int& count) : live(count) { ++live; }
    void Release() { --live; delete this; }
};
struct Buffer : Resource {
    std::vector<DWORD> words;
    UINT size;
    bool nullData = false;
    Buffer(int& count, UINT bytes) : Resource(count), words((bytes + 3) / 4), size(bytes) {}
    void* GetBufferPointer() { return nullData ? nullptr : words.data(); }
    UINT GetBufferSize() const { return size; }
};
struct File {
    std::vector<unsigned char> bytes;
    long long length = 8;
    int opens = 0, closes = 0, reads = 0;
    int chunk = 0, failRead = -1, closeResult = 0;
    bool missing = false;
    std::size_t position = 0;
    File() {
        const DWORD words[]{0xFFFE0101UL, 0x0000FFFFUL};
        bytes.resize(sizeof(words));
        std::memcpy(bytes.data(), words, sizeof(words));
    }
    int Open(const char*) { ++opens; return missing ? -1 : 42; }
    long long Length(int) const { return length; }
    int Close(int) { ++closes; return closeResult; }
    int Read(int, void* output, UINT requested) {
        if (reads++ == failRead) return -1;
        auto count = (std::min)(static_cast<std::size_t>(requested), bytes.size() - position);
        if (chunk > 0) count = (std::min)(count, static_cast<std::size_t>(chunk));
        if (count) std::memcpy(output, bytes.data() + position, count);
        position += count;
        return static_cast<int>(count);
    }
};

std::filesystem::path Asset(const std::string& name)
{
    char executable[MAX_PATH]{};
    GetModuleFileNameA(nullptr, executable, MAX_PATH);
    for (auto start : {std::filesystem::current_path(), std::filesystem::path(executable).parent_path()}) {
        for (int level = 0; level < 10 && !start.empty(); ++level, start = start.parent_path()) {
            for (const char* prefix : {"tmproject/client748/Shader", "client748/Shader"}) {
                auto candidate = start / prefix / name;
                if (std::filesystem::is_regular_file(candidate)) return candidate;
            }
        }
    }
    return {};
}

template<std::size_t N, class Check>
void TestBatch(Check check)
{
    int live = 0;
    Resource* current[N]{};
    for (unsigned int mask = 0; mask < (1U << N); ++mask) {
        for (std::size_t i = 0; i < N; ++i)
            if (mask & (1U << i)) current[i] = new Resource(live);
        check(shader_resources::Batch<Resource, N>::Complete(current) == (mask == (1U << N) - 1),
            "readiness requires every slot, not just the first");
        for (auto& value : current) { if (value) value->Release(); value = nullptr; }
    }
    for (auto& value : current) value = new Resource(live);
    for (std::size_t stop = 0; stop < N; ++stop) {
        Resource* snapshot[N];
        std::copy(std::begin(current), std::end(current), snapshot);
        {
            shader_resources::Batch<Resource, N> batch;
            // Include a non-null output returned with a failed creation result.
            for (std::size_t i = 0; i <= stop; ++i) *batch.Output(i) = new Resource(live);
            if (stop + 1 < N)
                check(!batch.Publish(current), "incomplete batch cannot publish");
            check(std::equal(std::begin(current), std::end(current), snapshot),
                "failed creation leaves all previous resources intact");
        }
        check(live == static_cast<int>(N), "failure releases all pending outputs exactly once");
        {
            shader_resources::Batch<Resource, N> retry;
            for (std::size_t i = 0; i < N; ++i) *retry.Output(i) = new Resource(live);
            check(retry.Complete() && retry.Publish(current), "retry publishes a complete batch");
            check(live == static_cast<int>(N), "publication releases replaced resources");
            check(!retry.Publish(current), "repeated publication preserves transferred resources");
        }
        check(live == static_cast<int>(N), "published resources survive local destruction");
    }
    for (auto& value : current) { value->Release(); value = nullptr; }
    check(live == 0, "finalization leaves no resource leaks");
}
}

int RunShaderResourcesTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::fprintf(stderr, "ShaderResources: %s\n", name); }
    };
    int live = 0;
    int allocationCalls = 0;
    const auto allocate = [&](UINT size, Buffer** output) {
        ++allocationCalls;
        *output = new Buffer(live, size);
        return S_OK;
    };
    for (long long length : {-1LL, 0LL, 1LL, 4LL, 7LL, 9LL, 11LL, 2147483648LL, 4294967296LL}) {
        File io;
        io.length = length;
        Buffer* output = nullptr;
        const int before = allocationCalls;
        check(FAILED(shader_resources::Read(io, "fixture", 0xFFFE0000UL, allocate, &output)) &&
            !output && live == 0 && allocationCalls == before && io.closes == 1 && io.reads == 0,
            "invalid, unaligned and oversized lengths fail before allocation and close the file");
    }
    for (int fault = 0; fault < 13; ++fault) {
        File io;
        if (fault == 0) io.missing = true;
        if (fault == 1) io.bytes.resize(4);
        if (fault == 2) io.bytes.push_back(0);
        if (fault == 3) io.failRead = 0;
        if (fault == 4) io.failRead = 1; // Failure checking unexpected trailing bytes.
        if (fault == 5) io.closeResult = -1;
        if (fault == 6) io.bytes[7] = 1;
        if (fault == 7) io.bytes[3] = 0; // Wrong shader stage.
        Buffer* output = nullptr;
        const auto failingAllocate = [&](UINT size, Buffer** buffer) -> HRESULT {
            if (fault == 8) return E_OUTOFMEMORY;
            if (fault == 9) return S_OK; // Defensive check for an invalid success output.
            *buffer = new Buffer(live, size);
            if (fault == 10) (*buffer)->nullData = true;
            if (fault == 11) (*buffer)->size = size - 4;
            if (fault == 12) return E_FAIL;
            return S_OK;
        };
        check(FAILED(shader_resources::Read(io, "fixture", 0xFFFE0000UL, failingAllocate, &output)) &&
            !output && live == 0 && io.closes == (io.missing ? 0 : 1),
            "read, EOF, growth, close, envelope and allocation failures roll back ownership");
    }
    for (int chunk = 0; chunk <= 8; ++chunk) {
        File io;
        io.chunk = chunk;
        shader_resources::Owned<Buffer> output;
        check(shader_resources::Read(io, "fixture", 0xFFFE0000UL, allocate, &output.value) == S_OK &&
            output.value && io.position == io.bytes.size() && io.closes == 1 &&
            std::memcmp(output.value->GetBufferPointer(), io.bytes.data(), io.bytes.size()) == 0,
            "complete and segmented reads preserve bytes and transfer ownership");
    }
    check(live == 0, "successful buffer ownership released");
    {
        File io;
        Buffer* output = nullptr;
        check(shader_resources::Read(io, nullptr, 0xFFFE0000UL, allocate, &output) == E_INVALIDARG &&
            shader_resources::Read(io, "", 0xFFFE0000UL, allocate, &output) == E_INVALIDARG &&
            shader_resources::Read(io, "fixture", 0xFFFE0000UL, allocate, static_cast<Buffer**>(nullptr)) == E_POINTER &&
            io.opens == 0, "invalid arguments do not open files");
    }
    // Read every installed shader through the production helper; do not rewrite assets.
    for (const auto& family : {std::pair<const char*, int>{"skinmesh", 8}, {"vseffect", 4}, {"pseffect", 6}}) {
        for (int index = 1; index <= family.second; ++index) {
            const auto path = Asset(std::string(family.first) + std::to_string(index) + ".bin");
            check(!path.empty(), "required runtime shader fixture exists");
            if (path.empty()) continue;
            std::ifstream input(path, std::ios::binary);
            File io;
            io.bytes = std::vector<unsigned char>(std::istreambuf_iterator<char>(input),
                std::istreambuf_iterator<char>());
            io.length = static_cast<long long>(io.bytes.size());
            shader_resources::Owned<Buffer> code;
            const DWORD stage = family.second == 6 ? 0xFFFF0000UL : 0xFFFE0000UL;
            check(shader_resources::Read(io, "runtime", stage, allocate, &code.value) == S_OK &&
                code.value && std::memcmp(code.value->GetBufferPointer(), io.bytes.data(), io.bytes.size()) == 0,
                "unchanged runtime shader accepted byte for byte");
        }
    }
    check(live == 0, "runtime buffers fully released");
    TestBatch<8>(check);
    TestBatch<4>(check);
    TestBatch<6>(check);
    return failures;
}
