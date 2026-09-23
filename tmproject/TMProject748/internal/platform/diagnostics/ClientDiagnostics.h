#pragma once

#include <cstddef>

class SControlContainer;

// Installs optional diagnostics for the 7.48 client adaptation. The handler
// writes a compact report and minidump without changing normal gameplay or
// requiring an attached debugger.
void WYD748_InstallDiagnostics();

// Appends a timestamped line to client-debug.log. This log is separate from
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
