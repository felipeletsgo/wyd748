#include "../internal/application/MusicListLine.h"

#include <cstdio>
#include <cstring>

int RunMusicListLineTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL music list line: %s\n", message);
        }
    };

    struct Case { const char* line; bool parsed; const char* path; };
    const Case cases[] = {
        {"Thema            music\\\\login.mp3\t\t\r\n", true, "music\\\\login.mp3"},
        {"Armia            music\\\\town01.mp3\r\n", true, "music\\\\town01.mp3"},
        {"Field1           music\\\\field01.mp3\n", true, "music\\\\field01.mp3"},
        {"under            music\\\\dungeon01.mp3", true, "music\\\\dungeon01.mp3"},
        {"Thema            \r\n", false, ""},
        {"short\r\n", false, ""},
        {"", false, ""},
    };
    for (const Case& item : cases) {
        char path[256] = "stale";
        const bool parsed = music_list::ParseLine(item.line, path, sizeof(path));
        check(parsed == item.parsed, "parse result");
        check(!std::strcmp(path, item.path), "path text");
    }

    char tiny[8] = "stale";
    check(!music_list::ParseLine("Thema            music\\\\login.mp3", tiny, sizeof(tiny)),
        "path longer than the destination is refused");
    check(tiny[0] == '\0', "refused path leaves an empty destination");
    check(!music_list::ParseLine(nullptr, tiny, sizeof(tiny)), "null line");
    return failures;
}
