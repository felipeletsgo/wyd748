#include "../internal/ui/UIBinary.h"
#include "../internal/ui/RCControlTransaction.h"
#include "../internal/core/WYD748Assets.h"
#include <climits>
#include <limits>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
// Minimal intrusive-tree fixture: ownership and sibling links match TreeNode.
struct TransactionNode
{
    TransactionNode* m_pTop = nullptr;
    TransactionNode* m_pDown = nullptr;
    TransactionNode* m_pNextLink = nullptr;
    TransactionNode* m_pPrevLink = nullptr;
    int* live;
    explicit TransactionNode(int& count) : live(&count) { ++*live; }
    virtual ~TransactionNode()
    {
        while (m_pDown)
            delete m_pDown;
        if (m_pPrevLink)
            m_pPrevLink->m_pNextLink = m_pNextLink;
        if (m_pNextLink)
            m_pNextLink->m_pPrevLink = m_pPrevLink;
        if (m_pTop && m_pTop->m_pDown == this)
            m_pTop->m_pDown = m_pNextLink;
        --*live;
    }
    void AddChild(TransactionNode* child)
    {
        child->m_pTop = this;
        child->m_pNextLink = m_pDown;
        m_pDown = child;
        if (child->m_pNextLink)
            child->m_pNextLink->m_pPrevLink = child;
    }
};

template<typename T>
struct FailingCheckpointAllocator
{
    using value_type = T;
    int* remaining;
    int* allocations;
    int* releases;
    T* allocate(std::size_t count)
    {
        if (*remaining == 0)
            throw std::bad_alloc();
        --*remaining;
        T* memory = std::allocator<T>{}.allocate(count);
        ++*allocations;
        return memory;
    }
    void deallocate(T* memory, std::size_t count) noexcept
    {
        ++*releases;
        std::allocator<T>{}.deallocate(memory, count);
    }
};

template<typename Check>
void TestRCTransactions(Check check)
{
    // Fail at every boundary in a nine-record load, on both an empty tree and
    // a populated tree whose nested parents may also receive new children.
    for (bool populated : {false, true})
    {
        for (int failureAfter = 0; failureAfter <= 9; ++failureAfter)
        {
            int live = 0;
            TransactionNode root(live);
            auto* oldFirst = populated ? new TransactionNode(live) : nullptr;
            auto* oldNext = populated ? new TransactionNode(live) : nullptr;
            auto* oldNested = populated ? new TransactionNode(live) : nullptr;
            if (populated)
            {
                root.AddChild(oldNext);
                root.AddChild(oldFirst);
                oldFirst->AddChild(oldNested);
            }
            const int originalLive = live;
            bool failed = false;
            try
            {
                RCControlTransaction<TransactionNode> transaction(&root);
                TransactionNode* newParent = nullptr;
                for (int index = 0; index < 9; ++index)
                {
                    if (index == failureAfter)
                        throw std::bad_alloc();
                    std::unique_ptr<TransactionNode> owner(new TransactionNode(live));
                    TransactionNode* parent = index % 3 == 0 ? &root :
                        (populated && index % 3 == 1 ? oldNested : newParent);
                    parent->AddChild(owner.get());
                    newParent = owner.release();
                }
                if (failureAfter == 9)
                    throw std::bad_alloc();
                transaction.Commit();
            }
            catch (const std::bad_alloc&) { failed = true; }
            check(failed && live == originalLive, "failed RC load deletes only newly created controls");
            check(root.m_pDown == oldFirst && (!populated ||
                (oldFirst->m_pPrevLink == nullptr && oldFirst->m_pNextLink == oldNext &&
                 oldNext->m_pPrevLink == oldFirst && oldNext->m_pTop == &root)),
                "RC rollback restores existing sibling order and back links");
            check(!populated || (oldFirst->m_pDown == oldNested &&
                oldNested->m_pDown == nullptr && oldNested->m_pTop == oldFirst),
                "RC rollback preserves existing nested parents");
        }
    }
    int live = 0;
    {
        TransactionNode root(live);
        {
            RCControlTransaction<TransactionNode> transaction(&root);
            root.AddChild(new TransactionNode(live));
            root.m_pDown->AddChild(new TransactionNode(live));
            transaction.Commit();
        }
        check(live == 3 && root.m_pDown && root.m_pDown->m_pDown,
            "successful RC load commits nested controls to their tree owner");
        {
            RCControlTransaction<TransactionNode> nextLoad(&root);
            root.AddChild(new TransactionNode(live));
            root.m_pDown->AddChild(new TransactionNode(live));
            // Simulate an ordinary read failure: no Commit, no exception.
        }
        check(live == 3 && root.m_pDown->m_pDown && !root.m_pDown->m_pNextLink,
            "failed later RC load preserves previously committed controls");
    }
    check(live == 0, "committed controls are freed exactly once with their root");
    {
        RCControlTransaction<TransactionNode> empty(nullptr);
        empty.Commit();
    }
    check(live == 0, "empty RC transaction does not invent an owner");

    // A failed snapshot must not mutate the original tree. Rollback itself
    // must work with the allocator disabled after a successful snapshot.
    for (int budget = 0; budget <= 6; ++budget)
    {
        TransactionNode root(live);
        for (int index = 0; index < 6; ++index)
            root.AddChild(new TransactionNode(live));
        auto* original = root.m_pDown;
        int remaining = budget, allocations = 0, releases = 0;
        using Allocator = FailingCheckpointAllocator<RCChildCheckpoint<TransactionNode>>;
        try
        {
            RCControlTransaction<TransactionNode, Allocator> transaction(&root,
                Allocator{&remaining, &allocations, &releases});
            root.AddChild(new TransactionNode(live));
            original->AddChild(new TransactionNode(live));
            remaining = 0;
        }
        catch (const std::bad_alloc&) {}
        check(root.m_pDown == original && original->m_pDown == nullptr && live == 7,
            "checkpoint allocation failure or rollback preserves the old tree");
        check(allocations == releases, "snapshot allocation failures release all checkpoint buffers");
    }
    check(live == 0, "transaction fixtures leave no live controls");
}

