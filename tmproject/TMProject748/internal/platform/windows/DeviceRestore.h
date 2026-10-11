#pragma once
#include <windows.h>

namespace device_restore {
template<class Font, class InitializeStates, class InitializeShaders>
HRESULT Complete(Font* normal, Font* large, InitializeStates states, InitializeShaders shaders)
{
    if (normal) {
        const HRESULT result = normal->OnResetDevice();
        if (FAILED(result)) return result;
    }
    if (large) {
        const HRESULT result = large->OnResetDevice();
        if (FAILED(result)) return result;
    }
    if (!states()) return E_FAIL;
    if (!shaders()) return E_FAIL;
    return S_OK;
}
}
