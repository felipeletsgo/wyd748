#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace premium_nell
{
    inline std::string Base64(const std::string& bytes)
    {
        constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve((bytes.size() + 2) / 3 * 4);
        for (std::size_t i = 0; i < bytes.size(); i += 3)
        {
            const auto remaining = bytes.size() - i;
            const unsigned int bits = (static_cast<unsigned char>(bytes[i]) << 16) |
                (remaining > 1 ? static_cast<unsigned char>(bytes[i + 1]) << 8 : 0) |
                (remaining > 2 ? static_cast<unsigned char>(bytes[i + 2]) : 0);
            out += alphabet[(bits >> 18) & 63]; out += alphabet[(bits >> 12) & 63];
            out += remaining > 1 ? alphabet[(bits >> 6) & 63] : '=';
            out += remaining > 2 ? alphabet[bits & 63] : '=';
        }
        return out;
    }
    struct EmbeddedBounds { int X, Y, Width, Height; };
    inline EmbeddedBounds FitEmbedded(int width, int height)
    {
        width = width > 0 ? width : 0;
        height = height > 0 ? height : 0;
        return {0, 0, width, height};
    }

    // NavigateToString reports about:blank on older runtimes and the exact
    // UTF-8 data URL on newer ones. Admit one host-supplied document only.
    class EmbeddedNavigation
    {
    public:
        void Prepare(const std::string& utf8)
        {
            uri_ = L"data:text/html;charset=utf-8;base64,";
            const auto encoded = Base64(utf8);
            uri_.append(encoded.begin(), encoded.end());
            prepared_ = true; started_ = false; id_ = 0;
        }
        bool Admit(const std::wstring& uri, std::uint64_t id)
        {
            if (!prepared_ || started_ || (uri != L"about:blank" && uri != uri_)) return false;
            started_ = true; id_ = id;
            return true;
        }
        bool IsDocument(std::uint64_t id) const { return started_ && id == id_; }
        bool Started() const { return started_; }
    private:
        std::wstring uri_;
        std::uint64_t id_ = 0;
        bool prepared_ = false, started_ = false;
    };

    struct Item
    {
        int ID = 0;
        std::wstring Name;
        int Quantity = 1;
        int Refinement = 0;
        std::uint32_t Price = 0;
        unsigned int Category = 0;
        std::vector<std::wstring> Details;
        struct Effect { unsigned int Code = 0, Value = 0; };
        std::array<Effect, 3> Effects{};
    };
    enum class ActionKind { None, Ready, Buy, Close, Failed };
    struct Action { ActionKind Kind = ActionKind::None; int Slot = -1; };

    // Exact, bounded bridge grammar. The page never supplies an item or price.
    inline Action ParseAction(const std::wstring& message)
    {
        if (message == L"ready") return {ActionKind::Ready};
        if (message == L"close") return {ActionKind::Close};
        if (message.size() < 5 || message.size() > 6 || message.compare(0, 4, L"buy:") != 0) return {};
        int slot = 0;
        for (std::size_t i = 4; i < message.size(); ++i)
        {
            if (message[i] < L'0' || message[i] > L'9') return {};
            slot = slot * 10 + message[i] - L'0';
        }
        return slot < 27 ? Action{ActionKind::Buy, slot} : Action{};
    }

    inline std::wstring Quote(const std::wstring& value)
    {
        std::wstring out = L"\"";
        constexpr wchar_t hex[] = L"0123456789abcdef";
        for (wchar_t c : value)
        {
            if (c == L'"' || c == L'\\') { out += L'\\'; out += c; }
            else if (c < 32) { out += L"\\u00"; out += hex[(c >> 4) & 15]; out += hex[c & 15]; }
            else out += c;
        }
        return out + L'"';
    }

    inline std::wstring ItemDetailsJSON(const Item& item)
    {
        std::wstring json = L",\"details\":[";
        for (std::size_t i = 0; i < item.Details.size(); ++i)
        {
            if (i) json += L',';
            json += Quote(item.Details[i]);
        }
        json += L"],\"effects\":[";
        for (std::size_t i = 0; i < item.Effects.size(); ++i)
        {
            if (i) json += L',';
            json += L"{\"code\":" + std::to_wstring(item.Effects[i].Code) +
                L",\"value\":" + std::to_wstring(item.Effects[i].Value) + L"}";
        }
        return json + L"]";
    }

    // Owns the asynchronous WebView lifetime, with no retained scene pointers.
    class Window
    {
    public:
        Window();
        ~Window();
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        bool Open(void* owner, bool portuguese, const char* account = nullptr);
        void Close();
        Action Poll();
        void Update(const std::array<Item, 27>& items, std::uint32_t balance, bool pending, int result);
        bool IsOpen() const;
        // Called by the existing game message loop; no nested pump or owner disable.
        static bool BlocksGameInput(void* owner);
        static bool HandleOwnerMessage(void* owner, unsigned int message, std::uintptr_t wparam);
    private:
        struct State;
        static std::weak_ptr<State> active_;
        std::shared_ptr<State> state_;
    };
}
