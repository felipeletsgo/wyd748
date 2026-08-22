#include "pch.h"

#include "WYD748Assets.h"

#include "TextureManager.h"
#include "Basedef.h"

#include <array>
#include <climits>
#include <cstdint>
#include <vector>

namespace
{
    constexpr std::size_t LegacyTextureRecordSize = 264;
    constexpr std::size_t ModernTextureRecordSize = 528;
    constexpr std::size_t LegacySpellRecordSize = 96;
    constexpr std::size_t ModernSpellRecordSize = 104;
    constexpr std::size_t LegacyItemRecordSize = 140;
    constexpr std::size_t WYD748SpellCount = 104;
    constexpr std::size_t WYD748ServerNameCount = 11;
    constexpr std::size_t WYD748ServerNameWidth = 9;
    constexpr std::size_t WYD748ServerNameFileSize =
        WYD748ServerNameCount * WYD748ServerNameWidth +
        WYD748ServerNameCount * sizeof(std::int32_t);

    struct LegacySpellRecord
    {
        std::int32_t SkillPoint;
        std::int32_t TargetType;
        std::int32_t ManaSpent;
        std::int32_t Delay;
        std::int32_t Range;
        std::int32_t InstanceType;
        std::int32_t InstanceValue;
        std::int32_t TickType;
        std::int32_t TickValue;
        std::int32_t AffectType;
        std::int32_t AffectValue;
        std::int32_t AffectTime;
        char Act1[8];
        char Act2[8];
        std::int32_t InstanceAttribute;
        std::int32_t TickAttribute;
        std::int32_t Aggressive;
        std::int32_t MaxTarget;
        std::int32_t bParty;
        std::int32_t AffectResist;
        std::int32_t Passive;
        std::int32_t ForceDamage;
    };

    static_assert(sizeof(LegacySpellRecord) == LegacySpellRecordSize);

    struct LegacyItemRecord
    {
        char Name[64];
        std::int16_t IndexMesh;
        std::int16_t IndexTexture;
        std::int16_t IndexVisualEffect;
        std::int16_t RequiredLevel;
        std::int16_t RequiredStrength;
        std::int16_t RequiredIntelligence;
        std::int16_t RequiredDexterity;
        std::int16_t RequiredConstitution;
        STRUCT_STATICEFFECT Effects[12];
        std::int32_t Price;
        std::int16_t Unique;
        std::uint16_t Position;
        std::int16_t Extra;
        std::int16_t Grade;
    };

    // These offsets are the on-disk 7.48 ABI proved by the client patch chain.
    // In particular, Position is a WORD at 0x86; TMProject 7.59 inserted a
    // field there and widened Position, so raw fread would corrupt every row.
    static_assert(sizeof(LegacyItemRecord) == LegacyItemRecordSize);
    static_assert(offsetof(LegacyItemRecord, Effects) == 0x50);
    static_assert(offsetof(LegacyItemRecord, Price) == 0x80);
    static_assert(offsetof(LegacyItemRecord, Position) == 0x86);
    static_assert(offsetof(LegacyItemRecord, Grade) == 0x8A);
    static_assert(sizeof(STRUCT_ITEMLIST) == 156);

    bool IsTextureMode(const unsigned char value)
    {
        return value == 'N' || value == 'A' || value == 'a' || value == 'C';
    }

    bool ReadWholeFile(const char* path, std::vector<unsigned char>& bytes)
    {
        FILE* file = nullptr;
        if (fopen_s(&file, path, "rb") != 0 || file == nullptr)
            return false;

        if (fseek(file, 0, SEEK_END) != 0)
        {
            fclose(file);
            return false;
        }

        const long length = ftell(file);
        if (length < 0 || fseek(file, 0, SEEK_SET) != 0)
        {
            fclose(file);
            return false;
        }

        bytes.resize(static_cast<std::size_t>(length));
        const std::size_t read = bytes.empty() ? 0 : fread(bytes.data(), 1, bytes.size(), file);
        fclose(file);
        return read == bytes.size();
    }

