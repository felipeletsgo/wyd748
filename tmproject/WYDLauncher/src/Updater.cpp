#include "Updater.h"

#include "PatchCache.h"
#include "PatchManifest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#include <winhttp.h>

#include <algorithm>
#include <functional>
#include <thread>
#include <vector>

namespace wydpatch {
namespace {

constexpr wchar_t kHost[] = L"wyd.forjago.com";
constexpr wchar_t kManifestPath[] = L"/patch/manifest.txt";
constexpr wchar_t kBlobPrefix[] = L"/patch/files/";
constexpr int kWorkers = 4;
constexpr int kAttempts = 3;
constexpr DWORD kChunk = 256 * 1024;

std::wstring Widen(std::string_view ascii)
{
    std::wstring wide;
    wide.reserve(ascii.size());
    for (char c : ascii)
        wide.push_back(c == '/' ? L'\\' : static_cast<wchar_t>(static_cast<unsigned char>(c)));
    return wide;
}

std::wstring SystemMessage(DWORD code)
{
    wchar_t* buffer = nullptr;
    const DWORD length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, 0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);
    std::wstring text = length ? std::wstring(buffer, length) : L"error " + std::to_wstring(code);
    if (buffer)
        LocalFree(buffer);
    while (!text.empty() && (text.back() == L'\n' || text.back() == L'\r' || text.back() == L' ' || text.back() == L'.'))
        text.pop_back();
    return text + L" (" + std::to_wstring(code) + L")";
}

struct InternetHandle {
    HINTERNET value = nullptr;
    InternetHandle() = default;
    explicit InternetHandle(HINTERNET h) : value(h) {}
    InternetHandle(InternetHandle&& other) noexcept : value(other.value) { other.value = nullptr; }
    InternetHandle(const InternetHandle&) = delete;
    InternetHandle& operator=(const InternetHandle&) = delete;
    ~InternetHandle() { if (value) WinHttpCloseHandle(value); }
};

struct FileHandle {
    HANDLE value = INVALID_HANDLE_VALUE;
    explicit FileHandle(HANDLE h) : value(h) {}
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    ~FileHandle() { Close(); }
    bool Valid() const { return value != INVALID_HANDLE_VALUE; }
    void Close()
    {
        if (value != INVALID_HANDLE_VALUE)
            CloseHandle(value);
        value = INVALID_HANDLE_VALUE;
    }
};

class Sha256 {
public:
    Sha256() { ok_ = BCRYPT_SUCCESS(BCryptCreateHash(BCRYPT_SHA256_ALG_HANDLE, &hash_, nullptr, 0, nullptr, 0, 0)); }
    Sha256(const Sha256&) = delete;
    Sha256& operator=(const Sha256&) = delete;
    ~Sha256() { if (hash_) BCryptDestroyHash(hash_); }

    bool Update(const void* data, ULONG size)
    {
        ok_ = ok_ && BCRYPT_SUCCESS(BCryptHashData(hash_, static_cast<PUCHAR>(const_cast<void*>(data)), size, 0));
        return ok_;
    }
    bool Finish(std::string& hex)
    {
        unsigned char digest[32];
        if (!ok_ || !BCRYPT_SUCCESS(BCryptFinishHash(hash_, digest, sizeof digest, 0)))
            return false;
        static const char digits[] = "0123456789abcdef";
        hex.clear();
        for (unsigned char byte : digest) {
            hex.push_back(digits[byte >> 4]);
            hex.push_back(digits[byte & 15]);
        }
        return true;
    }

private:
    BCRYPT_HASH_HANDLE hash_ = nullptr;
    bool ok_ = false;
};

struct LocalFile {
    bool exists = false;
    bool directory = false;
    std::uint64_t size = 0;
    std::uint64_t writeTime = 0;
};

LocalFile Inspect(const std::wstring& path)
{
    LocalFile file;
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data))
        return file;
    file.exists = true;
    file.directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    file.size = (static_cast<std::uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
    file.writeTime = (static_cast<std::uint64_t>(data.ftLastWriteTime.dwHighDateTime) << 32) |
        data.ftLastWriteTime.dwLowDateTime;
    return file;
}

