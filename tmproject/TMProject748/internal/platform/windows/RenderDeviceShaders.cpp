#include "pch.h"
// RenderDevice split by responsibility; device lifecycle stays in RenderDevice.cpp.
#include "RenderDevice.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMSky.h"
#include "TMFieldScene.h"
#include "TMCamera.h"
#include "TMMesh.h"
#include "ShaderResources.h"

namespace {
struct ShaderFileIO {
    int Open(const char* path) { return _open(path, _O_RDONLY | _O_BINARY); }
    __int64 Length(int handle) { return _filelengthi64(handle); }
    int Read(int handle, void* destination, unsigned int size) { return _read(handle, destination, size); }
    int Close(int handle) { return _close(handle); }
};

HRESULT ReadShader(const char* path, DWORD stage, ID3DXBuffer** output)
{
    ShaderFileIO io;
    const HRESULT result = shader_resources::Read(io, path, stage,
        [](UINT size, ID3DXBuffer** buffer) { return D3DXCreateBuffer(size, buffer); }, output);
    if (FAILED(result))
        LOG_WRITELOG("Shader read failed: %s (0x%08lX)\r\n", path, static_cast<unsigned long>(result));
    return result;
}
}

int RenderDevice::InitVertexShader()
{
	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 VertexDecl1[5] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 4, 0, 2, 0 },
		{ 0, 16, 2, 0, 3, 0 },
		{ 0, 28, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};
	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 VertexDecl2[6] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 0, 0, 1, 0 },
		{ 0, 16, 4, 0, 2, 0 },
		{ 0, 20, 2, 0, 3, 0 },
		{ 0, 32, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};
	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 VertexDecl3[6] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 1, 0, 1, 0 },
		{ 0, 20, 4, 0, 2, 0 },
		{ 0, 24, 2, 0, 3, 0 },
		{ 0, 36, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};
	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 VertexDecl4[6] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 1, 0 },
		{ 0, 24, 4, 0, 2, 0 },
		{ 0, 28, 2, 0, 3, 0 },
		{ 0, 40, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};


	LPD3DVERTEXELEMENT9 VertexDecl[4];
	VertexDecl[0] = VertexDecl1;
	VertexDecl[1] = VertexDecl2;
	VertexDecl[2] = VertexDecl3;
	VertexDecl[3] = VertexDecl4;

	D3DCAPS9 d3dCaps{};
	if (!m_pd3dDevice || FAILED(m_pd3dDevice->GetDeviceCaps(&d3dCaps)))
		return 0;
	const bool hardware = d3dCaps.VertexShaderVersion >= 0xFFFE0101;
	const bool rebuildSkin = !shader_resources::Batch<IDirect3DVertexShader9, 8>::Complete(m_pVertexShader) ||
		!shader_resources::Batch<IDirect3DVertexDeclaration9, 8>::Complete(m_pVertexDeclaration);
	shader_resources::Batch<IDirect3DVertexShader9, 8> skin;
	shader_resources::Batch<IDirect3DVertexDeclaration9, 8> skinDeclarations;

	if (rebuildSkin)
	{
		for (int i = 0; i < 8; ++i)
		{
			shader_resources::Owned<ID3DXBuffer> code;
			char szBinFile[128];
			sprintf_s(szBinFile, ShaderSkinMesh_Path, i + 1);
			if (FAILED(ReadShader(szBinFile, 0xFFFE0000UL, &code.value)))
				return 0;
			if (FAILED(m_pd3dDevice->CreateVertexDeclaration(VertexDecl[i % 4], skinDeclarations.Output(i))))
				return 0;
			if (FAILED(m_pd3dDevice->CreateVertexShader(
				static_cast<const DWORD*>(code.value->GetBufferPointer()), skin.Output(i))))
				return 0;
		}

		D3DXMATRIX mat;
		D3DXMatrixTranspose(&mat, &m_matProj);
		if (FAILED(m_pd3dDevice->SetVertexShaderConstantF(2, (const float*)&mat, 4)))
			return 0;
	}

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclWater[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 4, 0, 10, 0 },
		{ 0, 16, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclEquip[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclEquipShadow[2] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclBlur[3] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclMesh[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 4, 0, 10, 0 },
		{ 0, 16, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclSwing[6] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 4, 0, 10, 0 },
		{ 0, 16, 1, 0, 5, 0 },
		{ 0, 24, 1, 0, 5, 1 },
		{ 0, 32, 1, 0, 5, 2 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclEfMesh[3] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclBumpEquip[5] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 1, 0, 5, 0 },
		{ 0, 32, 2, 0, 6, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclEnvMesh[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclBumpMesh[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclUV2Mesh[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 1, 0, 5, 0 },
		{ 0, 20, 1, 0, 5, 1 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclWaterFallMesh[5] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 4, 0, 10, 0 },
		{ 0, 28, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclLODMesh[3] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclNormalMesh[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclSpecNormalMesh[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 1, 0, 5, 0 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclSpecWaterFall2Mesh[5] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 4, 0, 10, 0 },
		{ 0, 16, 1, 0, 5, 0 },
		{ 0, 24, 1, 0, 5, 1 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclSpecWater[5] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 2, 0, 3, 0 },
		{ 0, 24, 1, 0, 5, 0 },
		{ 0, 32, 1, 0, 5, 1 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclChannel2Alpha[4] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 1, 0, 5, 0 },
		{ 0, 20, 1, 0, 5, 1 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	// { Stream, Offset, Type, Method, Usage, UsageIndex }
	D3DVERTEXELEMENT9 DeclRefectWater[5] =
	{
		{ 0, 0, 2, 0, 0, 0 },
		{ 0, 12, 4, 0, 10, 0 },
		{ 0, 16, 1, 0, 5, 0 },
		{ 0, 24, 1, 0, 5, 1 },
		{ 255, 0, 17, 0, 0, 0 }
	};

	LPD3DVERTEXELEMENT9 VertexDeclEff[19];
	VertexDeclEff[0] = DeclWater;
	VertexDeclEff[1] = DeclEquip;
	VertexDeclEff[2] = DeclEquipShadow;
	VertexDeclEff[3] = DeclBlur;
	VertexDeclEff[4] = DeclMesh;
	VertexDeclEff[5] = DeclSwing;
	VertexDeclEff[6] = DeclEfMesh;
	VertexDeclEff[7] = DeclBumpEquip;
	VertexDeclEff[8] = DeclEnvMesh;
	VertexDeclEff[9] = DeclBumpMesh;
	VertexDeclEff[10] = DeclUV2Mesh;
	VertexDeclEff[11] = DeclWaterFallMesh;
	VertexDeclEff[12] = DeclLODMesh;
	VertexDeclEff[13] = DeclNormalMesh;
	VertexDeclEff[14] = DeclSpecNormalMesh;
	VertexDeclEff[15] = DeclSpecWaterFall2Mesh;
	VertexDeclEff[16] = DeclSpecWater;
	VertexDeclEff[17] = DeclChannel2Alpha;
	VertexDeclEff[18] = DeclRefectWater;

	const bool rebuildEffects = !shader_resources::Batch<IDirect3DVertexShader9, 4>::Complete(m_pVSEffect) ||
		!shader_resources::Batch<IDirect3DVertexDeclaration9, 4>::Complete(m_pVDEffect);
	shader_resources::Batch<IDirect3DVertexShader9, 4> effects;
	shader_resources::Batch<IDirect3DVertexDeclaration9, 4> effectDeclarations;
	if (rebuildEffects)
	{
		for (int j = 0; j < 4; ++j)
		{
			shader_resources::Owned<ID3DXBuffer> code;
			char szFileName[128];
			sprintf_s(szFileName, ShaderVertexShader_Path, j + 1);
			if (FAILED(ReadShader(szFileName, 0xFFFE0000UL, &code.value)))
				return 0;
			if (FAILED(m_pd3dDevice->CreateVertexDeclaration(VertexDeclEff[j], effectDeclarations.Output(j))))
				return 0;
			if (FAILED(m_pd3dDevice->CreateVertexShader(
				static_cast<const DWORD*>(code.value->GetBufferPointer()), effects.Output(j))))
				return 0;
		}
	}
	if (rebuildSkin && (!skin.Complete() || !skinDeclarations.Complete())) return 0;
	if (rebuildEffects && (!effects.Complete() || !effectDeclarations.Complete())) return 0;
	if (rebuildSkin) {
		skin.Publish(m_pVertexShader);
		skinDeclarations.Publish(m_pVertexDeclaration);
		LOG_WRITELOG(hardware ? "VertexShader HW Accel Enabled\r\n" : "VertexShader SW Mode\r\n");
	}
	if (rebuildEffects) {
		effects.Publish(m_pVSEffect);
		effectDeclarations.Publish(m_pVDEffect);
	}
	m_bUseSW = hardware ? 0 : 16;
	if (hardware) m_dwBufferUsage = 8;
	return 1;
}

int RenderDevice::InitPixelShader()
{
	D3DCAPS9 d3dCaps{};
	if (!m_pd3dDevice || FAILED(m_pd3dDevice->GetDeviceCaps(&d3dCaps)))
		return 0;
	if (d3dCaps.PixelShaderVersion >= 0xFFFF0101)
	{
		if (!shader_resources::Batch<IDirect3DPixelShader9, 6>::Complete(m_pPSEffect))
		{
			shader_resources::Batch<IDirect3DPixelShader9, 6> pixels;
			for (int i = 0; i < 6; ++i)
			{
				shader_resources::Owned<ID3DXBuffer> code;
				char szBinFile[128];
				sprintf_s(szBinFile, ShaderPixelShader_Path, i + 1);
				if (FAILED(ReadShader(szBinFile, 0xFFFF0000UL, &code.value)))
					return 0;
				if (FAILED(m_pd3dDevice->CreatePixelShader(
					static_cast<const DWORD*>(code.value->GetBufferPointer()), pixels.Output(i))))
					return 0;
			}
			if (!pixels.Complete()) return 0;
			pixels.Publish(m_pPSEffect);
		}

		return 1;
	}
	else
	{
		m_bSupportPS11 = 0;
		m_nShadowTextureSize = 64;
		return 1;
	}

	return 1;
}