    bool LooksLikeLegacyTextureTable(const std::vector<unsigned char>& bytes)
    {
        if (bytes.size() < LegacyTextureRecordSize || bytes.size() % LegacyTextureRecordSize != 0)
            return false;

        // In 7.48 alpha is byte 255. In the modern record it is byte 510.
        // Checking both offsets avoids confusing 512 legacy rows with 256
        // modern rows because both files are exactly 135168 bytes.
        const bool legacyMode = IsTextureMode(bytes[255]);
        const bool modernMode = bytes.size() > 510 && IsTextureMode(bytes[510]);
        return legacyMode && !modernMode;
    }

    void ResetTextureRecord(stTextureListInfo& record)
    {
        memset(&record, 0, sizeof(record));
        record.cAlpha = 'N';
    }

    bool DecodeLegacyTextureTable(
        const std::vector<unsigned char>& bytes,
        stTextureListInfo* destination,
        const std::size_t destinationCount)
    {
        // Expand each compact 7.48 row instead of copying raw bytes into the
        // wider TMProject struct, whose second path field did not exist yet.
        const std::size_t sourceCount = bytes.size() / LegacyTextureRecordSize;
        const std::size_t count = (std::min)(sourceCount, destinationCount);

        for (std::size_t index = 0; index < destinationCount; ++index)
            ResetTextureRecord(destination[index]);

        for (std::size_t index = 0; index < count; ++index)
        {
            const unsigned char* source = bytes.data() + index * LegacyTextureRecordSize;
            memcpy(destination[index].szFileName, source, sizeof(destination[index].szFileName));
            destination[index].szFileName[sizeof(destination[index].szFileName) - 1] = '\0';
            destination[index].cAlpha = static_cast<char>(source[255]);
        }

        return sourceCount <= destinationCount;
    }

    bool DecodeModernTextureTable(
        const std::vector<unsigned char>& bytes,
        stTextureListInfo* destination,
        const std::size_t destinationCount)
    {
        // Decode named fields so compiler padding never becomes part of the
        // on-disk contract, even when the source is built with another toolset.
        if (bytes.empty() || bytes.size() % ModernTextureRecordSize != 0)
            return false;

        const std::size_t sourceCount = bytes.size() / ModernTextureRecordSize;
        if (sourceCount > destinationCount)
            return false;

        for (std::size_t index = 0; index < destinationCount; ++index)
            ResetTextureRecord(destination[index]);

        for (std::size_t index = 0; index < sourceCount; ++index)
        {
            const unsigned char* source = bytes.data() + index * ModernTextureRecordSize;
            memcpy(destination[index].szFileName, source, sizeof(destination[index].szFileName));
            memcpy(destination[index].szFilePart, source + 255, sizeof(destination[index].szFilePart));
            destination[index].szFileName[sizeof(destination[index].szFileName) - 1] = '\0';
            destination[index].szFilePart[sizeof(destination[index].szFilePart) - 1] = '\0';
            destination[index].cAlpha = static_cast<char>(source[510]);
            memcpy(&destination[index].dwLastUsedTime, source + 512, sizeof(unsigned int));
            memcpy(&destination[index].dwShowTime, source + 516, sizeof(unsigned int));
            memcpy(&destination[index].dwLastUsedTimeOld, source + 520, sizeof(unsigned int));
            memcpy(&destination[index].dwShowTimeOld, source + 524, sizeof(unsigned int));
        }

        return true;
    }

    void CopyLegacySpell(const LegacySpellRecord& source, STRUCT_SPELL& destination)
    {
        // The two trailing TMProject fields are intentionally zero: they do
        // not exist in SkillData 7.48 and must never inherit uninitialized data.
        destination = {};
        destination.SkillPoint = source.SkillPoint;
        destination.TargetType = source.TargetType;
        destination.ManaSpent = source.ManaSpent;
        destination.Delay = source.Delay;
        destination.Range = source.Range;
        destination.InstanceType = source.InstanceType;
        destination.InstanceValue = source.InstanceValue;
        destination.TickType = source.TickType;
        destination.TickValue = source.TickValue;
        destination.AffectType = source.AffectType;
        destination.AffectValue = source.AffectValue;
        destination.AffectTime = source.AffectTime;
        memcpy(destination.Act1, source.Act1, sizeof(destination.Act1));
        memcpy(destination.Act2, source.Act2, sizeof(destination.Act2));
        destination.InstanceAttribute = source.InstanceAttribute;
        destination.TickAttribute = source.TickAttribute;
        destination.Aggressive = source.Aggressive;
        destination.MaxTarget = source.MaxTarget;
        destination.bParty = source.bParty;
        destination.AffectResist = source.AffectResist;
        destination.Passive = source.Passive;
        destination.ForceDamage = source.ForceDamage;
    }

