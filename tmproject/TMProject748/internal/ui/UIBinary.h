#pragma once

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <climits>
#include <limits>

enum class RCControlTypeReadResult { Record, End, Invalid };

// Use the actual caption table dimensions, not the independent message-table
// limit. Index zero is an intentional blank/dynamic caption in Scene2 resources.
template<std::size_t Count, std::size_t Width>
inline bool CopyRCIndexedCaption(const char (&table)[Count][Width], int index,
    char* output, std::size_t capacity)
{
    if (!output || capacity == 0 || index < 0 ||
        static_cast<std::size_t>(index) >= Count)
        return false;
    const char* row = table[index];
    const auto* end = static_cast<const char*>(std::memchr(row, '\0', Width));
    if (!end || (index != 0 && end == row))
        return false;
    const auto length = (std::min)(static_cast<std::size_t>(end - row), capacity - 1);
    std::memmove(output, row, length);
    output[length] = '\0';
    return true;
}

// A clean EOF is valid only between complete four-byte control-type tags.
inline RCControlTypeReadResult ReadRCControlType(std::FILE* stream, int& type)
{
    static_assert(sizeof(type) == 4, "RC control type ABI changed");
    if (!stream)
        return RCControlTypeReadResult::Invalid;
    const std::size_t bytesRead = std::fread(&type, 1, sizeof(type), stream);
    if (bytesRead == sizeof(type))
        return RCControlTypeReadResult::Record;
    return bytesRead == 0 && std::feof(stream)
        ? RCControlTypeReadResult::End : RCControlTypeReadResult::Invalid;
}

// Resolve the resource extension without modifying directory names. Preserve
// the old optional three-character suffix (for example Scene_en.txt).
inline bool ResolveRCBinaryPath(const char* source, char* output, std::size_t capacity)
{
    if (!source || !output || capacity == 0)
        return false;
    const char* base = source;
    for (const char* cursor = source; *cursor; ++cursor)
        if (*cursor == '/' || *cursor == '\\')
            base = cursor + 1;
    const char* extension = std::strrchr(base, '.');
    if (!extension || extension == base || !extension[1])
        return false;
    const char* stemEnd = extension;
    if (extension - base > 3 && extension[-3] == '_')
        stemEnd -= 3;
    const auto stemLength = static_cast<std::size_t>(stemEnd - source);
    if (stemLength > capacity || capacity - stemLength < sizeof(".bin"))
        return false;
    std::memmove(output, source, stemLength);
    std::memcpy(output + stemLength, ".bin", sizeof(".bin"));
    return true;
}

struct BinCheckBox
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
};

struct BinListBox
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nMaxCount;
    int nVisibleCount;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nFillType;
    int nSelect;
    int nScroll;
};

struct BinGrid
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nRowCount;
    int nColumnCount;
    int nType;
};

struct BinPanel
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nFillType;
    int nPickable;
};

struct Bin3DObj
{
    int nID;
    int nParentID;
    int n3DObjIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
};

struct BinButton
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nSound;
    int nStringIndex;
};

// WYD 7.48 stores button captions directly in older RC records. Keeping this
// disk-only shape separate prevents the modern string-table index from being
// read out of the first four caption bytes.
struct LegacyBinButton
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nSound;
    char szString[128];
};

static_assert(sizeof(LegacyBinButton) == 164, "WYD 7.48 button RC ABI changed");

struct BinText
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nFontColor;
    int nBorder;
    int nBorderColor;
    int nTextType;
    int nAlignType;
    int nStringIndex;
};

// Text controls in the original 7.48 RC format also own a 128-byte inline
// caption; newer TMProject RC files replaced it with nStringIndex.
struct LegacyBinText
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nFontColor;
    int nBorder;
    int nBorderColor;
    int nTextType;
    int nAlignType;
    char szString[128];
};

static_assert(sizeof(LegacyBinText) == 176, "WYD 7.48 text RC ABI changed");

struct BinEdit
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nFontColor;
    int nBorder;
    int nBorderColor;
    int nTextType;
    int nAlignType;
    int nMaxStringLength;
    int nPassword;
    char szString[128];
};