bool HashFile(const std::wstring& path, const std::atomic<bool>& cancel, std::string& hex, std::wstring& error)
{
    FileHandle file(CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, nullptr));
    if (!file.Valid()) {
        error = L"Cannot read " + path + L": " + SystemMessage(GetLastError());
        return false;
    }
    Sha256 hash;
    std::vector<unsigned char> buffer(1 << 20);
    while (true) {
        if (cancel)
            return false;
        DWORD read = 0;
        if (!ReadFile(file.value, buffer.data(), static_cast<DWORD>(buffer.size()), &read, nullptr)) {
            error = L"Cannot read " + path + L": " + SystemMessage(GetLastError());
            return false;
        }
        if (read == 0)
            break;
        hash.Update(buffer.data(), read);
    }
    if (!hash.Finish(hex)) {
        error = L"SHA-256 is unavailable on this system.";
        return false;
    }
    return true;
}

bool EnsureParentDirectories(const std::wstring& root, std::string_view relative, std::wstring& error)
{
    std::wstring current = root;
    std::size_t start = 0;
    while (true) {
        const std::size_t slash = relative.find('/', start);
        if (slash == std::string_view::npos)
            return true;
        current += L'\\';
        current += Widen(relative.substr(start, slash - start));
        if (!CreateDirectoryW(current.c_str(), nullptr)) {
            const DWORD code = GetLastError();
            const LocalFile existing = Inspect(current);
            if (code != ERROR_ALREADY_EXISTS || !existing.directory) {
                error = L"Cannot create folder " + current + L": " + SystemMessage(code);
                return false;
            }
        }
        start = slash + 1;
    }
}

