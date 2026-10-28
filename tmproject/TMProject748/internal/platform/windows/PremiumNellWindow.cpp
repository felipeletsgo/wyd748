#include "pch.h"
#include "PremiumNellWindow.h"
#include "../../core/resource.h"
#include "../diagnostics/ClientDiagnostics.h"
#include <WebView2.h>
#include <wrl.h>
#include <deque>
#include <shlobj.h>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace premium_nell
{
// Original presentation assets are shipped in the executable. No third-party
// page, authentication, scripts or checkout requests enter the store document.
static bool EmbedArt(HMODULE module, std::string& html)
{
    const struct { const char* Name; int Resource; const char* Type; } art[] = {
        {"bar6.png", IDR_NELL_BANNER, "png"}, {"base3.png", IDR_NELL_WALLET, "png"},
        {"np_icon.png", IDR_NELL_COIN, "png"}, {"np_icon_small7.png", IDR_NELL_SMALL_COIN, "png"},
        {"search2.png", IDR_NELL_SEARCH, "png"}, {"store_item_base2.png", IDR_NELL_CARD, "png"},
        {"button5.png", IDR_NELL_BUTTON, "png"}, {"pt_br.gif", IDR_NELL_PT, "gif"}, {"en.gif", IDR_NELL_EN, "gif"}
    };
    for (const auto& asset : art)
    {
        const auto resource = FindResourceW(module, MAKEINTRESOURCEW(asset.Resource), MAKEINTRESOURCEW(10));
        const auto loaded = resource ? LoadResource(module, resource) : nullptr;
        const auto bytes = loaded ? static_cast<const char*>(LockResource(loaded)) : nullptr;
        if (!bytes) return false;
        const std::string token = std::string("nell-art:") + asset.Name;
        const std::string url = std::string("data:image/") + asset.Type + ";base64," +
            Base64(std::string(bytes, SizeofResource(module, resource)));
        std::size_t at = 0;
        while ((at = html.find(token, at)) != std::string::npos)
        {
            html.replace(at, token.size(), url);
            at += url.size();
        }
    }
    return true;
}
struct Window::State
{
    HWND Handle = nullptr;
    HWND Owner = nullptr;
    ComPtr<ICoreWebView2Controller> Controller;
    ComPtr<ICoreWebView2> View;
    std::deque<Action> Actions;
    std::wstring Snapshot;
    std::wstring Account;
    EmbeddedNavigation Navigation;
    bool Portuguese = false, Pending = false, Alive = true, Ready = false, Apartment = false;
    bool Presented = false, Failed = false;
    ULONGLONG Started = GetTickCount64();

    void Fail(const char* stage, HRESULT error = E_FAIL)
    {
        if (Alive && !Failed)
        {
            Failed = true;
            WYD748_DiagnosticsLog("Premium Nell failure stage=%s hr=0x%08lX\r\n", stage, static_cast<unsigned long>(error));
            Actions.clear(); Actions.push_back({ActionKind::Failed});
        }
    }
    bool Present(bool visible)
    {
        if (!Alive || !Controller || Failed) return false;
        RECT bounds{};
        if (!GetClientRect(Handle, &bounds))
        {
            Fail("client-bounds", HRESULT_FROM_WIN32(GetLastError()));
            return false;
        }
        // A minimized host has no drawable area. Restore its bounds before
        // making the browser visible again; showing the HWND alone is insufficient.
        if (visible && bounds.right > 0 && bounds.bottom > 0)
        {
            const HRESULT resized = Controller->put_Bounds(bounds);
            if (FAILED(resized)) { Fail("browser-bounds", resized); return false; }
        }
        const HRESULT shown = Controller->put_IsVisible(visible ? TRUE : FALSE);
        if (FAILED(shown)) { Fail("browser-visible", shown); return false; }
        return true;
    }
    bool SyncOwner()
    {
        RECT client{};
        if (!IsWindow(Handle) || !GetClientRect(Owner, &client))
        {
            Fail("owner-bounds", HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE));
            return false;
        }
        const auto bounds = FitEmbedded(client.right, client.bottom);
        const bool visible = Presented && !IsIconic(Owner) && IsWindowVisible(Owner) &&
            bounds.Width > 0 && bounds.Height > 0;
        if (!SetWindowPos(Handle, HWND_TOP, bounds.X, bounds.Y, bounds.Width, bounds.Height,
            SWP_NOACTIVATE | (visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW)))
        {
            Fail("host-bounds", HRESULT_FROM_WIN32(GetLastError()));
            return false;
        }
        if (Controller)
        {
            Controller->NotifyParentWindowPositionChanged();
            return Present(visible);
        }
        return true;
    }
    HRESULT Focus()
    {
        if (Alive && Controller && Presented && !Failed && IsWindowVisible(Handle) &&
            GetForegroundWindow() == GetAncestor(Owner, GA_ROOT))
            return Controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
        return S_FALSE;
    }
    static LRESULT CALLBACK Procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto self = reinterpret_cast<State*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        if (message == WM_NCCREATE)
        {
            self = static_cast<State*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        if (self && message == WM_SIZE && self->Controller)
        {
            self->Present(self->Presented && wparam != SIZE_MINIMIZED && IsWindowVisible(window) && !IsIconic(self->Owner));
            return 0;
        }
        if (self && message == WM_SHOWWINDOW && self->Controller)
            self->Present(self->Presented && wparam != FALSE && !IsIconic(self->Owner));
        if (self && message == WM_MOVE && self->Controller)
            self->Controller->NotifyParentWindowPositionChanged();
        if (self && message == WM_CLOSE)
        {
            if (!self->Pending) { self->Actions.clear(); self->Actions.push_back({ActionKind::Close}); }
            return 0;
        }
        if (self && message == WM_SETFOCUS) self->Focus();
        if (self && message == WM_NCDESTROY)
        {
            SetWindowLongPtrW(window, GWLP_USERDATA, 0);
            self->Handle = nullptr;
            self->Fail("host-destroyed");
        }
        return DefWindowProcW(window, message, wparam, lparam);
    }
};

