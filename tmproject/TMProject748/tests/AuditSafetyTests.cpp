#include "../internal/application/AssetLoadValidation.h"
#include "../internal/application/IndexedPartyMessage.h"
#include "../internal/application/OwnedSoundBuffers.h"
#include "../internal/application/FieldChatSubmitPolicy.h"
#include "../internal/application/WrappedTexture.h"
#include "../internal/application/ShadeGrid.h"
#include <climits>
#include <string>

int RunAuditSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) { ++failures; std::fprintf(stderr, "FAIL audit safety: %s\n", message); }
    };
    int id = -1, count = -1, parts = -1;
    std::string path;
    check(asset_load::ParseList("220 sound.wav 4", id, path, count, parts, true, false, 512, 256), "sound record");
    for (const char* line : {"0 sound.wav 1", "512 sound.wav 1", "x sound.wav 1", "2 sound.wav", "2 sound.wav 0", "2 sound.wav -1", "2 sound.wav 513", "2 sound.wav 999999999999", "2 sound.wav 1 extra"})
        check(!asset_load::ParseList(line, id, path, count, parts, true, false, 512, 256), "invalid sound record");
    check(!asset_load::ParseList("-1 mesh", id, path, count, parts, false, false, 3048, 64), "negative mesh index");
    check(!asset_load::ParseList("3048 mesh", id, path, count, parts, false, false, 3048, 64), "mesh index capacity");
    check(!asset_load::ParseList(("1 " + std::string(64, 'a')).c_str(), id, path, count, parts, false, false, 3048, 64), "long mesh name");
    check(asset_load::ParseList("0 186 8 mesh\\ch01", id, path, count, parts, false, true, 100, 116), "bone record");
    for (const char* line : {"100 1 1 mesh", "0 187 8 mesh", "0 -1 8 mesh", "0 1 9 mesh", "0 1 8", "0 1 8 mesh extra"})
        check(!asset_load::ParseList(line, id, path, count, parts, false, true, 100, 116), "invalid bone record");
    std::size_t bytes = 0;
    check(asset_load::AnimationBytes(2, 3, 3, bytes) && bytes == 384, "animation extent");
    check(!asset_load::AnimationBytes(2, 4, 3, bytes), "inconsistent frame stride");
    check(!asset_load::AnimationBytes(0, 3, 3, bytes), "zero ticks");
    check(!asset_load::AnimationBytes(2, 0, 0, bytes), "zero frames");
    check(!asset_load::AnimationBytes(UINT_MAX, UINT_MAX, 0, bytes), "animation overflow");
    check(!asset_load::AnimationBytes(1u << 30, 1u << 30, 0, bytes), "animation byte multiplication cannot wrap to zero");
    check(!asset_load::AnimationMapping(0, 0), "negative mapped type");
    check(asset_load::AnimationMapping(0, 100), "valid mapping");
    check(!asset_load::AnimationMapping(0, 156), "motion capacity");
    check(!asset_load::AnimationMapping(3, 100), "alternate type capacity");
    char source[260]{};
    check(asset_load::BoneSourcePath("mesh\\ed.dat", source, sizeof(source)) && !std::strcmp(source, "mesh\\ed.bon"), "BGM extension replacement");
    check(!asset_load::BoneSourcePath("", source, sizeof(source)), "empty BGM slot");
    check(!asset_load::BoneSourcePath("sound.wav", source, sizeof(source)), "unestablished BGM format");
    check(asset_load::BoneSourcePath("%s.dat", source, sizeof(source)) && !std::strcmp(source, "%s.bon"), "BGM path is data");

    static char table[2000][128]{};
    char payload[96]{1, 0}, text[128]{};
    for (const int code : {-32768, -1001, -1000, 999, 1000, 32767})
    {
        const auto value = static_cast<std::int16_t>(code);
        std::memcpy(payload + 2, &value, sizeof(value));
        const bool valid = code >= -1000 && code <= 999;
        check(party_message::Decode(payload, sizeof(payload), table, text) == valid, "indexed party boundaries");
        if (valid) check(text == std::to_string(code), "numeric fallback");
    }
    std::int16_t value = 0;
    std::memcpy(payload + 2, &value, sizeof(value));
    std::memset(table[1000], 'x', 128);
    check(party_message::Decode(payload, sizeof(payload), table, text) && std::strlen(text) == 127, "unterminated table entry");
    check(table[1000][127] == 'x' && table[1000][126] == 'x', "global table unchanged");
    check(!party_message::Decode(payload, 3, table, text), "short indexed payload");
    payload[0] = 0;
    check(!party_message::Decode(payload, sizeof(payload), table, text), "unsupported party envelope");

    struct Buffer { int& releases; void Release() { ++releases; } };
    int releases = 0;
    Buffer first{releases}, second{releases};
    { OwnedSoundBuffers<Buffer> buffers(4); buffers.data()[0] = &first; buffers.data()[2] = &second; }
    check(releases == 2, "partial sound construction releases exactly acquired references");
    { OwnedSoundBuffers<Buffer> buffers(1); buffers.data()[0] = &first; buffers.transfer(); }
    check(releases == 2, "successful sound ownership transfer is not released twice");

    FILE* file = nullptr;
    if (tmpfile_s(&file) == 0)
    {
        std::fprintf(file, "%s\n1 mesh\n", std::string(600, 'a').c_str());
        std::rewind(file);
        char line[512]{};
        check(asset_load::ReadLine(file, line) == asset_load::LineResult::Invalid, "overlong physical line rejected");
        check(asset_load::ReadLine(file, line) == asset_load::LineResult::Record && !std::strcmp(line, "1 mesh\n"), "parser progresses after overlong record");
        check(asset_load::ReadLine(file, line) == asset_load::LineResult::End, "EOF terminates parser");
        std::fclose(file);
    }
    else check(false, "temporary parser fixture could not be created");

    for (const int length : {0, 1, 15, 16, 31, 32, 127})
        check(chat_submit::FitsWhisperName(std::string(length, 'x').c_str()) == (length > 0 && length < 16), "whisper name boundaries before packet copy");
    check(!chat_submit::FitsWhisperName(nullptr), "null whisper name");
    for (const int capacity : {64, 512, 3000})
        for (const int index : {-1, 0, capacity - 1, capacity, INT_MAX})
            check(wrapped_texture::ValidIndex(index, capacity) == (index >= 0 && index < capacity), "texture index bounds");
    int textureLength = 0;
    for (const long length : {-1L, 0L, 1L, 4L, 21L, 22L, 88L, 128L, 129L, 64L * 1024 * 1024, LONG_MAX})
    {
        check(wrapped_texture::PayloadLength(length, false, textureLength) == (length >= 22 && length <= 64L * 1024 * 1024), "TGA extent boundaries");
        check(wrapped_texture::PayloadLength(length, true, textureLength) == (length >= 129 && length <= 64L * 1024 * 1024), "DDS extent boundaries");
    }
    for (const bool dds : {false, true})
    {
        FILE* textureFile = nullptr;
        if (tmpfile_s(&textureFile) != 0) { check(false, "texture fixture creation"); continue; }
        char encoded[129]{};
        encoded[85] = '2';
        const int fileLength = dds ? 129 : 22;
        std::fwrite(encoded, 1, fileLength, textureFile);
        std::fflush(textureFile);
        std::rewind(textureFile);
        const char footer[18] = "TRUEVISION-XFILE.";
        std::unique_ptr<char[]> decoded(wrapped_texture::Read(_dup(_fileno(textureFile)), dds, footer, textureLength));
        check(decoded != nullptr, "valid wrapped texture decode");
        if (decoded)
        {
            check(textureLength == (dds ? 128 : 18), "decoded payload extent");
            check(dds ? !std::memcmp(decoded.get(), "DDS", 3) && !std::memcmp(decoded.get() + 84, "DXT1", 4) : !std::memcmp(decoded.get() + textureLength, footer, 18), "established texture wrapper transformation");
        }
        // A descriptor can report a valid file length but still provide a short read.
        std::fseek(textureFile, fileLength - 1, SEEK_SET);
        std::unique_ptr<char[]> truncated(wrapped_texture::Read(_dup(_fileno(textureFile)), dds, footer, textureLength));
        check(truncated == nullptr, "short texture read rejected");
        std::fclose(textureFile);
    }
    check(shade_grid::VertexCount(70) == 5041 && shade_grid::IndexCount(70) == 29400, "shade allocation counts elements, not bytes");
    check(shade_grid::BoundedSize(INT_MAX) == 255 && shade_grid::VertexCount(INT_MAX) == 65536, "shade indices fit unsigned short");
    check(shade_grid::BoundedSize(INT_MIN) == 2, "shade lower grid bound");
    return failures;
}
