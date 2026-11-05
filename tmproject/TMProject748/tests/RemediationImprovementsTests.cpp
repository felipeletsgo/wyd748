#include "../internal/core/TreeCleanup.h"
#include "../internal/platform/windows/AdapterQuirks.h"
#include "../internal/render/effects/core/EffectVisualMath.h"
#include "../internal/ui/UnsupportedItemFeatures.h"
#include "../internal/wire/ReceivedPacketDispatch.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::filesystem::path Find(const char* relative)
{
    for (auto path = std::filesystem::current_path(); !path.empty(); path = path.parent_path()) {
        if (std::filesystem::exists(path / relative)) return path / relative;
        if (path == path.parent_path()) break;
    }
    return {};
}

// Exercise the production traversal with real allocation, unlinking and peer
// destruction. The intrusive link and destructor rules match TreeNode.
struct Node {
    Node* m_pTop = nullptr;
    Node* m_pDown = nullptr;
    Node* m_pNextLink = nullptr;
    Node* previous = nullptr;
    Node* deletePeer = nullptr;
    Node* markPeer = nullptr;
    bool m_cDeleted = false;
    bool* requested = nullptr;
    int id = 0;
    inline static unsigned long long destroyed = 0;
    inline static int live = 0;
    explicit Node(int value = 0) : id(value) { ++live; }
    ~Node() {
        if (markPeer) {
            markPeer->m_cDeleted = true;
            if (requested) *requested = true;
        }
        delete deletePeer;
        while (m_pDown) delete m_pDown;
        if (previous) previous->m_pNextLink = m_pNextLink;
        else if (m_pTop) m_pTop->m_pDown = m_pNextLink;
        if (m_pNextLink) m_pNextLink->previous = previous;
        ++destroyed;
        --live;
    }
    Node* Add(int value, bool deleted = false) {
        auto node = new Node(value);
        node->m_pTop = this;
        node->m_cDeleted = deleted;
        if (!m_pDown) m_pDown = node;
        else {
            auto tail = m_pDown;
            while (tail->m_pNextLink) tail = tail->m_pNextLink;
            tail->m_pNextLink = node;
            node->previous = tail;
        }
        return node;
    }
};

std::size_t Sweep(Node* root)
{
    return tree_cleanup::Sweep(root, [] { return Node::destroyed; }, [](Node* node) { delete node; });
}

// Retain the previous restart-on-delete algorithm only as a comparison oracle.
std::size_t OldSweep(Node* root)
{
    std::size_t visits = 0;
    Node* node = root->m_pDown;
    while (node && node != root) {
        ++visits;
        if (node->m_cDeleted) {
            auto parent = node->m_pTop;
            delete node;
            node = parent;
            if (node == root) node = root->m_pDown;
        } else if (node->m_pDown) node = node->m_pDown;
        else {
            while (node && node != root && !node->m_pNextLink) node = node->m_pTop;
            if (node && node != root) node = node->m_pNextLink;
        }
    }
    return visits;
}

unsigned int Word(const std::vector<char>& bytes, std::size_t offset)
{
    return static_cast<unsigned char>(bytes[offset]) |
        (static_cast<unsigned int>(static_cast<unsigned char>(bytes[offset + 1])) << 8);
}
void PutWord(std::vector<char>& bytes, std::size_t offset, unsigned int value)
{
    bytes[offset] = static_cast<char>(value & 255);
    bytes[offset + 1] = static_cast<char>((value >> 8) & 255);
}
}

int RunRemediationImprovementsTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const std::string& name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", name.c_str()); }
    };
    check(Sweep(nullptr) == 0, "null tree has no work");
    {
        Node root;
        root.m_cDeleted = true;
        root.Add(1, true);
        const auto destroyed = Node::destroyed;
        check(Sweep(&root) == 1 && Node::destroyed == destroyed + 1 && !root.m_pDown,
            "cleanup never deletes its scene root even when the root is marked for retirement");
    }
    {
        Node root;
        check(Sweep(&root) == 0, "empty tree has no work");
        auto a = root.Add(1);
        auto removed = root.Add(2, true);
        auto peer = root.Add(3);
        auto survivor = root.Add(4);
        removed->deletePeer = peer;
        check(Sweep(&root) > 0 && root.m_pDown == a && a->m_pNextLink == survivor &&
            survivor->previous == a && !survivor->m_pNextLink && Node::live == 3,
            "peer destructor invalidates the saved next link without use after free");
        auto branch = a->Add(5, true);
        branch->Add(6)->Add(7);
        a->Add(8);
        check(Sweep(&root) > 0 && a->m_pDown && a->m_pDown->id == 8 && Node::live == 4,
            "a deleted subtree is pruned and surviving branches remain linked");
    }
    check(Node::live == 0, "subtree and peer cases release every allocation");
    {
        Node root;
        auto earlier = root.Add(1);
        auto later = root.Add(2, true);
        bool requested = false;
        later->markPeer = earlier;
        later->requested = &requested;
        Sweep(&root);
        check(requested && root.m_pDown == earlier && earlier->m_cDeleted,
            "a destructor can request cleanup of an already visited peer");
        Sweep(&root);
        check(!root.m_pDown, "the next cleanup consumes the newly marked peer");
    }
    {
        Node root;
        root.Add(1, true);
        root.Add(2, true);
        Node::destroyed = std::numeric_limits<unsigned long long>::max() - 1;
        check(Sweep(&root) == 2 && !root.m_pDown && Node::destroyed == 0,
            "destruction deltas remain valid when the counter wraps");
    }
    {
        constexpr int count = 4096;
        Node oldRoot, newRoot;
        for (int i = 0; i < count; ++i) {
            oldRoot.Add(i, i % 2 != 0);
            newRoot.Add(i, i % 2 != 0);
        }
        const auto start = std::chrono::steady_clock::now();
        const auto oldVisits = OldSweep(&oldRoot);
        const auto middle = std::chrono::steady_clock::now();
        const auto newVisits = Sweep(&newRoot);
        const auto end = std::chrono::steady_clock::now();
        auto oldNode = oldRoot.m_pDown;
        auto newNode = newRoot.m_pDown;
        bool same = true;
        int survivors = 0;
        while (oldNode && newNode) {
            same = same && oldNode->id == newNode->id && !newNode->m_cDeleted;
            ++survivors;
            oldNode = oldNode->m_pNextLink;
            newNode = newNode->m_pNextLink;
        }
        check(same && !oldNode && !newNode && survivors == count / 2,
            "linear leaf cleanup matches the old tree result and order");
        check(newVisits == count && oldVisits > newVisits * 100,
            "mixed leaf cleanup removes the repeated parent scan");
        std::printf("Cleanup isolated benchmark: old=%zu visits/%lld us new=%zu visits/%lld us\n",
            oldVisits, static_cast<long long>(std::chrono::duration_cast<std::chrono::microseconds>(middle - start).count()),
            newVisits, static_cast<long long>(std::chrono::duration_cast<std::chrono::microseconds>(end - middle).count()));
    }
    check(Node::live == 0, "benchmark releases both trees");

    check(!adapter_quirks::Classify("Intel(R) UHD Graphics 770", 64, 0x8086).intel &&
        !adapter_quirks::Classify("Intel(R) Iris Xe Graphics", 64, 0x8086).intel &&
        adapter_quirks::Classify("Intel(R) 82810 Graphics Controller", 64, 0x8086).intel,
        "Intel 810 workaround excludes UHD and Iris families");
    check(adapter_quirks::Classify("NVIDIA GeForce MX150", 64, 0x10DE).vgaId == 1 &&
        adapter_quirks::Classify("NVIDIA GeForce MX350", 64, 0x10DE).vgaId == 1 &&
        adapter_quirks::Classify("NVIDIA GeForce2 MX", 64, 0x10DE).vgaId == 0,
        "modern MX adapters do not use the GeForce2/4 MX workaround");
    check(adapter_quirks::Classify("NVIDIA misleading description", 64, 0x1002).vgaId == 2 &&
        !adapter_quirks::Classify("NVIDIA misleading description", 64, 0x1002).nvidia &&
        adapter_quirks::Classify("S3 Savage4", 64).savage,
        "PCI vendor takes precedence and Savage spelling is recognized");
    check(effect_visual::FadeArgb(128, 64, 32, 16, 1.0f) == 0x80402010 &&
        effect_visual::FadeArgb(128, 64, 32, 16, 0.5f) == 0x40201008 &&
        effect_visual::FadeArgb(128, 64, 32, 16, 0.0f) == 0,
        "charge keeps and fades distinct alpha, red, green and blue components");
    check(effect_visual::FadeArgb(300, 400, 500, 600, 2.0f) == 0xFFFFFFFF &&
        effect_visual::FadeArgb(128, 64, 32, 16, -1.0f) == 0,
        "fade clamps the progress and source channels");
    check(std::fabs(effect_visual::LitChannel(0.8f, 0.2f, 0.2f, 0.3f) - 0.22f) < 0.00001f &&
        std::fabs(effect_visual::LitChannel(0.1f, 0.9f, 0.2f, 0.3f) - 0.29f) < 0.00001f,
        "lighting computes independent source components");
    for (int index : {3467, 3468, 3469, 3470, 3471, 4906, 4907})
        check(unsupported_item_features::Reject(index, 0), "unsupported later-client item is rejected");
    for (int type = 235; type <= 241; ++type)
        check(unsupported_item_features::Reject(1, type), "unsupported volatile transaction is rejected");
    check(!unsupported_item_features::Reject(3466, 234) && !unsupported_item_features::Reject(3472, 242),
        "the unsupported feature guard keeps adjacent ordinary values");

    const auto fixturePath = Find("tmproject/TMProject748/tests/fixtures/go-receive-packets.txt");
    std::ifstream fixtures(fixturePath);
    check(fixtures.is_open(), "independent Go packet fixture opens");
    std::string line;
    int packets = 0;
    while (std::getline(fixtures, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream row(line);
        std::string name, opcodeText, hex;
        row >> name >> opcodeText >> hex;
        const auto opcode = static_cast<unsigned int>(std::stoul(opcodeText, nullptr, 16));
        const auto digit = [](char c) { return c <= '9' ? c - '0' : c - 'a' + 10; };
        std::vector<char> bytes;
        for (std::size_t i = 0; i + 1 < hex.size(); i += 2)
            bytes.push_back(static_cast<char>(digit(hex[i]) * 16 + digit(hex[i + 1])));
        check(bytes.size() >= 12 && hex.size() == bytes.size() * 2 &&
            Word(bytes, 0) == bytes.size() && Word(bytes, 4) == opcode, name + " has coherent Go metadata");
        if (bytes.size() < 12) continue;
        check(received_packet::CanDispatch({opcode, bytes.data(), bytes.size()}), name + " reaches the C++ receive gate");
        for (std::size_t size = 0; size < bytes.size(); ++size)
            check(!received_packet::CanDispatch({opcode, bytes.data(), size}), name + " rejects a truncated Go frame");
        check(!received_packet::CanDispatch({opcode ^ 0x8000, bytes.data(), bytes.size()}), name + " rejects mismatched metadata");
        if (name == "send_item")
            check(Word(bytes, 12) == 1 && Word(bytes, 14) == 62 && Word(bytes, 16) == 1234,
                "Go send-item bytes retain placement and item offsets");
        if (name == "message_panel")
            check(std::strcmp(bytes.data() + 12, "go-sentinel") == 0, "Go message starts at the C++ text offset");
        bytes.push_back(0);
        check(!received_packet::CanDispatch({opcode, bytes.data(), bytes.size()}), name + " rejects an unannounced trailing byte");
        ++packets;
    }
    check(packets == 72, "all independent Go builder cases were exercised");
    for (unsigned int opcode : {0u, 0x1234u, 0xFFFFu}) {
        std::vector<char> bytes(16);
        PutWord(bytes, 0, 16);
        PutWord(bytes, 4, opcode);
        check(!received_packet::CanDispatch({opcode, bytes.data(), bytes.size()}), "unknown opcode never reaches a consumer");
    }
    for (unsigned int opcode : {0x119u, 0x11Bu, 0x11Du, 0x196u, 0x19Cu, 0x301u, 0x3B3u, 0x7A9u}) {
        std::vector<char> bytes(12);
        PutWord(bytes, 0, 12);
        PutWord(bytes, 4, opcode);
        check(received_packet::CanDispatch({opcode, bytes.data(), bytes.size()}), "named header-only consumer remains accepted");
        bytes.push_back(0);
        PutWord(bytes, 0, 13);
        check(received_packet::CanDispatch({opcode, bytes.data(), bytes.size()}),
            "named header-only consumer keeps opaque bytes without inventing an exact legacy size");
        check(!received_packet::CanDispatch({opcode + 1, bytes.data(), bytes.size()}) &&
            !received_packet::CanDispatch({opcode, bytes.data(), 12}),
            "named header-only consumer still requires matching metadata and declared size");
    }
    return failures;
}