template<typename Record>
void Append(std::vector<unsigned char>& bytes, int type, const Record& record)
{
    const auto* tag = reinterpret_cast<const unsigned char*>(&type);
    bytes.insert(bytes.end(), tag, tag + sizeof(type));
    const auto* payload = reinterpret_cast<const unsigned char*>(&record);
    bytes.insert(bytes.end(), payload, payload + sizeof(record));
}

std::filesystem::path FindRuntime()
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        if (std::filesystem::is_directory(current / "client748" / "UI"))
            return current / "client748";
        current = current.parent_path();
    }
    return {};
}
}

int RunUIBinaryTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL UI binary: %s\n", name);
        }
    };
    char path[128]{};
    const auto resolve = [&](const char* input, const char* expected) {
        check(ResolveRCBinaryPath(input, path, sizeof(path)) &&
            std::strcmp(path, expected) == 0, "resource path resolves safely");
    };
    resolve("UI\\FieldScene2.txt", "UI\\FieldScene2.bin");
    resolve("UI/SelCharScene2.bin", "UI/SelCharScene2.bin");
    resolve("my_project/UI/Scene.txt", "my_project/UI/Scene.bin");
    resolve("UI/Scene_en.txt", "UI/Scene.bin");
    resolve("UI/Scene_name.txt", "UI/Scene_name.bin");
    resolve("a.t", "a.bin");
    for (const char* invalid : {"", "a", ".txt", "UI/", "UI/a.", "a.b/c"})
        check(!ResolveRCBinaryPath(invalid, path, sizeof(path)), "invalid path rejected");
    check(!ResolveRCBinaryPath(nullptr, path, sizeof(path)), "null source rejected");
    check(!ResolveRCBinaryPath("a.txt", nullptr, 128), "null destination rejected");
    check(!ResolveRCBinaryPath("a.txt", path, 0), "zero path capacity rejected");
    strcpy_s(path, "keep");
    check(!ResolveRCBinaryPath("a.txt", path, 5) && std::strcmp(path, "keep") == 0,
        "short output preserves destination");
    check(ResolveRCBinaryPath("a.txt", path, 6), "exact path capacity accepted");
    const std::string longName(128, 'x');
    check(!ResolveRCBinaryPath((longName + ".txt").c_str(), path, sizeof(path)),
        "long path rejected without CRT invalid-parameter termination");
    int nullTag = 0;
    check(ReadRCControlType(nullptr, nullTag) == RCControlTypeReadResult::Invalid,
        "null stream rejected");

    const auto validate = [&](const std::vector<unsigned char>& bytes, bool legacy,
        bool expected, const char* reason = nullptr, unsigned int externalParent = 0) {
        std::FILE* file = nullptr;
        if (tmpfile_s(&file) != 0 || !file) {
            check(false, "temporary RC fixture created");
            return;
        }
        const bool written = bytes.empty() ||
            std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size();
        std::rewind(file);
        RCValidationError error{};
        const bool result = ValidateRCStream(file, legacy, 500, 256,
            [&](unsigned int id) { return externalParent && id == externalParent; },
            WYD748_TranslateControlID, error, 64);
        check(written && result == expected, "RC preflight returns expected result");
        check(std::ftell(file) == 0, "preflight restores stream on success and rejection");
        if (reason)
            check(error.reason && std::strcmp(reason, error.reason) == 0,
                "RC rejection has a specific diagnostic");
        std::fclose(file);
    };

    for (bool inlineCaptions : {false, true}) {
        for (int type : {1, 2, 3, 6, 10, 12, 13, 15, 16}) {
            const auto size = RCRecordSize(type, inlineCaptions);
            std::vector<unsigned char> record(sizeof(type) + size, 0);
            std::memcpy(record.data(), &type, sizeof(type));
            const auto setInt = [&](std::size_t offset, int value) {
                std::memcpy(record.data() + sizeof(type) + offset, &value, sizeof(value));
            };
            const auto widthOffset = type == 6 ? offsetof(BinListBox, nWidth)
                : type == 10 ? offsetof(BinProgress, nWidth) : offsetof(BinPanel, nWidth);
            const auto heightOffset = widthOffset + sizeof(int);
            setInt(widthOffset, 24);
            setInt(heightOffset, 24);
            if (type == 6) {
                setInt(offsetof(BinListBox, nMaxCount), 10);
                setInt(offsetof(BinListBox, nVisibleCount), 1);
            }
            if (type == 16) {
                setInt(offsetof(BinGrid, nRowCount), 1);
                setInt(offsetof(BinGrid, nColumnCount), 1);
            }
            validate(record, inlineCaptions, true);
            for (const auto offset : {widthOffset, heightOffset}) {
                setInt(offset, -1);
                validate(record, inlineCaptions, false, "negative control size");
                setInt(offset, 24);
            }
            record.pop_back();
            validate(record, inlineCaptions, false, "incomplete control record");
        }
    }

    BinPanel panel{};
    panel.nID = 100;
    BinButton button{};
    button.nID = 101;
    button.nParentID = 100;
    button.nStringIndex = 439;
    std::vector<unsigned char> valid;
    Append(valid, 1, panel);
    Append(valid, 2, button);
    validate(valid, false, true);
    auto duplicate = valid;
    Append(duplicate, 2, button);
    validate(duplicate, false, true);
    for (std::size_t length = 0; length < valid.size(); ++length) {
        if (length == sizeof(int) + sizeof(panel))
            continue; // A complete root-only resource remains valid.
        validate(std::vector<unsigned char>(valid.begin(), valid.begin() + length), false, false);
    }
    auto trailing = valid;
    trailing.push_back(1);
    validate(trailing, false, false, "incomplete control type");
    auto unknown = valid;
    const int unsupported = 99;
    const auto* tag = reinterpret_cast<const unsigned char*>(&unsupported);
    unknown.insert(unknown.end(), tag, tag + sizeof(unsupported));
    validate(unknown, false, false, "unsupported control type");
    std::vector<unsigned char> child;
    Append(child, 2, button);
    validate(child, false, false, "parent not found before child");
    validate(child, false, true, nullptr, 100);
    Append(child, 1, panel);
    validate(child, false, false, "parent not found before child");
    for (int index : {-1, 500, 1999}) {
        auto invalid = std::vector<unsigned char>{};
        button.nParentID = 0;
        button.nStringIndex = index;
        Append(invalid, 2, button);
        validate(invalid, false, false, "UI string index out of range");
    }
    LegacyBinButton inlineButton{};
    inlineButton.nID = 1;
    LegacyBinText inlineText{};
    inlineText.nID = 2;
    inlineText.nParentID = 1;
    std::vector<unsigned char> legacy;
    Append(legacy, 2, inlineButton);
    Append(legacy, 12, inlineText);
    validate(legacy, true, true);
    std::memset(inlineText.szString, 'x', sizeof(inlineText.szString));
    legacy.clear();
    Append(legacy, 2, inlineButton);
    Append(legacy, 12, inlineText);
    validate(legacy, true, false, "unterminated inline caption");
    std::memset(inlineButton.szString, 'x', sizeof(inlineButton.szString));
    legacy.clear();
    Append(legacy, 2, inlineButton);
    validate(legacy, true, false, "unterminated inline caption");
    BinEdit edit{};
    edit.nID = 3;
    for (int maximum : {-1, 0, 255, 256}) {
        std::vector<unsigned char> editable;
        edit.nMaxStringLength = maximum;
        Append(editable, 13, edit);
        validate(editable, false, maximum == 0 || maximum == 255);
    }
    edit.nMaxStringLength = 255;
    std::memset(edit.szString, 'x', sizeof(edit.szString));
    std::vector<unsigned char> unterminatedEdit;
    Append(unterminatedEdit, 13, edit);
    validate(unterminatedEdit, false, false, "unterminated inline caption");

    // A later-source parent alias remains supported without rewriting raw IDs.
    panel.nID = 4622;
    button.nParentID = 65537;
    button.nStringIndex = 0;
    std::vector<unsigned char> alias;
    Append(alias, 1, panel);
    Append(alias, 2, button);
    validate(alias, false, true);

    // Preflight protects the exact divisors and array counts used by widgets.
    for (bool legacyLayout : {false, true}) {
        BinGrid grid{};
        grid.nID = 200;
        grid.nWidth = 24;
        grid.nHeight = 24;
        grid.nRowCount = 1;
        grid.nColumnCount = 1;
        const auto validateGrid = [&](bool expected, const char* reason = nullptr) {
            std::vector<unsigned char> bytes;
            Append(bytes, 16, grid);
            validate(bytes, legacyLayout, expected, reason);
        };
        for (int invalid : {-1, 0}) {
            grid.nRowCount = invalid;
            validateGrid(false, "invalid grid cell counts");
            grid.nRowCount = 1;
            grid.nColumnCount = invalid;
            validateGrid(false, "invalid grid cell counts");
            grid.nColumnCount = 1;
        }
        grid.nRowCount = INT_MAX;
        grid.nColumnCount = 2;
        validateGrid(false, "invalid grid cell counts");
        grid.nRowCount = 1;
        grid.nColumnCount = 1;
        grid.nWidth = 0;
        validateGrid(false, "zero grid extent");
        grid.nWidth = 24;
        grid.nHeight = 0;
        validateGrid(false, "zero grid extent");
        grid.nHeight = 24;
        // The fixture models a 64-byte render cell; the production caller
        // supplies sizeof(GeomControl), without hardcoding the renderer ABI.
        const auto maximumCells = (std::numeric_limits<std::size_t>::max)() / 64;
        if (maximumCells < INT_MAX) {
            grid.nRowCount = static_cast<int>(maximumCells);
            validateGrid(true);
            ++grid.nRowCount;
            validateGrid(false, "grid allocation size overflow");
            grid.nRowCount = 1;
        }
        grid.nStartX = -24;
        grid.nStartY = INT_MAX;
        validateGrid(true); // Offscreen placement is not malformed geometry.

        BinListBox list{};
        list.nID = 201;
        list.nWidth = 100;
        list.nHeight = 100;
        list.nMaxCount = 10;
        list.nVisibleCount = 5;
        const auto validateList = [&](bool expected, const char* reason = nullptr) {
            std::vector<unsigned char> bytes;
            Append(bytes, 6, list);
            validate(bytes, legacyLayout, expected, reason);
        };
        for (int invalid : {-1, 0}) {
            list.nVisibleCount = invalid;
            validateList(false, "invalid list row counts");
        }
        list.nVisibleCount = 5;
        list.nMaxCount = -1;
        validateList(false, "invalid list row counts");
        list.nMaxCount = 0;
        validateList(true); // An empty list may retain its visible row layout.
        list.nMaxCount = INT_MAX;
        validateList(true); // The constructor retains its existing 1000-item clamp.
        list.nHeight = 0;
        validateList(false, "zero list height");
    }

    TestRCTransactions(check);

    char captions[500][64]{};
    char caption[128] = "keep";
    for (int index : {-1, 500, 1999, INT_MAX}) {
        check(!CopyRCIndexedCaption(captions, index, caption, sizeof(caption)) &&
            std::strcmp(caption, "keep") == 0,
            "invalid caption indexes preserve the destination");
    }
    check(!CopyRCIndexedCaption(captions, 1, caption, sizeof(caption)) &&
        std::strcmp(caption, "keep") == 0, "missing nonzero caption is rejected");
    check(CopyRCIndexedCaption(captions, 0, caption, sizeof(caption)) && !caption[0],
        "zero caption index remains intentionally blank");
    strcpy_s(captions[499], "Final caption");
    check(CopyRCIndexedCaption(captions, 499, caption, sizeof(caption)) &&
        std::strcmp(caption, "Final caption") == 0, "last valid caption row is safe");
    std::memset(captions[2], 'x', sizeof(captions[2]));
    strcpy_s(caption, "keep");
    check(!CopyRCIndexedCaption(captions, 2, caption, sizeof(caption)) &&
        std::strcmp(caption, "keep") == 0, "unterminated caption row is rejected");
    check(!CopyRCIndexedCaption(captions, 499, nullptr, sizeof(caption)),
        "null caption destination is rejected");
    check(!CopyRCIndexedCaption(captions, 499, caption, 0) &&
        std::strcmp(caption, "keep") == 0, "zero caption capacity preserves destination");
    check(CopyRCIndexedCaption(captions, 499, caption, 6) &&
        std::strcmp(caption, "Final") == 0, "bounded caption truncation is preserved");
    check(CopyRCIndexedCaption(captions, 499, caption, 1) && !caption[0],
        "one-byte caption destination is terminated");

    const auto runtime = FindRuntime();
    check(!runtime.empty(), "runtime fixtures are available");
    const auto strings = runtime / "UI" / "UIString.txt";
    const bool stringsLoaded = WYD748_LoadUIStrings(strings.string().c_str(),
        &captions[0][0], 500, sizeof(captions[0]));
    check(stringsLoaded, "real indexed caption table loads");
    for (const char* name : {"FieldScene2.bin", "SelCharScene2.bin", "SelServerScene2.bin"}) {
        const auto fixture = runtime / "UI" / name;
        std::ifstream input(fixture, std::ios::binary);
        check(input.good(), "real Scene2 fixture opens");
        const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)), {});
        const bool inlineCaptions = WYD748_IsLegacyRCFile(fixture.string().c_str());
        validate(bytes, inlineCaptions, true);
        check(!inlineCaptions, "Scene2 fixture uses indexed captions");
        if (inlineCaptions || !stringsLoaded)
            continue;
        std::size_t offset = 0;
        std::size_t captionCount = 0;
        while (offset < bytes.size()) {
            if (bytes.size() - offset < sizeof(int)) {
                check(false, "caption audit requires a complete control tag");
                break;
            }
            int type = 0;
            std::memcpy(&type, bytes.data() + offset, sizeof(type));
            offset += sizeof(type);
            const auto size = RCRecordSize(type, false);
            if (size == 0 || size > bytes.size() - offset) {
                check(false, "caption audit requires a complete known record");
                break;
            }
            if (type == 2 || type == 12) {
                const auto indexOffset = type == 2 ? offsetof(BinButton, nStringIndex)
                    : offsetof(BinText, nStringIndex);
                int index = 0;
                std::memcpy(&index, bytes.data() + offset + indexOffset, sizeof(index));
                check(CopyRCIndexedCaption(captions, index, caption, sizeof(caption)),
                    "shipped Scene2 indexed caption resolves in the actual UI table");
                ++captionCount;
            }
            offset += size;
        }
        check(offset == bytes.size() && captionCount != 0,
            "caption audit walks the entire Scene2 fixture");
    }
    return failures;
}