struct BinProgress
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nCurrent;
    int nMaxValue;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nProgressColor;
    int nColor;
    int nStyle;
};

// ReadRCBin consumes these records directly from disk. Keep their Win32 ABI
// explicit so a field reorder or alignment change cannot desynchronize every
// following control in the RC stream.
static_assert(sizeof(BinCheckBox) == 32, "checkbox RC ABI changed");
static_assert(sizeof(BinListBox) == 52, "list-box RC ABI changed");
static_assert(sizeof(BinGrid) == 40, "grid RC ABI changed");
static_assert(sizeof(BinPanel) == 40, "panel RC ABI changed");
static_assert(sizeof(Bin3DObj) == 28, "3D-object RC ABI changed");
static_assert(sizeof(BinButton) == 40, "button RC ABI changed");
static_assert(sizeof(BinText) == 52, "text RC ABI changed");
static_assert(sizeof(BinEdit) == 184, "editable-text RC ABI changed");
static_assert(sizeof(BinProgress) == 48, "progress-bar RC ABI changed");

static_assert(offsetof(BinCheckBox, nID) == 0, "checkbox ID offset changed");
static_assert(offsetof(BinCheckBox, nParentID) == 4, "checkbox parent offset changed");
static_assert(offsetof(BinCheckBox, nTextureSetIndex) == 8, "checkbox texture offset changed");
static_assert(offsetof(BinCheckBox, nStartX) == 12, "checkbox X offset changed");
static_assert(offsetof(BinCheckBox, nStartY) == 16, "checkbox Y offset changed");
static_assert(offsetof(BinCheckBox, nWidth) == 20, "checkbox width offset changed");
static_assert(offsetof(BinCheckBox, nHeight) == 24, "checkbox height offset changed");
static_assert(offsetof(BinCheckBox, nColor) == 28, "checkbox color offset changed");

static_assert(offsetof(BinListBox, nID) == 0, "list-box ID offset changed");
static_assert(offsetof(BinListBox, nParentID) == 4, "list-box parent offset changed");
static_assert(offsetof(BinListBox, nTextureSetIndex) == 8, "list-box texture offset changed");
static_assert(offsetof(BinListBox, nMaxCount) == 12, "list-box max-count offset changed");
static_assert(offsetof(BinListBox, nVisibleCount) == 16, "list-box visible-count offset changed");
static_assert(offsetof(BinListBox, nStartX) == 20, "list-box X offset changed");
static_assert(offsetof(BinListBox, nStartY) == 24, "list-box Y offset changed");
static_assert(offsetof(BinListBox, nWidth) == 28, "list-box width offset changed");
static_assert(offsetof(BinListBox, nHeight) == 32, "list-box height offset changed");
static_assert(offsetof(BinListBox, nColor) == 36, "list-box color offset changed");
static_assert(offsetof(BinListBox, nFillType) == 40, "list-box fill-type offset changed");
static_assert(offsetof(BinListBox, nSelect) == 44, "list-box select offset changed");
static_assert(offsetof(BinListBox, nScroll) == 48, "list-box scroll offset changed");

static_assert(offsetof(BinGrid, nID) == 0, "grid ID offset changed");
static_assert(offsetof(BinGrid, nParentID) == 4, "grid parent offset changed");
static_assert(offsetof(BinGrid, nTextureSetIndex) == 8, "grid texture offset changed");
static_assert(offsetof(BinGrid, nStartX) == 12, "grid X offset changed");
static_assert(offsetof(BinGrid, nStartY) == 16, "grid Y offset changed");
static_assert(offsetof(BinGrid, nWidth) == 20, "grid width offset changed");
static_assert(offsetof(BinGrid, nHeight) == 24, "grid height offset changed");
static_assert(offsetof(BinGrid, nRowCount) == 28, "grid row offset changed");
static_assert(offsetof(BinGrid, nColumnCount) == 32, "grid column offset changed");
static_assert(offsetof(BinGrid, nType) == 36, "grid item-type offset changed");