    void CopyLegacyItem(const LegacyItemRecord& source, STRUCT_ITEMLIST& destination)
    {
        // Fields introduced after 7.48 stay zero. Copying named members keeps
        // the richer runtime struct while preserving the compact file's ABI.
        destination = {};
        memcpy(destination.Name, source.Name, sizeof(source.Name));
        destination.Name[sizeof(destination.Name) - 1] = '\0';
        destination.nIndexMesh = source.IndexMesh;
        destination.nIndexTexture = source.IndexTexture;
        destination.nIndexVisualEffect = source.IndexVisualEffect;
        destination.nReqLvl = source.RequiredLevel;
        destination.nReqStr = source.RequiredStrength;
        destination.nReqInt = source.RequiredIntelligence;
        destination.nReqDex = source.RequiredDexterity;
        destination.nReqCon = source.RequiredConstitution;
        memcpy(destination.stEffect, source.Effects, sizeof(source.Effects));
        destination.nPrice = source.Price;
        destination.nUnique = source.Unique;
        destination.nPos = source.Position;
        destination.nExtra = source.Extra;
        destination.nGrade = source.Grade;
    }

    bool WalkRCRecords(const std::vector<unsigned char>& bytes, const bool legacyCaptions)
    {
        // Only controls understood by TMScene::ReadRCBin are accepted here.
        // Exact walking prevents a malformed modern RC from being mistaken for
        // the larger 7.48 inline-caption layout.
        std::size_t offset = 0;
        while (offset < bytes.size())
        {
            if (bytes.size() - offset < sizeof(std::int32_t))
                return false;

            std::int32_t controlType = 0;
            memcpy(&controlType, bytes.data() + offset, sizeof(controlType));
            offset += sizeof(controlType);

            std::size_t payloadSize = 0;
            switch (controlType)
            {
            case 1:  payloadSize = 40; break; // panel
            case 2:  payloadSize = legacyCaptions ? 164 : 40; break; // button
            case 3:  payloadSize = 32; break; // checkbox
            case 6:  payloadSize = 52; break; // list box
            case 10: payloadSize = 48; break; // progress bar
            case 12: payloadSize = legacyCaptions ? 176 : 52; break; // text
            case 13: payloadSize = 184; break; // editable text
            case 15: payloadSize = 28; break; // 3D object
            case 16: payloadSize = 40; break; // grid
            default: return false;
            }

            if (payloadSize > bytes.size() - offset)
                return false;
            offset += payloadSize;
        }

        return offset == bytes.size();
    }

}

bool WYD748_InitializeAssetRoot()
{
    // An explicit root lets the freshly compiled executable consume client748
    // directly without copying or modifying the distributed WYD.exe.
    std::array<char, MAX_PATH> root{};
    const DWORD configuredLength = GetEnvironmentVariableA(
        "WYD_ASSET_ROOT", root.data(), static_cast<DWORD>(root.size()));

    if (configuredLength > 0 && configuredLength < root.size())
        return SetCurrentDirectoryA(root.data()) != FALSE;

    if (GetModuleFileNameA(nullptr, root.data(), static_cast<DWORD>(root.size())) == 0)
        return false;

    char* separator = strrchr(root.data(), '\\');
    if (separator == nullptr)
        return false;

    *separator = '\0';
    return SetCurrentDirectoryA(root.data()) != FALSE;
}

