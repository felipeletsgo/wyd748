#pragma once

namespace device_mode_family {
// True when enumeration selected a complete adapter, device and combination
// for the windowed (true) or fullscreen (false) family. Accessors of the
// other family must not run while it is absent.
template<class Settings>
bool Has(const Settings& settings, bool windowed)
{
    return windowed
        ? settings.pWindowed_AdapterInfo != nullptr && settings.pWindowed_DeviceInfo != nullptr &&
            settings.pWindowed_DeviceCombo != nullptr
        : settings.pFullscreen_AdapterInfo != nullptr && settings.pFullscreen_DeviceInfo != nullptr &&
            settings.pFullscreen_DeviceCombo != nullptr;
}

// A mode toggle needs the current family and the opposite family.
template<class Settings>
bool CanToggle(const Settings& settings, bool currentWindowed)
{
    return Has(settings, settings.IsWindowed) && Has(settings, !currentWindowed);
}
}
