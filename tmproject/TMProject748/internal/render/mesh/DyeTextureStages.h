#pragma once

#include <d3d9.h>

namespace dye_texture_stages
{
// WYD 7.48 FUN_004c3eec uses COLOROP 0x18 (DOTPRODUCT3), then
// 6 (MODULATE4X), or 4 (MODULATE) for black. These are not the adjacent
// MULTIPLYADD/ADD enum values. Preserve the animated second UV channel.
template <typename Device>
void Apply(Device& device, short legend, char alpha, bool legacyAdapter, bool tntAdapter = false)
{
    const bool black = legend == 120;
    device.SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    device.SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
    device.SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_CURRENT);
    device.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    device.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    device.SetTextureStageState(0, D3DTSS_COLOROP,
        legacyAdapter || tntAdapter ? D3DTOP_MODULATE : D3DTOP_DOTPRODUCT3);
    device.SetTextureStageState(1, D3DTSS_COLOROP,
        legacyAdapter || tntAdapter ? D3DTOP_ADD : (black ? D3DTOP_MODULATE : D3DTOP_MODULATE4X));
    if (alpha != 'C' || legacyAdapter)
    {
        device.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}
}