static_assert(offsetof(BinPanel, nID) == 0, "panel ID offset changed");
static_assert(offsetof(BinPanel, nParentID) == 4, "panel parent offset changed");
static_assert(offsetof(BinPanel, nTextureSetIndex) == 8, "panel texture offset changed");
static_assert(offsetof(BinPanel, nStartX) == 12, "panel X offset changed");
static_assert(offsetof(BinPanel, nStartY) == 16, "panel Y offset changed");
static_assert(offsetof(BinPanel, nWidth) == 20, "panel width offset changed");
static_assert(offsetof(BinPanel, nHeight) == 24, "panel height offset changed");
static_assert(offsetof(BinPanel, nColor) == 28, "panel color offset changed");
static_assert(offsetof(BinPanel, nFillType) == 32, "panel fill-type offset changed");
static_assert(offsetof(BinPanel, nPickable) == 36, "panel pickable offset changed");

static_assert(offsetof(Bin3DObj, nID) == 0, "3D-object ID offset changed");
static_assert(offsetof(Bin3DObj, nParentID) == 4, "3D-object parent offset changed");
static_assert(offsetof(Bin3DObj, n3DObjIndex) == 8, "3D-object index offset changed");
static_assert(offsetof(Bin3DObj, nStartX) == 12, "3D-object X offset changed");
static_assert(offsetof(Bin3DObj, nStartY) == 16, "3D-object Y offset changed");
static_assert(offsetof(Bin3DObj, nWidth) == 20, "3D-object width offset changed");
static_assert(offsetof(Bin3DObj, nHeight) == 24, "3D-object height offset changed");

static_assert(offsetof(BinButton, nID) == 0, "button ID offset changed");
static_assert(offsetof(BinButton, nParentID) == 4, "button parent offset changed");
static_assert(offsetof(BinButton, nTextureSetIndex) == 8, "button texture offset changed");
static_assert(offsetof(BinButton, nStartX) == 12, "button X offset changed");
static_assert(offsetof(BinButton, nStartY) == 16, "button Y offset changed");
static_assert(offsetof(BinButton, nWidth) == 20, "button width offset changed");
static_assert(offsetof(BinButton, nHeight) == 24, "button height offset changed");
static_assert(offsetof(BinButton, nColor) == 28, "button color offset changed");
static_assert(offsetof(BinButton, nSound) == 32, "button sound offset changed");
static_assert(offsetof(BinButton, nStringIndex) == 36, "button string-index offset changed");
static_assert(offsetof(LegacyBinButton, szString) == 36, "legacy button caption offset changed");

static_assert(offsetof(BinText, nID) == 0, "text ID offset changed");
static_assert(offsetof(BinText, nParentID) == 4, "text parent offset changed");
static_assert(offsetof(BinText, nTextureSetIndex) == 8, "text texture offset changed");
static_assert(offsetof(BinText, nStartX) == 12, "text X offset changed");
static_assert(offsetof(BinText, nStartY) == 16, "text Y offset changed");
static_assert(offsetof(BinText, nWidth) == 20, "text width offset changed");
static_assert(offsetof(BinText, nHeight) == 24, "text height offset changed");
static_assert(offsetof(BinText, nFontColor) == 28, "text font-color offset changed");
static_assert(offsetof(BinText, nBorder) == 32, "text border offset changed");
static_assert(offsetof(BinText, nBorderColor) == 36, "text border-color offset changed");
static_assert(offsetof(BinText, nTextType) == 40, "text type offset changed");
static_assert(offsetof(BinText, nAlignType) == 44, "text alignment offset changed");
static_assert(offsetof(BinText, nStringIndex) == 48, "text string-index offset changed");
static_assert(offsetof(LegacyBinText, szString) == 48, "legacy text caption offset changed");

