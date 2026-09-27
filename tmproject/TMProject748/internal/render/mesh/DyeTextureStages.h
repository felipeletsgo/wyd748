#pragma once

#include <d3d9.h>

namespace dye_texture_stages
{
// WYD 7.48 FUN_004c3eec: colored dyes add their effect texture;
// black (120) modulates it. Use the animated second UV channel.
template <typename Device>
void Apply(Device& device, short legend, char alpha, bool legacyAdapter)
{
    device.SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    device.SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
    device.SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_CURRENT);
    device.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    device.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    device.SetTextureStageState(0, D3DTSS_COLOROP,
        legacyAdapter ? D3DTOP_MODULATE : D3DTOP_MULTIPLYADD);
    device.SetTextureStageState(1, D3DTSS_COLOROP,
        legacyAdapter ? D3DTOP_ADDSIGNED : (legend == 120 ? D3DTOP_MODULATE : D3DTOP_ADD));
    if (alpha != 'C' || legacyAdapter)
    {
        device.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}
}
