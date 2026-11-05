#pragma once

#include <d3d9.h>

namespace effect_render_state
{
// Device state that effects restore after drawing, as in
// TMEffectBillBoard::Render. modulatedAlpha restores texture stage 0 alpha
// after a draw that selected D3DTOP_MODULATE.
template <typename Device>
void RestoreDefaults(Device& device, bool modulatedAlpha)
{
    device.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    device.SetRenderState(D3DRS_LIGHTING, TRUE);
    device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    device.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    device.SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    device.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    if (modulatedAlpha)
        device.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
}
}
