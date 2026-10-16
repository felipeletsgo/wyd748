#pragma once

// Guild marks are uploaded by guild leaders on the player panel and served by
// the WYD-Go website. WYD.exe 7.48 read this address from strdef row 377
// (a retired wydglobal.com host); the client requests
// b<group:02><channel:02><guild:04>.bmp below it, a 16x12 24-bit BMP.
namespace guild_mark_download
{
    constexpr const char* Address = "http://wyd.forjago.com/patch/guildmark/";

    // A guild without a mark, or a failed download, is requested again after
    // five minutes, so a newly uploaded mark appears without a restart.
    constexpr unsigned int RetryMs = 300000;
}
