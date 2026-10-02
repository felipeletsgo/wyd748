#include "pch.h"
// RenderDevice split by responsibility; device lifecycle stays in RenderDevice.cpp.
#include "RenderDevice.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMSky.h"
#include "TMFieldScene.h"
#include "TMCamera.h"
#include "TMMesh.h"

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

	m_bUseSW = 16;
	D3DCAPS9 d3dCaps{};
	m_pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (d3dCaps.VertexShaderVersion >= 0xFFFE0101)
	{
		m_bUseSW = 0;
		m_dwBufferUsage = 8;
		if (m_pVertexShader[0] == nullptr)
			LOG_WRITELOG("VertexShader HW Accel Enabled\r\n");
	}
	else if (m_pVertexShader[0] == nullptr)
	{
		LOG_WRITELOG("VertexShader SW Mode \r\n");
	}

	if (m_pVertexShader[0] == nullptr)
	{
		for (int i = 0; i < 8; ++i)
		{
			LPD3DXBUFFER pCode = nullptr;
			char szBinFile[128];
			sprintf_s(szBinFile, ShaderSkinMesh_Path, i + 1);

			int handle = _open(szBinFile, _O_BINARY);
			if (handle == -1)
			{
				LOG_WRITELOG("Read VertexShader %d Error.. \r\n", i + 1);
				return 0;
			}

			int nLength = _filelength(handle);
			D3DXCreateBuffer(nLength, &pCode);
			_read(handle, pCode->GetBufferPointer(), pCode->GetBufferSize());
			_close(handle);

			if (FAILED(m_pd3dDevice->CreateVertexDeclaration(VertexDecl[i % 4], &m_pVertexDeclaration[i])))
				return 0;

			if (FAILED(m_pd3dDevice->CreateVertexShader((const DWORD*)pCode->GetBufferPointer(), &m_pVertexShader[i])))
				return 0;

			pCode->Release();
		}

		D3DXMATRIX mat;
		D3DXMatrixTranspose(&mat, &m_matProj);
		m_pd3dDevice->SetVertexShaderConstantF(2, (const float*)&mat, 4);
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

	if (m_pVSEffect[0] == nullptr)
	{
		for (int j = 0; j < 4; ++j)
		{
			LPD3DXBUFFER pCode = nullptr;
			char szFileName[128];
			sprintf_s(szFileName, ShaderVertexShader_Path, j + 1);

			int fh = _open(szFileName, _O_BINARY);
			if (fh == -1)
			{
				LOG_WRITELOG("Read VertexShader %d Error.. \r\n", j + 1);
				return 0;
			}

			int nLength = _filelength(fh);
			D3DXCreateBuffer(nLength, &pCode);
			_read(fh, pCode->GetBufferPointer(), pCode->GetBufferSize());
			_close(fh);

			if (FAILED(m_pd3dDevice->CreateVertexDeclaration(VertexDeclEff[j], &m_pVDEffect[j])))
				return 0;

			if (FAILED(m_pd3dDevice->CreateVertexShader((const DWORD*)pCode->GetBufferPointer(), &m_pVSEffect[j])))
				return 0;

			pCode->Release();
		}
	}

	return 1;
}

int RenderDevice::InitPixelShader()
{
	D3DCAPS9 d3dCaps{};
	m_pd3dDevice->GetDeviceCaps(&d3dCaps);
	if (d3dCaps.PixelShaderVersion >= 0xFFFF0101)
	{
		if (m_pPSEffect[0] == nullptr)
		{
			for (int i = 0; i < 6; ++i)
			{
				LPD3DXBUFFER pCode = nullptr;
				char szBinFile[128];
				sprintf_s(szBinFile, ShaderPixelShader_Path, i + 1);

				int handle = _open(szBinFile, _O_BINARY);
				if (handle == -1)
				{
					LOG_WRITELOG("Read PixelShader %d Error.. \r\n", i + 1);
					return 0;
				}

				int nLength = _filelength(handle);
				D3DXCreateBuffer(nLength, &pCode);
				_read(handle, pCode->GetBufferPointer(), pCode->GetBufferSize());
				_close(handle);

				if (FAILED(m_pd3dDevice->CreatePixelShader((const DWORD*)pCode->GetBufferPointer(), &m_pPSEffect[i])))
					return 0;

				pCode->Release();
			}
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