static_assert(offsetof(BinEdit, nID) == 0, "editable-text ID offset changed");
static_assert(offsetof(BinEdit, nParentID) == 4, "editable-text parent offset changed");
static_assert(offsetof(BinEdit, nTextureSetIndex) == 8, "editable-text texture offset changed");
static_assert(offsetof(BinEdit, nStartX) == 12, "editable-text X offset changed");
static_assert(offsetof(BinEdit, nStartY) == 16, "editable-text Y offset changed");
static_assert(offsetof(BinEdit, nWidth) == 20, "editable-text width offset changed");
static_assert(offsetof(BinEdit, nHeight) == 24, "editable-text height offset changed");
static_assert(offsetof(BinEdit, nFontColor) == 28, "editable-text font-color offset changed");
static_assert(offsetof(BinEdit, nBorder) == 32, "editable-text border offset changed");
static_assert(offsetof(BinEdit, nBorderColor) == 36, "editable-text border-color offset changed");
static_assert(offsetof(BinEdit, nTextType) == 40, "editable-text type offset changed");
static_assert(offsetof(BinEdit, nAlignType) == 44, "editable-text alignment offset changed");
static_assert(offsetof(BinEdit, nMaxStringLength) == 48, "editable-text max-length offset changed");
static_assert(offsetof(BinEdit, nPassword) == 52, "editable-text password offset changed");
static_assert(offsetof(BinEdit, szString) == 56, "editable-text caption offset changed");

static_assert(offsetof(BinProgress, nID) == 0, "progress-bar ID offset changed");
static_assert(offsetof(BinProgress, nParentID) == 4, "progress-bar parent offset changed");
static_assert(offsetof(BinProgress, nTextureSetIndex) == 8, "progress-bar texture offset changed");
static_assert(offsetof(BinProgress, nCurrent) == 12, "progress-bar current offset changed");
static_assert(offsetof(BinProgress, nMaxValue) == 16, "progress-bar maximum offset changed");
static_assert(offsetof(BinProgress, nStartX) == 20, "progress-bar X offset changed");
static_assert(offsetof(BinProgress, nStartY) == 24, "progress-bar Y offset changed");
static_assert(offsetof(BinProgress, nWidth) == 28, "progress-bar width offset changed");
static_assert(offsetof(BinProgress, nHeight) == 32, "progress-bar height offset changed");
static_assert(offsetof(BinProgress, nProgressColor) == 36, "progress-bar progress-color offset changed");
static_assert(offsetof(BinProgress, nColor) == 40, "progress-bar color offset changed");
static_assert(offsetof(BinProgress, nStyle) == 44, "progress-bar style offset changed");

inline std::size_t RCRecordSize(int type, bool inlineCaptions)
{
    switch (type)
    {
    case 1: return sizeof(BinPanel);
    case 2: return inlineCaptions ? sizeof(LegacyBinButton) : sizeof(BinButton);
    case 3: return sizeof(BinCheckBox);
    case 6: return sizeof(BinListBox);
    case 10: return sizeof(BinProgress);
    case 12: return inlineCaptions ? sizeof(LegacyBinText) : sizeof(BinText);
    case 13: return sizeof(BinEdit);
    case 15: return sizeof(Bin3DObj);
    case 16: return sizeof(BinGrid);
    default: return 0;
    }
}

struct RCValidationError
{
    const char* reason = "invalid stream";
    long offset = 0;
    int controlID = 0;
};