InternetHandle OpenSession()
{
    InternetHandle session(WinHttpOpen(L"WYDLauncher/1", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session.value) // Automatic proxy discovery needs Windows 8.1.
        session.value = WinHttpOpen(L"WYDLauncher/1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session.value)
        return session;
    DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
    if (!WinHttpSetOption(session.value, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof protocols)) {
        protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        WinHttpSetOption(session.value, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof protocols);
    }
    WinHttpSetTimeouts(session.value, 15000, 15000, 30000, 30000);
    return session;
}

// Streams one HTTPS GET into sink. Fails on a non-200 status, on more than
// maxBytes, on cancellation or on a sink failure.
bool HttpGet(HINTERNET connection, const std::wstring& path, std::uint64_t maxBytes,
    const std::atomic<bool>& cancel, const std::function<bool(const unsigned char*, DWORD)>& sink,
    std::uint64_t& received, std::wstring& error)
{
    received = 0;
    InternetHandle request(WinHttpOpenRequest(connection, L"GET", path.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE | WINHTTP_FLAG_REFRESH));
    if (!request.value ||
        !WinHttpSendRequest(request.value, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request.value, nullptr)) {
        error = L"Cannot reach the update server: " + SystemMessage(GetLastError());
        return false;
    }
    DWORD status = 0;
    DWORD statusSize = sizeof status;
    if (!WinHttpQueryHeaders(request.value, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX) || status != 200) {
        error = L"The update server answered HTTP " + std::to_wstring(status) + L" for " + path + L".";
        return false;
    }
    std::vector<unsigned char> buffer(kChunk);
    while (true) {
        if (cancel)
            return false;
        DWORD read = 0;
        if (!WinHttpReadData(request.value, buffer.data(), kChunk, &read)) {
            error = L"Download interrupted: " + SystemMessage(GetLastError());
            return false;
        }
        if (read == 0)
            return true;
        if (read > maxBytes - received) {
            error = L"The update server sent more data than expected for " + path + L".";
            return false;
        }
        received += read;
        if (!sink(buffer.data(), read)) {
            error = L"Cannot write the downloaded data: " + SystemMessage(GetLastError());
            return false;
        }
    }
}

// Downloads a content-addressed blob next to target and moves it into place
// only after its size and SHA-256 match.
bool DownloadBlob(HINTERNET connection, const std::string& sha256, std::uint64_t size,
    const std::wstring& target, const std::atomic<bool>& cancel, std::atomic<std::uint64_t>& done,
    std::wstring& error)
{
    const std::wstring partial = target + Widen(kPartialSuffix);
    std::uint64_t counted = 0;
    bool ok = false;
    {
        FileHandle file(CreateFileW(partial.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr));
        if (!file.Valid()) {
            error = L"Cannot write " + partial + L": " + SystemMessage(GetLastError());
            return false;
        }
        Sha256 hash;
        std::uint64_t received = 0;
        auto sink = [&](const unsigned char* data, DWORD length) {
            DWORD written = 0;
            if (!WriteFile(file.value, data, length, &written, nullptr) || written != length)
                return false;
            hash.Update(data, length);
            counted += length;
            done += length;
            return true;
        };
        std::string actual;
        if (HttpGet(connection, std::wstring(kBlobPrefix) + Widen(sha256), size, cancel, sink, received, error)) {
            if (received != size || !hash.Finish(actual) || actual != sha256)
                error = L"Downloaded data failed verification for " + target + L".";
            else if (!FlushFileBuffers(file.value))
                error = L"Cannot write " + partial + L": " + SystemMessage(GetLastError());
            else
                ok = true;
        }
    }
    if (ok) {
        SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_NORMAL);
        if (!MoveFileExW(partial.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            const DWORD code = GetLastError();
            error = L"Cannot replace " + target + L": " + SystemMessage(code);
            if (code == ERROR_SHARING_VIOLATION || code == ERROR_ACCESS_DENIED)
                error += L"\nClose the game and try again.";
            ok = false;
        }
    }
    if (!ok) {
        DeleteFileW(partial.c_str());
        done -= counted; // a retry downloads the blob again from zero
    }
    return ok;
}

bool FetchManifest(HINTERNET connection, const std::atomic<bool>& cancel, Manifest& manifest, std::wstring& error)
{
    std::string text;
    std::uint64_t received = 0;
    auto sink = [&](const unsigned char* data, DWORD length) {
        text.append(reinterpret_cast<const char*>(data), length);
        return true;
    };
    if (!HttpGet(connection, kManifestPath, kMaxManifestBytes, cancel, sink, received, error))
        return false;
    std::string parseError;
    if (!ParseManifest(text, manifest, parseError)) {
        error = L"The update manifest is invalid (" + Widen(parseError) + L").";
        return false;
    }
    return true;
}

std::string ReadSmallFile(const std::wstring& path, std::size_t limit)
{
    FileHandle file(CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr));
    if (!file.Valid())
        return {};
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file.value, &size) || size.QuadPart < 0 || static_cast<std::uint64_t>(size.QuadPart) > limit)
        return {};
    std::string text(static_cast<std::size_t>(size.QuadPart), '\0');
    DWORD read = 0;
    if (!text.empty() && (!ReadFile(file.value, text.data(), static_cast<DWORD>(text.size()), &read, nullptr) ||
            read != text.size()))
        return {};
    return text;
}