std::weak_ptr<Window::State> Window::active_;
Window::Window() = default;
Window::~Window() { Close(); }
bool Window::IsOpen() const { return state_ && state_->Alive; }

bool Window::BlocksGameInput(void* owner)
{
    const auto state = active_.lock();
    return state && state->Alive && state->Owner == owner && state->Presented && IsWindow(state->Handle);
}

bool Window::HandleOwnerMessage(void* owner, unsigned int message, std::uintptr_t wparam)
{
    const auto state = active_.lock();
    if (!state || !state->Alive || state->Owner != owner) return false;
    switch (message)
    {
    case WM_SIZE:
    case WM_WINDOWPOSCHANGED:
        state->SyncOwner();
        break;
    case WM_SHOWWINDOW:
        if (wparam) state->SyncOwner();
        else if (state->Controller) state->Present(false);
        break;
    case WM_SETFOCUS:
        state->Focus();
        break;
    }
    if (!BlocksGameInput(owner)) return false;
    if (message == WM_SETCURSOR)
    {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return true;
    }
    // Keep window activation, sizing, shutdown and network messages flowing.
    // The WebView's own child messages still reach its normal message handlers.
    return (message >= WM_KEYFIRST && message <= WM_KEYLAST) ||
        (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
        (message >= WM_IME_STARTCOMPOSITION && message <= WM_IME_COMPOSITION) ||
        (message >= WM_IME_SETCONTEXT && message <= WM_IME_KEYUP) ||
        message == WM_INPUT || message == WM_COMMAND || message == WM_HSCROLL || message == WM_VSCROLL;
}

bool Window::Open(void* owner, bool portuguese, const char* account)
{
    Close();
    if (!IsWindow(static_cast<HWND>(owner)) || !active_.expired()) return false;
    auto state = std::make_shared<State>();
    state->Portuguese = portuguese;
    if (account)
    {
        const int count = static_cast<int>(strnlen_s(account, 16));
        const int length = MultiByteToWideChar(1252, 0, account, count, nullptr, 0);
        state->Account.resize(length);
        if (length) MultiByteToWideChar(1252, 0, account, count, state->Account.data(), length);
    }
    state->Owner = static_cast<HWND>(owner);
    WNDCLASSW wc{};
    wc.lpfnWndProc = State::Procedure;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"WYD.PremiumNell";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    RECT parent{};
    if (!GetClientRect(state->Owner, &parent)) return false;
    const auto bounds = FitEmbedded(parent.right, parent.bottom);
    state->Handle = CreateWindowExW(0, wc.lpszClassName, L"Premium Nell", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        bounds.X, bounds.Y, bounds.Width, bounds.Height,
        static_cast<HWND>(owner), nullptr, wc.hInstance, state.get());
    if (!state->Handle) return false;
    state_ = state;
    active_ = state;
    // The game's UI thread owns COM and all browser callbacks. Never pump a
    // nested message loop or capture a scene across asynchronous completion.
    const HRESULT apartment = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(apartment)) { Close(); return false; }
    state->Apartment = true;
    PWSTR local = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) { Close(); return false; }
    const std::wstring profile = std::wstring(local) + L"\\WYD-Go\\PremiumNell";
    CoTaskMemFree(local);
    std::weak_ptr<State> weak = state;
    const HRESULT started = CreateCoreWebView2EnvironmentWithOptions(nullptr, profile.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [weak](HRESULT error, ICoreWebView2Environment* environment) -> HRESULT
        {
            auto s = weak.lock();
            if (!s || !s->Alive || s->Failed || !IsWindow(s->Handle)) return S_OK;
            if (FAILED(error) || !environment) { s->Fail("environment", error); return S_OK; }
            const HRESULT created = environment->CreateCoreWebView2Controller(s->Handle,
                Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [weak](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                {
                    auto current = weak.lock();
                    if (!current || !current->Alive || current->Failed || !IsWindow(current->Handle))
                    { if (controller) controller->Close(); return S_OK; }
                    if (FAILED(result) || !controller) { current->Fail("controller", result); return S_OK; }
                    current->Controller = controller;
                    if (!current->Present(false)) return S_OK;
                    const HRESULT viewResult = controller->get_CoreWebView2(&current->View);
                    if (FAILED(viewResult)) { current->Fail("view", viewResult); return S_OK; }
                    ComPtr<ICoreWebView2Settings> settings;
                    if (SUCCEEDED(current->View->get_Settings(&settings)))
                    {
                        settings->put_AreDevToolsEnabled(FALSE);
                        settings->put_AreDefaultContextMenusEnabled(FALSE);
                        settings->put_IsStatusBarEnabled(FALSE);
                        settings->put_AreDefaultScriptDialogsEnabled(FALSE);
                        settings->put_IsWebMessageEnabled(TRUE);
                        ComPtr<ICoreWebView2Settings3> keyboard;
                        if (SUCCEEDED(settings.As(&keyboard))) keyboard->put_AreBrowserAcceleratorKeysEnabled(FALSE);
                    }
                    EventRegistrationToken token{};
                    current->View->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
                        [weak](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
                        {
                            UINT64 id = 0;
                            if (FAILED(args->get_NavigationId(&id))) return S_OK;
                            auto s = weak.lock();
                            if (!s || !s->Navigation.IsDocument(id)) return S_OK;
                            BOOL success = FALSE; args->get_IsSuccess(&success);
                            WYD748_DiagnosticsLog("Premium Nell document completed success=%d\r\n", success);
                            if (!success)
                            {
                                COREWEBVIEW2_WEB_ERROR_STATUS status{}; args->get_WebErrorStatus(&status);
                                WYD748_DiagnosticsLog("Premium Nell navigation failed status=%d\r\n", static_cast<int>(status));
                                s->Fail("navigation");
                            }
                            return S_OK;
                        }).Get(), &token);
                    current->View->add_PermissionRequested(Callback<ICoreWebView2PermissionRequestedEventHandler>(
                        [](ICoreWebView2*, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT
                        { args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY); return S_OK; }).Get(), &token);
                    current->View->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
                        [weak](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT
                        {
                            PWSTR uri = nullptr;
                            UINT64 id = 0;
                            auto s = weak.lock();
                            if (SUCCEEDED(args->get_Uri(&uri)))
                            {
                                const bool allowed = s && s->Alive && SUCCEEDED(args->get_NavigationId(&id)) &&
                                    s->Navigation.Admit(uri, id);
                                args->put_Cancel(!allowed);
                                CoTaskMemFree(uri);
                            }
                            else args->put_Cancel(TRUE);
                            return S_OK;
                        }).Get(), &token);
                    current->View->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>(
                        [](ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT
                        { args->put_Handled(TRUE); return S_OK; }).Get(), &token);
                    current->View->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                        [weak](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                        {
                            auto target = weak.lock();
                            if (!target || !target->Alive || target->Failed || !target->Navigation.Started()) return S_OK;
                            PWSTR source = nullptr, message = nullptr;
                            if (FAILED(args->get_Source(&source))) return S_OK;
                            const bool trusted = wcscmp(source, L"about:blank") == 0;
                            CoTaskMemFree(source);
                            if (!trusted || FAILED(args->TryGetWebMessageAsString(&message))) return S_OK;
                            const Action action = ParseAction(message);
                            CoTaskMemFree(message);
                            if (action.Kind == ActionKind::Ready && !target->Ready)
                            {
                                target->Ready = true;
                                WYD748_DiagnosticsLog("Premium Nell document ready elapsed_ms=%llu\r\n", GetTickCount64() - target->Started);
                                target->Actions.push_back(action);
                            }
                            else if (target->Ready && !target->Pending && target->Actions.empty() &&
                                (action.Kind == ActionKind::Buy || action.Kind == ActionKind::Close))
                            {
                                // Latch at receipt, before a second queued click can enter.
                                if (action.Kind == ActionKind::Buy) target->Pending = true;
                                target->Actions.push_back(action);
                            }
                            return S_OK;
                        }).Get(), &token);
                    current->View->add_ProcessFailed(Callback<ICoreWebView2ProcessFailedEventHandler>(
                        [weak](ICoreWebView2*, ICoreWebView2ProcessFailedEventArgs* args) -> HRESULT
                        {
                            COREWEBVIEW2_PROCESS_FAILED_KIND kind{}; args->get_ProcessFailedKind(&kind);
                            WYD748_DiagnosticsLog("Premium Nell process failed kind=%d\r\n", static_cast<int>(kind));
                            if (auto s = weak.lock()) s->Fail("process"); return S_OK;
                        }).Get(), &token);
                    HMODULE module = GetModuleHandleW(nullptr);
                    HRSRC resource = FindResourceW(module, MAKEINTRESOURCEW(IDR_PREMIUM_NELL), MAKEINTRESOURCEW(10));
                    HGLOBAL loaded = resource ? LoadResource(module, resource) : nullptr;
                    const char* bytes = loaded ? static_cast<const char*>(LockResource(loaded)) : nullptr;
                    const int size = resource ? static_cast<int>(SizeofResource(module, resource)) : 0;
                    std::string document = bytes ? std::string(bytes, size) : std::string();
                    if (!bytes || !EmbedArt(module, document)) { current->Fail("art-resource"); return S_OK; }
                    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, document.data(), static_cast<int>(document.size()), nullptr, 0);
                    if (!length) { current->Fail("resource", HRESULT_FROM_WIN32(GetLastError())); return S_OK; }
                    std::wstring html(length, L'\0');
                    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, document.data(), static_cast<int>(document.size()), html.data(), length);
                    current->Navigation.Prepare(document);
                    const HRESULT navigationResult = current->View->NavigateToString(html.c_str());
                    if (FAILED(navigationResult)) current->Fail("navigate-start", navigationResult);
                    return S_OK;
                }).Get());
            if (FAILED(created)) s->Fail("controller-start", created);
            return S_OK;
        }).Get());
    if (FAILED(started)) { Close(); return false; }
    return true;
}

