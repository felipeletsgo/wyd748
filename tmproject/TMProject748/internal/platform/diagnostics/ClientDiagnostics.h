#pragma once

// Returns -1 for normal startup; otherwise the external crash monitor's exit code.
int WYD748_RunCrashMonitor();

#include <cstddef>

class SControlContainer;

// Prepares a dedicated minidump worker and reserves main-thread exception stack.
// The fault filter hands off the original thread/context without doing file I/O
// on an exhausted stack. Startup failures are explicitly logged.
void WYD748_InstallDiagnostics();

// Appends UTC time, process/thread IDs and elapsed milliseconds to client-debug.log.
// This log is separate from
// TMLog because the exception handler may run while TMLog is being torn down.
// The format follows printf; arguments are not retained.
void WYD748_DiagnosticsLog(const char* format, ...);

// Records only network metadata when WYD748_TRACE_PACKETS is enabled in the
// environment. Never persists packet payloads or buffers.
void WYD748_DiagnosticsPacket(const char* direction, unsigned int opcode, std::size_t size);

// Dumps the loaded control tree to correlate legacy FieldScene2 IDs with the
// imported source's semantic controls. The container is a valid non-owning
// reference; reason is only copied into the log.
void WYD748_DumpControlTree(SControlContainer* container, const char* reason);

// Layout probe for UI checks: when WYD748_LAYOUT_PROBE is enabled, logs the
// rectangle of every visible top-level window whenever that set changes,
// with the screen size and UI scale. Call once per frame; otherwise a no-op.
void WYD748_LayoutProbe(SControlContainer* container, unsigned int screenWidth,
    unsigned int screenHeight, float uiScale);
