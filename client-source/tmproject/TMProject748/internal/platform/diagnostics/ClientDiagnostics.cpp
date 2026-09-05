#include "pch.h"
#include "ClientDiagnostics.h"
#include "SControlContainer.h"
#include "SControl.h"

#include <DbgHelp.h>
#include <algorithm>
#include <cstdarg>
#include <string>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr char kDiagnosticsFile[] = "client-debug.log";

    const char* ControlTypeName(const CONTROL_TYPE type)
    {
        // Keep the diagnostic vocabulary stable across builds so a captured
        // 7.48 window can be compared with its source and RC definition.
        switch (type)
        {
        case CONTROL_TYPE::CTRL_TYPE_CURSOR: return "cursor";
        case CONTROL_TYPE::CTRL_TYPE_PANEL: return "panel";
        case CONTROL_TYPE::CTRL_TYPE_BUTTON: return "button";
        case CONTROL_TYPE::CTRL_TYPE_CHECKBOX: return "checkbox";
        case CONTROL_TYPE::CTRL_TYPE_RADIOBUTTON: return "radio";
        case CONTROL_TYPE::CTRL_TYPE_RADIOBUTTONSET: return "radio-set";
        case CONTROL_TYPE::CTRL_TYPE_LISTBOX: return "list";
        case CONTROL_TYPE::CTRL_TYPE_LISTBOXITEM: return "list-item";
        case CONTROL_TYPE::CTRL_TYPE_MESSAGEBOX: return "message-box";
        case CONTROL_TYPE::CTRL_TYPE_MESSAGEPANEL: return "message-panel";
        case CONTROL_TYPE::CTRL_TYPE_PROGRESSBAR: return "progress";
        case CONTROL_TYPE::CTRL_TYPE_SCROLLBAR: return "scroll";
        case CONTROL_TYPE::CTRL_TYPE_TEXT: return "text";
        case CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT: return "edit";
        case CONTROL_TYPE::CTRL_TYPE_DIALOG: return "dialog";
        case CONTROL_TYPE::CTRL_TYPE_3DOBJ: return "3d-object";
        case CONTROL_TYPE::CTRL_TYPE_GRID: return "grid";
        default: return "none";
        }
    }

    std::string SanitizedLabel(const char* text)
    {
        std::string label = text ? text : "";
        std::replace(label.begin(), label.end(), '\r', ' ');
        std::replace(label.begin(), label.end(), '\n', ' ');
        std::replace(label.begin(), label.end(), '\t', ' ');
        if (label.size() > 80)
            label.resize(80);
        return label;
    }

    void CollectWindowLabels(TreeNode* node, std::vector<std::string>& labels,
        std::unordered_set<TreeNode*>& seen)
    {
        if (!node || labels.size() >= 16 || !seen.insert(node).second)
            return;

        auto control = dynamic_cast<SControl*>(node);
        if (control)
        {
            // Never persist editable text: it may contain an account name,
            // password, whisper or other player-supplied private value.
            if (control->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_TEXT)
            {
                auto text = static_cast<SText*>(control);
                const std::string label = SanitizedLabel(text->GetText());
                if (!label.empty())
                    labels.push_back(label);
            }
            else if (control->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_BUTTON)
            {
                auto button = static_cast<SButton*>(control);
                if (button->m_pAltText)
                {
                    const std::string label = SanitizedLabel(button->m_pAltText->GetText());
                    if (!label.empty())
                        labels.push_back(label);
                }
            }
        }

        for (TreeNode* child = node->m_pDown; child && labels.size() < 16; child = child->m_pNextLink)
            CollectWindowLabels(child, labels, seen);
    }

    void AppendLine(const char* text)
    {
        HANDLE file = CreateFileA(
            kDiagnosticsFile,
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file == INVALID_HANDLE_VALUE)
            return;

        DWORD written = 0;
        WriteFile(file, text, static_cast<DWORD>(strlen(text)), &written, nullptr);
        CloseHandle(file);
    }

    LONG WINAPI ClientUnhandledException(EXCEPTION_POINTERS* exceptionPointers)
    {
        char line[512]{};
        const DWORD code = exceptionPointers && exceptionPointers->ExceptionRecord
            ? exceptionPointers->ExceptionRecord->ExceptionCode
            : 0;
        const ULONG_PTR address = exceptionPointers && exceptionPointers->ExceptionRecord
            ? reinterpret_cast<ULONG_PTR>(exceptionPointers->ExceptionRecord->ExceptionAddress)
            : 0;
        sprintf_s(line, "Unhandled exception code=0x%08lX address=0x%p pid=%lu\r\n",
            code, reinterpret_cast<void*>(address), GetCurrentProcessId());
        AppendLine(line);

        // Resolve DbgHelp dynamically so the client remains launchable on a
        // clean 7.48 machine even when the SDK import library is unavailable.
        HMODULE dbghelp = LoadLibraryA("Dbghelp.dll");
        if (dbghelp)
        {
            using MiniDumpWriteDumpProc = BOOL(WINAPI*)(
                HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
                const MINIDUMP_EXCEPTION_INFORMATION*,
                const MINIDUMP_USER_STREAM_INFORMATION*,
                const MINIDUMP_CALLBACK_INFORMATION*);
            auto writeDump = reinterpret_cast<MiniDumpWriteDumpProc>(
                GetProcAddress(dbghelp, "MiniDumpWriteDump"));
            if (writeDump)
            {
                SYSTEMTIME now{};
                GetLocalTime(&now);
                char dumpName[MAX_PATH]{};
                sprintf_s(dumpName, "client-crash-%04u%02u%02u-%02u%02u%02u.dmp",
                    now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);
                HANDLE dump = CreateFileA(dumpName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL, nullptr);
                if (dump != INVALID_HANDLE_VALUE)
                {
                    MINIDUMP_EXCEPTION_INFORMATION info{};
                    info.ThreadId = GetCurrentThreadId();
                    info.ExceptionPointers = exceptionPointers;
                    info.ClientPointers = FALSE;
                    writeDump(GetCurrentProcess(), GetCurrentProcessId(), dump,
                        static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithDataSegs),
                        exceptionPointers ? &info : nullptr, nullptr, nullptr);
                    CloseHandle(dump);
                }
            }
            FreeLibrary(dbghelp);
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void DumpNode(TreeNode* node, int depth, std::unordered_set<TreeNode*>& seen)
    {
        if (!node || !seen.insert(node).second)
            return;

        auto control = dynamic_cast<SControl*>(node);
        if (control)
        {
            char line[512]{};
			auto parent = control->m_pTop ? dynamic_cast<SControl*>(control->m_pTop) : nullptr;
            sprintf_s(line,
                "UI depth=%d id=%u type=%s(%d) visible=%d enabled=%d modal=%d x=%.1f y=%.1f w=%.1f h=%.1f parent=%u\r\n",
                depth,
                control->GetControlID(),
                ControlTypeName(control->m_eCtrlType),
                static_cast<int>(control->m_eCtrlType),
                control->m_bVisible,
                control->m_bEnable,
                control->m_bModal,
                control->m_nPosX,
                control->m_nPosY,
                control->m_nWidth,
                control->m_nHeight,
                parent ? parent->GetControlID() : 0u);
            AppendLine(line);
        }

        for (TreeNode* child = node->m_pDown; child; child = child->m_pNextLink)
            DumpNode(child, depth + 1, seen);
    }

    void DumpTopLevelWindows(SControlContainer* container)
    {
        if (!container || !container->m_pControlRoot)
            return;

        // A top-level catalog records undocumented 7.48 panels (including GM
        // and event tools) without relying on screenshots or guessed IDs.
        for (TreeNode* node = container->m_pControlRoot->m_pDown; node; node = node->m_pNextLink)
        {
            auto window = dynamic_cast<SControl*>(node);
            if (!window)
                continue;

            std::vector<std::string> labels;
            std::unordered_set<TreeNode*> seen;
            CollectWindowLabels(node, labels, seen);

            std::string joined;
            for (const std::string& label : labels)
            {
                if (!joined.empty())
                    joined += " | ";
                joined += label;
            }

            WYD748_DiagnosticsLog(
                "UI window id=%u type=%s visible=%d enabled=%d x=%.1f y=%.1f w=%.1f h=%.1f labels=\"%s\"\r\n",
                window->GetControlID(),
                ControlTypeName(window->m_eCtrlType),
                window->m_bVisible,
                window->m_bEnable,
                window->m_nPosX,
                window->m_nPosY,
                window->m_nWidth,
                window->m_nHeight,
                joined.c_str());
        }
    }
}

void WYD748_InstallDiagnostics()
{
    // The filter is intentionally installed once by NewApp after logging has
    // started, so every compatibility crash produces evidence next to the
    // deployed executable rather than only a generic Windows event entry.
    SetUnhandledExceptionFilter(ClientUnhandledException);
    WYD748_DiagnosticsLog("diagnostics installed pid=%lu\r\n", GetCurrentProcessId());
}

void WYD748_DiagnosticsLog(const char* format, ...)
{
    if (!format)
        return;

    char line[1024]{};
    va_list args;
    va_start(args, format);
    vsprintf_s(line, format, args);
    va_end(args);
    AppendLine(line);
}

void WYD748_DumpControlTree(SControlContainer* container, const char* reason)
{
    if (!container)
    {
        WYD748_DiagnosticsLog("UI dump skipped reason=%s container=null\r\n", reason ? reason : "unknown");
        return;
    }

    WYD748_DiagnosticsLog("UI dump begin reason=%s\r\n", reason ? reason : "unknown");
    DumpTopLevelWindows(container);
    std::unordered_set<TreeNode*> seen;
    DumpNode(container->m_pControlRoot, 0, seen);
    WYD748_DiagnosticsLog("UI dump end controls=%zu\r\n", seen.size());
}