bool WYD748_LoadTextureList(
    const char* path,
    const char* legacyFallbackPath,
    stTextureListInfo* destination,
    const std::size_t destinationCount)
{
    // UITextureListN.bin is a newer name; the 7.48 package uses
    // UITextureList.bin and reaches it through the supplied fallback.
    if (path == nullptr || destination == nullptr || destinationCount == 0)
        return false;

    std::vector<unsigned char> bytes;
    if (!ReadWholeFile(path, bytes))
    {
        if (legacyFallbackPath == nullptr || !ReadWholeFile(legacyFallbackPath, bytes))
            return false;
    }

    if (LooksLikeLegacyTextureTable(bytes))
        return DecodeLegacyTextureTable(bytes, destination, destinationCount);

    return DecodeModernTextureTable(bytes, destination, destinationCount);
}

bool WYD748_LoadSkillData(
    const char* path,
    STRUCT_SPELL* destination,
    const std::size_t destinationCount)
{
    // Reject unknown lengths rather than accepting a partial fread: shifted
    // skill rows would corrupt targeting, delays and visual effects together.
    if (path == nullptr || destination == nullptr || destinationCount == 0)
        return false;

    std::vector<unsigned char> bytes;
    if (!ReadWholeFile(path, bytes))
        return false;

    memset(destination, 0, sizeof(STRUCT_SPELL) * destinationCount);

    // The 7.48 table has no trailing checksum: 104 records x 96 bytes.
    if (bytes.size() == WYD748SpellCount * LegacySpellRecordSize)
    {
        if (destinationCount < WYD748SpellCount)
            return false;

        for (unsigned char& value : bytes)
            value ^= 0x5A;

        for (std::size_t index = 0; index < WYD748SpellCount; ++index)
        {
            LegacySpellRecord source{};
            memcpy(&source, bytes.data() + index * LegacySpellRecordSize, sizeof(source));
            CopyLegacySpell(source, destination[index]);
        }
        return true;
    }

    // TMProject tables may include a four-byte checksum after 104-byte rows.
    std::size_t payloadSize = bytes.size();
    if (payloadSize >= 4 && (payloadSize - 4) % ModernSpellRecordSize == 0)
        payloadSize -= 4;

    if (payloadSize == 0 || payloadSize % ModernSpellRecordSize != 0)
        return false;

    const std::size_t sourceCount = payloadSize / ModernSpellRecordSize;
    if (sourceCount > destinationCount)
        return false;

    for (std::size_t index = 0; index < payloadSize; ++index)
        bytes[index] ^= 0x5A;

    for (std::size_t index = 0; index < sourceCount; ++index)
        memcpy(&destination[index], bytes.data() + index * ModernSpellRecordSize, sizeof(STRUCT_SPELL));

    return true;
}

bool WYD748_LoadItemList(
    const char* path,
    STRUCT_ITEMLIST* destination,
    const std::size_t destinationCount)
{
    // The shipped file is exactly 6500 compact rows plus a four-byte stamp.
    // The native 7.48 reader excludes the stamp from XOR decoding and does not
    // use it as a record, so accepting any other shape would reintroduce drift.
    if (path == nullptr || destination == nullptr || destinationCount == 0)
        return false;

    std::vector<unsigned char> bytes;
    if (!ReadWholeFile(path, bytes))
        return false;

    const std::size_t payloadSize = destinationCount * LegacyItemRecordSize;
    if (bytes.size() != payloadSize + sizeof(std::uint32_t))
        return false;

    memset(destination, 0, sizeof(STRUCT_ITEMLIST) * destinationCount);
    for (std::size_t index = 0; index < payloadSize; ++index)
        bytes[index] ^= 0x5A;

    for (std::size_t index = 0; index < destinationCount; ++index)
    {
        LegacyItemRecord source{};
        memcpy(&source, bytes.data() + index * LegacyItemRecordSize, sizeof(source));
        CopyLegacyItem(source, destination[index]);
    }

    return true;
}

