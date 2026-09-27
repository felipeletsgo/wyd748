// Minimal translation unit created by the TMProject Win32 template.
//
// This unit does not declare game state, protocol, or bootstrap logic.
// It bundles precompiled headers, Win32 base, and executable resources.
// Active bootstrap belongs to the existing client flow; a second WinMain
// here would introduce an invalid concurrent lifecycle.

#include "pch.h"
#include "framework.h"
#include "cmd/client/TMProject.h"

// Request high-performance discrete GPU on systems with hybrid graphics (NVIDIA Optimus / AMD PowerXpress).
// Graphics drivers query these exported symbols upon loading the executable to select the dedicated GPU.
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