void SaveCache(const std::wstring& directory, const std::string& text)
{
    const std::wstring path = directory + L"\\" + Widen(kCacheName);
    const std::wstring temporary = path + Widen(kPartialSuffix);
    {
        FileHandle file(CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
        DWORD written = 0;
        if (!file.Valid() || !WriteFile(file.value, text.data(), static_cast<DWORD>(text.size()), &written, nullptr) ||
                written != text.size()) {
            file.Close();
            DeleteFileW(temporary.c_str());
            return;
        }
    }
    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
        DeleteFileW(temporary.c_str());
}

std::wstring FormatMiB(std::uint64_t bytes)
{
    wchar_t text[32];
    swprintf_s(text, L"%.1f MiB", static_cast<double>(bytes) / (1024.0 * 1024.0));
    return text;
}

// Installs a new launcher build. The running image can be renamed but not
// overwritten, so it moves aside and the verified download takes its name.
// If the swap fails, TMPWYDLauncher.exe stays for the client's native
// launcher swap on the server-selection screen.
UpdateResult ReplaceLauncher(HINTERNET connection, const UpdateOptions& options, const Manifest& manifest,
    UpdateProgress& progress, const std::atomic<bool>& cancel, std::wstring& error)
{
    progress.SetStatus(L"Updating the launcher...");
    progress.done = 0;
    progress.total = manifest.launcherSize;
    const std::wstring pending = options.clientDirectory + L"\\" + Widen(kPendingLauncherName);
    const std::wstring retired = options.clientDirectory + L"\\" + Widen(kRetiredLauncherName);
    bool downloaded = false;
    for (int attempt = 0; attempt < kAttempts && !downloaded && !cancel; ++attempt)
        downloaded = DownloadBlob(connection, manifest.launcherSha256, manifest.launcherSize, pending, cancel,
            progress.done, error);
    if (!downloaded)
        return cancel ? UpdateResult::Cancelled : UpdateResult::Failed;
    if (!MoveFileExW(options.launcherPath.c_str(), retired.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        error = L"Cannot replace the launcher: " + SystemMessage(GetLastError());
        return UpdateResult::Failed;
    }
    if (!MoveFileExW(pending.c_str(), options.launcherPath.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        const DWORD code = GetLastError();
        MoveFileExW(retired.c_str(), options.launcherPath.c_str(), MOVEFILE_REPLACE_EXISTING);
        error = L"Cannot install the new launcher: " + SystemMessage(code);
        return UpdateResult::Failed;
    }
    return UpdateResult::SelfUpdated;
}

} // namespace

UpdateResult RunUpdate(const UpdateOptions& options, UpdateProgress& progress,
    const std::atomic<bool>& cancel, std::wstring& error)
{
    progress.done = 0;
    progress.total = 0;
    progress.SetStatus(L"Contacting the update server...");
    // A previous self-update leaves the old image behind once it has exited.
    DeleteFileW((options.clientDirectory + L"\\" + Widen(kRetiredLauncherName)).c_str());

    InternetHandle session = OpenSession();
    if (!session.value) {
        error = L"Cannot initialize HTTPS: " + SystemMessage(GetLastError());
        return UpdateResult::Failed;
    }
    InternetHandle connection(WinHttpConnect(session.value, kHost, INTERNET_DEFAULT_HTTPS_PORT, 0));
    if (!connection.value) {
        error = L"Cannot reach the update server: " + SystemMessage(GetLastError());
        return UpdateResult::Failed;
    }
    Manifest manifest;
    if (!FetchManifest(connection.value, cancel, manifest, error))
        return cancel ? UpdateResult::Cancelled : UpdateResult::Failed;

    if (options.allowSelfUpdate) {
        std::string launcherHash;
        if (!HashFile(options.launcherPath, cancel, launcherHash, error))
            return cancel ? UpdateResult::Cancelled : UpdateResult::Failed;
        if (launcherHash != manifest.launcherSha256)
            return ReplaceLauncher(connection.value, options, manifest, progress, cancel, error);
    }

    // Verify the installed files. Unchanged size and timestamp reuse the
    // cached hash; everything else is read again.
    const PatchCache cache = options.repair ? PatchCache{}
        : ParseCache(ReadSmallFile(options.clientDirectory + L"\\" + Widen(kCacheName), 64u << 20));
    std::vector<CacheEntry> verified(manifest.files.size());
    std::vector<std::size_t> pending;
    std::uint64_t pendingBytes = 0;
    for (std::size_t i = 0; i < manifest.files.size(); ++i) {
        if (cancel)
            return UpdateResult::Cancelled;
        const FileEntry& entry = manifest.files[i];
        progress.SetStatus(L"Checking files (" + std::to_wstring(i + 1) + L" of " +
            std::to_wstring(manifest.files.size()) + L")...");
        const std::wstring target = options.clientDirectory + L"\\" + Widen(entry.path);
        const LocalFile local = Inspect(target);
        if (local.directory) {
            error = L"A folder is in the way of " + target + L".";
            return UpdateResult::Failed;
        }
        if (local.exists && entry.install == Install::Keep)
            continue; // player-owned settings are installed once, never overwritten
        if (local.exists && local.size == entry.size) {
            std::string actual = entry.sha256;
            if (!CacheMatches(cache, entry.path, local.size, local.writeTime, entry.sha256) &&
                !HashFile(target, cancel, actual, error))
                return cancel ? UpdateResult::Cancelled : UpdateResult::Failed;
            if (actual == entry.sha256) {
                verified[i] = CacheEntry{entry.sha256, local.size, local.writeTime};
                continue;
            }
        }
        if (!EnsureParentDirectories(options.clientDirectory, entry.path, error))
            return UpdateResult::Failed;
        pending.push_back(i);
        pendingBytes += entry.size;
    }

    if (!pending.empty()) {
        progress.total = pendingBytes;
        progress.SetStatus(L"Downloading " + std::to_wstring(pending.size()) + L" file(s), " +
            FormatMiB(pendingBytes) + L"...");
        std::atomic<std::size_t> next{0};
        std::atomic<bool> failed{false};
        std::mutex errorMutex;
        auto worker = [&] {
            InternetHandle workerConnection(WinHttpConnect(session.value, kHost, INTERNET_DEFAULT_HTTPS_PORT, 0));
            while (!failed && !cancel) {
                const std::size_t slot = next++;
                if (slot >= pending.size())
                    return;
                const std::size_t index = pending[slot];
                const FileEntry& entry = manifest.files[index];
                const std::wstring target = options.clientDirectory + L"\\" + Widen(entry.path);
                std::wstring workerError;
                bool ok = false;
                if (!workerConnection.value)
                    workerError = L"Cannot reach the update server: " + SystemMessage(GetLastError());
                // Transient network failures are retried; a locked game file is not.
                for (int attempt = 0; workerConnection.value && !ok && !cancel && attempt < kAttempts; ++attempt) {
                    ok = DownloadBlob(workerConnection.value, entry.sha256, entry.size, target, cancel,
                        progress.done, workerError);
                    if (!ok && workerError.find(L"Close the game") != std::wstring::npos)
                        break;
                }
                const LocalFile installed = Inspect(target);
                if (ok && installed.exists && installed.size == entry.size) {
                    verified[index] = CacheEntry{entry.sha256, installed.size, installed.writeTime};
                    continue;
                }
                if (!cancel && !failed.exchange(true)) {
                    std::lock_guard<std::mutex> lock(errorMutex);
                    error = workerError.empty() ? L"Cannot install " + target + L"." : workerError;
                }
                return;
            }
        };
        std::vector<std::thread> workers;
        const std::size_t count = std::min<std::size_t>(kWorkers, pending.size());
        for (std::size_t i = 0; i < count; ++i)
            workers.emplace_back(worker);
        for (std::thread& thread : workers)
            thread.join();
        if (cancel)
            return UpdateResult::Cancelled;
        if (failed) {
            // Keep what was verified so a retry resumes instead of rehashing.
            std::string text;
            for (std::size_t i = 0; i < manifest.files.size(); ++i)
                if (!verified[i].sha256.empty())
                    text += SerializeCacheLine(manifest.files[i].path, verified[i]);
            SaveCache(options.clientDirectory, text);
            return UpdateResult::Failed;
        }
    }

    std::string text;
    for (std::size_t i = 0; i < manifest.files.size(); ++i)
        if (!verified[i].sha256.empty())
            text += SerializeCacheLine(manifest.files[i].path, verified[i]);
    SaveCache(options.clientDirectory, text);
    progress.done = progress.total.load();
    progress.SetStatus(L"The game is up to date (release " + Widen(manifest.release) + L").");
    return UpdateResult::UpToDate;
}

} // namespace wydpatch