bool WYD748_LoadMessageStrings(
    const char* path,
    char* destination,
    const std::size_t destinationCount,
    const std::size_t stringWidth)
{
    // 7.48 stores only the rows it knows (440 in the current corpus), while
    // TMProject reserves 2000. Preserve indices and leave newer rows empty.
    if (path == nullptr || destination == nullptr || destinationCount == 0 || stringWidth == 0)
        return false;

    std::vector<unsigned char> bytes;
    if (!ReadWholeFile(path, bytes) || bytes.size() < sizeof(std::int32_t))
        return false;

    const std::size_t payloadSize = bytes.size() - sizeof(std::int32_t);
    if (payloadSize == 0 || payloadSize % stringWidth != 0)
        return false;

    const std::size_t sourceCount = payloadSize / stringWidth;
    if (sourceCount > destinationCount)
        return false;

    std::int32_t storedChecksum = 0;
    memcpy(&storedChecksum, bytes.data() + payloadSize, sizeof(storedChecksum));
    if (storedChecksum != BASE_GetSum(reinterpret_cast<char*>(bytes.data()), static_cast<int>(payloadSize)))
        return false;

    memset(destination, 0, destinationCount * stringWidth);
    for (std::size_t index = 0; index < payloadSize; ++index)
        destination[index] = static_cast<char>(bytes[index] ^ 0x5A);

    return true;
}

bool WYD748_LoadServerNameList(
    const char* path,
    char (*names)[16],
    const std::size_t nameCount,
    int* groupOrder,
    const std::size_t groupOrderCount)
{
    // sn.bin is 11 compact nine-byte names followed by 11 little-endian
    // group-order values. Rejecting every other size avoids treating the file
    // as the line-oriented format used by newer TMProject clients.
    if (path == nullptr || names == nullptr || groupOrder == nullptr ||
        nameCount < WYD748ServerNameCount || groupOrderCount < WYD748ServerNameCount)
    {
        return false;
    }

    std::vector<unsigned char> bytes;
    if (!ReadWholeFile(path, bytes) || bytes.size() != WYD748ServerNameFileSize)
        return false;

    const std::size_t orderOffset = WYD748ServerNameCount * WYD748ServerNameWidth;
    std::array<std::int32_t, WYD748ServerNameCount> decodedOrder{};
    for (std::size_t index = 0; index < WYD748ServerNameCount; ++index)
    {
        memcpy(&decodedOrder[index], bytes.data() + orderOffset + index * sizeof(std::int32_t), sizeof(std::int32_t));
        if (decodedOrder[index] < 0 || decodedOrder[index] >= static_cast<std::int32_t>(WYD748ServerNameCount))
            return false;
    }

    memset(names, 0, nameCount * 16);
    memset(groupOrder, 0, groupOrderCount * sizeof(int));
    for (std::size_t index = 0; index < WYD748ServerNameCount; ++index)
    {
        memcpy(names[index], bytes.data() + index * WYD748ServerNameWidth, WYD748ServerNameWidth);
        names[index][15] = '\0';
        groupOrder[index] = decodedOrder[index];
    }

    return true;
}

bool WYD748_LoadClientConfig(
    const char* path,
    short* version,
    short* values,
    const std::size_t valueCount)
{
    // The original 7.48 launcher writes one named value per line. Requiring
    // every key exactly once prevents a partial or foreign config variant from
    // silently selecting a different resolution, UI mode or input contract.
    static constexpr const char* keys[] = {
        "VERSION",
        "RES",
        "ANIMATION",
        "SOUND",
        "MUSIC",
        "SERVER",
        "BRIGHT",
        "CURSOR",
        "DEMO",
        "WINDOW",
        "CLASSIC",
        "CAMERAROTATE",
        "DXT",
        "KEYTYPE",
        "CAMERAVIEW",
    };

    if (path == nullptr || version == nullptr || values == nullptr ||
        valueCount < _countof(keys) - 1)
    {
        return false;
    }

    FILE* file = nullptr;
    if (fopen_s(&file, path, "rt") != 0 || file == nullptr)
        return false;

    std::array<int, _countof(keys)> parsedValues{};
    std::array<bool, _countof(keys)> seen{};
    char line[128]{};
    bool valid = true;
    while (fgets(line, static_cast<int>(sizeof(line)), file) != nullptr)
    {
        char key[32]{};
        int value = 0;
        if (sscanf_s(line, " [%31[^]]] %d", key, static_cast<unsigned int>(_countof(key)), &value) != 2)
        {
            // Blank lines are harmless, but any other syntax means this is
            // not the 7.48 config schema and must fall back as a whole.
            if (strspn(line, " \t\r\n") != strlen(line))
                valid = false;
            continue;
        }

        std::size_t keyIndex = _countof(keys);
        for (std::size_t index = 0; index < _countof(keys); ++index)
        {
            if (strcmp(key, keys[index]) == 0)
            {
                keyIndex = index;
                break;
            }
        }

        if (keyIndex == _countof(keys) || seen[keyIndex] || value < SHRT_MIN || value > SHRT_MAX)
        {
            valid = false;
            continue;
        }

        parsedValues[keyIndex] = value;
        seen[keyIndex] = true;
    }
    fclose(file);

    for (const bool present : seen)
        valid = valid && present;
    if (!valid)
        return false;

    *version = static_cast<short>(parsedValues[0]);
    for (std::size_t index = 0; index < valueCount; ++index)
        values[index] = static_cast<short>(parsedValues[index + 1]);
    return true;
}