void Window::Close()
{
    if (!state_) return;
    auto state = std::move(state_);
    if (active_.lock() == state) active_.reset();
    const HWND focus = GetFocus();
    const bool restoreFocus = state->Handle && (focus == state->Handle || IsChild(state->Handle, focus)) &&
        GetForegroundWindow() == GetAncestor(state->Owner, GA_ROOT);
    state->Alive = false;
    if (state->Controller) state->Controller->Close();
    state->View.Reset(); state->Controller.Reset();
    if (state->Handle) DestroyWindow(state->Handle);
    if (restoreFocus && IsWindow(state->Owner)) SetFocus(state->Owner);
    if (state->Apartment) CoUninitialize();
}

Action Window::Poll()
{
    if (!state_) return {};
    if (!state_->Ready && GetTickCount64() - state_->Started > 15000) state_->Fail("ready-timeout", HRESULT_FROM_WIN32(WAIT_TIMEOUT));
    if (state_->Actions.empty()) return {};
    Action action = state_->Actions.front(); state_->Actions.pop_front();
    if (action.Kind == ActionKind::Ready)
    {
        if (!state_->Snapshot.empty())
        {
            const HRESULT posted = state_->View->PostWebMessageAsJson(state_->Snapshot.c_str());
            if (FAILED(posted)) { state_->Fail("initial-snapshot", posted); return Poll(); }
        }
        // A disabled parent also disables its children. Capture gameplay input
        // in the game loop while leaving the embedded browser interactive.
        state_->Presented = true;
        if (GetCapture() == state_->Owner) ReleaseCapture();
        if (!state_->SyncOwner()) return Poll();
        const HRESULT focused = state_->Focus();
        BOOL visible = FALSE;
        RECT bounds{};
        const HRESULT visibility = state_->Controller->get_IsVisible(&visible);
        const HRESULT size = state_->Controller->get_Bounds(&bounds);
        WYD748_DiagnosticsLog("Premium Nell presented visible=%d bounds=%ldx%ld visibility_hr=0x%08lX bounds_hr=0x%08lX focus_hr=0x%08lX\r\n",
            visible, bounds.right - bounds.left, bounds.bottom - bounds.top,
            static_cast<unsigned long>(visibility), static_cast<unsigned long>(size), static_cast<unsigned long>(focused));
    }
    return action;
}