// Preflight the entire file before creating controls. Duplicate IDs are legal
// in shipped Scene2 files; retain their first-match lookup behavior. Parents
// must already exist, just as in the live sequential loader. Always restore
// the file position so successful validation does not consume the resource.
template<typename ExistingParent, typename TranslateID>
bool ValidateRCStream(std::FILE* stream, bool inlineCaptions, int stringCount,
    std::size_t editCapacity, ExistingParent existingParent, TranslateID translateID,
    RCValidationError& error, std::size_t gridCellBytes)
{
    error = {};
    std::fpos_t initialPosition{};
    if (!stream || std::fgetpos(stream, &initialPosition) != 0)
        return false;
    const bool valid = [&]() {
        std::vector<unsigned int> available;
        bool sawRecord = false;
        while (true)
        {
            error.offset = std::ftell(stream);
            error.controlID = 0;
            int type = 0;
            const auto tag = ReadRCControlType(stream, type);
            if (tag == RCControlTypeReadResult::End)
            {
                error.reason = sawRecord ? nullptr : "empty resource";
                return sawRecord;
            }
            error.reason = "incomplete control type";
            if (tag != RCControlTypeReadResult::Record)
                return false;
            const auto size = RCRecordSize(type, inlineCaptions);
            error.reason = "unsupported control type";
            if (!size)
                return false;
            unsigned char record[sizeof(BinEdit)]{};
            error.reason = "incomplete control record";
            if (size > sizeof(record) || std::fread(record, 1, size, stream) != size)
                return false;
            const auto readInt = [&](std::size_t offset) {
                int value = 0;
                std::memcpy(&value, record + offset, sizeof(value));
                return value;
            };
            error.controlID = readInt(0);
            // Coordinates may intentionally place a widget outside the viewport.
            // Zero-sized passive controls also occur as placeholders. Only
            // reject extents/counts that cannot be safely consumed by widgets.
            const auto widthOffset = type == 6 ? offsetof(BinListBox, nWidth)
                : type == 10 ? offsetof(BinProgress, nWidth) : offsetof(BinPanel, nWidth);
            const int width = readInt(widthOffset);
            const int height = readInt(widthOffset + sizeof(int));
            error.reason = "negative control size";
            if (width < 0 || height < 0)
                return false;
            if (type == 16)
            {
                const int rows = readInt(offsetof(BinGrid, nRowCount));
                const int columns = readInt(offsetof(BinGrid, nColumnCount));
                error.reason = "invalid grid cell counts";
                if (rows <= 0 || columns <= 0 || columns > INT_MAX / rows)
                    return false;
                error.reason = "grid allocation size overflow";
                const auto cells = static_cast<std::size_t>(rows * columns);
                const auto maximumBytes = (std::numeric_limits<std::size_t>::max)();
                if (!gridCellBytes || cells > maximumBytes / gridCellBytes ||
                    cells > maximumBytes / sizeof(int))
                    return false;
                error.reason = "zero grid extent";
                if (!width || !height)
                    return false;
            }
            if (type == 6)
            {
                error.reason = "invalid list row counts";
                if (readInt(offsetof(BinListBox, nMaxCount)) < 0 ||
                    readInt(offsetof(BinListBox, nVisibleCount)) <= 0)
                    return false;
                error.reason = "zero list height";
                if (!height)
                    return false;
            }
            const auto parent = static_cast<unsigned int>(readInt(sizeof(int)));
            error.reason = "parent not found before child";
            if (parent && !existingParent(parent) &&
                std::find(available.begin(), available.end(), parent) == available.end() &&
                std::none_of(available.begin(), available.end(), [&](unsigned int id) {
                    return translateID(id) == parent;
                }))
                return false;
            if ((type == 2 || type == 12) && !inlineCaptions)
            {
                const int index = readInt(type == 2
                    ? offsetof(BinButton, nStringIndex) : offsetof(BinText, nStringIndex));
                error.reason = "UI string index out of range";
                if (index < 0 || index >= stringCount)
                    return false;
            }
            std::size_t captionOffset = 0;
            if (type == 13)
            {
                captionOffset = offsetof(BinEdit, szString);
                const int maximum = readInt(offsetof(BinEdit, nMaxStringLength));
                error.reason = "editable text limit out of range";
                if (maximum < 0 || static_cast<std::size_t>(maximum) >= editCapacity)
                    return false;
            }
            else if (inlineCaptions && (type == 2 || type == 12))
                captionOffset = type == 2 ? offsetof(LegacyBinButton, szString)
                    : offsetof(LegacyBinText, szString);
            error.reason = "unterminated inline caption";
            if (captionOffset && !std::memchr(record + captionOffset, '\0', 128))
                return false;
            available.push_back(static_cast<unsigned int>(error.controlID));
            sawRecord = true;
        }
    }();
    if (std::fsetpos(stream, &initialPosition) != 0)
    {
        error.reason = "cannot restore resource position";
        return false;
    }
    return valid;
}