bool WYD748_IsLegacyRCFile(const char* path)
{
    // A legacy file must walk exactly with inline caption sizes and must not
    // also be a valid modern walk. This keeps all newer scene files on their
    // original parser while enabling the exclusive 7.48 server-selection UI.
    if (path == nullptr)
        return false;

    std::vector<unsigned char> bytes;
    if (!ReadWholeFile(path, bytes) || bytes.empty())
        return false;

    return WalkRCRecords(bytes, true) && !WalkRCRecords(bytes, false);
}

unsigned int WYD748_TranslateControlID(const unsigned int controlID)
{
    // The 7.48 server-selection RC files keep their original IDs, while the
    // imported TMProject scene addresses the same controls through newer IDs. The
    // explicit table preserves event and parent relationships without editing
    // the user's UI binary or guessing from record order at runtime.
    struct ControlIDTranslation
    {
        unsigned int legacy;
        unsigned int modern;
    };

    static constexpr ControlIDTranslation translations[] = {
        { 4622, 65537 }, // server selection panel
        { 4620, 65538 }, // confirm server
        { 5637, 65539 }, // leave server selection
        { 5635, 65540 }, // server title
        { 5636, 65541 }, // channel title
        { 4618, 65542 }, // server group list
        { 4619, 65543 }, // channel list
        { 4608, 65870 }, // login panel
        { 5121, 65871 }, // account edit
        { 5122, 65872 }, // password edit
        { 4609, 65873 }, // login button
        { 4611, 65874 }, // quit button
        { 4610, 65875 }, // create-account button
        { 5632, 65876 }, // login caption
        { 5633, 65877 }, // account caption
        { 5634, 65878 }, // password caption

        // WYD 7.48 FieldScene2.bin keeps the original Main Menu and panel
        // button IDs. Translate the whole gameplay family at this single ABI
        // boundary so the newer source handlers remain the only place that
        // builds packets or changes UI state.
        { 293, 65790 }, // Character
        { 294, 65791 }, // Inventory/equipment
        { 295, 65792 }, // Skills
        { 297, 65799 }, // Party
        { 304, 65796 }, // System
        { 313, 65794 }, // Auto trade
        { 314, 65795 }, // Helper
        { 315, 65793 }, // Quest log
        { 368, 65562 }, // Inventory close
        { 533, 65769 }, // Character close
        { 1913, 65568 }, // Skill close

        // Legacy score '+' buttons use different IDs. Mapping the complete
        // family here routes them through the existing MSG_ApplyBonus handlers
        // and keeps the server authoritative for every allocated point.
        { 1073, 65716 }, // STR
        { 1076, 65719 }, // INT
        { 1079, 65722 }, // DEX
        { 1088, 65725 }, // CON
        { 1139, 65754 }, // weapon mastery
        { 1142, 65757 }, // class mastery 1
        { 1145, 65760 }, // class mastery 2
        { 1154, 65763 }, // class mastery 3
    };

    for (const ControlIDTranslation& translation : translations)
    {
        if (controlID == translation.legacy)
            return translation.modern;
    }

    return controlID;
}