void Window::Update(const std::array<Item, 27>& items, std::uint32_t balance, bool pending, int result)
{
    if (!state_) return;
    state_->Pending = pending;
    std::wstring json = L"{\"locale\":\"" + std::wstring(state_->Portuguese ? L"PT" : L"EN") +
        L"\",\"account\":" + Quote(state_->Account) + L",\"balance\":" + std::to_wstring(balance) + L",\"pending\":" + (pending ? L"true" : L"false") +
        L",\"result\":" + std::to_wstring(result) + L",\"items\":[";
    bool first = true;
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        const auto& item = items[i];
        if (!item.ID || !item.Price) continue;
        if (!first) json += L',';
        first = false;
        json += L"{\"slot\":" + std::to_wstring(i) + L",\"id\":" + std::to_wstring(item.ID) +
            L",\"name\":" + Quote(item.Name) + L",\"price\":" + std::to_wstring(item.Price) +
            L",\"category\":" + std::to_wstring(item.Category) +
            L",\"quantity\":" + std::to_wstring(item.Quantity) + L",\"refinement\":" + std::to_wstring(item.Refinement) +
            ItemDetailsJSON(item) + L"}";
    }
    state_->Snapshot = json + L"]}";
    if (state_->Ready)
    {
        const HRESULT posted = state_->View->PostWebMessageAsJson(state_->Snapshot.c_str());
        if (FAILED(posted)) state_->Fail("snapshot", posted);
    }
}
}
