#pragma once

class SControlContainer;

// Installs the opt-in diagnostics used while porting the 7.48 client.  The
// handler writes a compact crash report and a minidump without changing the
// normal gameplay path or requiring a debugger to be attached.
void WYD748_InstallDiagnostics();

// Appends a timestamped diagnostic line to client-debug.log.  This is kept
// separate from TMLog because the exception handler may run while TMLog is
// already being torn down.
void WYD748_DiagnosticsLog(const char* format, ...);

// Dumps the loaded control tree so legacy FieldScene2 IDs can be mapped to the
// semantic controls used by the imported scene code instead of guessed IDs.
void WYD748_DumpControlTree(SControlContainer* container, const char* reason);
