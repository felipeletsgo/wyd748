#include "pch.h"
#include "TMMesh.h"
#include "MeshManager.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMPaths.h"
#include "../../application/AssetLoadValidation.h"
#include <climits>
static_assert(sizeof(D3DXMATRIX) == 64, "Animation extents require 64-byte matrices");

namespace
{
void ClearBoneAnimations()
{
	for (auto& animation : MeshManager::m_BoneAnimationList)
	{
		free(animation.pBone);
		free(animation.matAnimation);
		free(animation.matQuaternion);
		animation = {};
	}
	memset(MeshManager::m_sAnimationArray, 0, sizeof(MeshManager::m_sAnimationArray));
	memset(g_nMaxSizeOffset, 0, sizeof(g_nMaxSizeOffset));
	MeshManager::m_nFullCount = 0;
}
}

int g_nMaxSizeOffset[MAX_BONE_ANIMATION_LIST];

stAniSoundTable2 g_MobAniTableEx[4][2];
stAniSoundTable g_MobAniTable[60];

stBoneAni MeshManager::m_BoneAnimationList[MAX_BONE_ANIMATION_LIST]{};
short MeshManager::m_sAnimationArray[MAX_ANI_CLASS][MAX_ANI_TYPE][MAX_ANI_MOTION]{};
char MeshManager::m_aObjectMask[MAX_OBJECT_MASK][MAX_MASK_X][MAX_MASK_Y]{};
int MeshManager::m_nFullCount{0};

MeshManager::MeshManager()
{
	memset(m_stCommonMesh, 0, sizeof(m_stCommonMesh));
	memset(m_stSkinMeshList, 0, sizeof(m_stSkinMeshList));
	memset(m_stValidAniList, 0, sizeof(m_stValidAniList));

	for (int nClass = 0; nClass < MAX_ANI_CLASS; ++nClass)
	{
		for (int nAniTypeNum = 0; nAniTypeNum < MAX_ANI_TYPE; ++nAniTypeNum)
		{
			for (int nAniMotionNum = 0; nAniMotionNum < MAX_ANI_MOTION; ++nAniMotionNum)
			{
				MeshManager::m_sAnimationArray[nClass][nAniTypeNum][nAniMotionNum] = 0;
			}
		}
	}

	g_pMeshManager = this;
	m_LastLoadTime = 0;
}

MeshManager::~MeshManager()
{
	ReleaseMesh();
	ClearBoneAnimations();
	g_pMeshManager = nullptr;
}

int MeshManager::InitMeshManager()
{
	if (!InitBoneAnimation())
	{
		LOG_WRITELOG("Initialize Bone Animation Failed\r\n");
		return 0;
	}

	FILE* fp = nullptr;
	fopen_s(&fp, MeshList_Path, "rt");

	if (fp == nullptr)
		return 0;

	char line[512]{};
	for (;;)
	{
		const auto result = asset_load::ReadLine(fp, line);
		if (result == asset_load::LineResult::End) break;
		if (result == asset_load::LineResult::Blank) continue;
		int index, count, parts;
		std::string path;
		if (result == asset_load::LineResult::Invalid ||
			!asset_load::ParseList(line, index, path, count, parts, false, false,
				MAX_COMMON_MESH, sizeof(m_stCommonMesh[0].szFileName)))
		{ fclose(fp); return 0; }
		strcpy_s(m_stCommonMesh[index].szFileName, path.c_str());
	}

	fclose(fp);

	return ReadObjectMask();
}

int MeshManager::InitBoneAnimation()
{
	ClearBoneAnimations();

	char szFileName[128]{};
	strcpy_s(szFileName, BoneAni4_Path);

	FILE* fpList = nullptr;
	fopen_s(&fpList, szFileName, "rt");

	if (fpList == nullptr)
		return 0;
	const auto fail = [&]() { fclose(fpList); ClearBoneAnimations(); return 0; };

	FILE* fpValidAni = nullptr;
	fopen_s(&fpValidAni, ValidIndex_Path, "rb");

	if (fpValidAni != nullptr)
	{
		const bool complete = fread(m_stValidAniList, sizeof(m_stValidAniList), 1, fpValidAni) == 1;
		fclose(fpValidAni);
		if (!complete) return fail();
	}

	int nCount = 0;
	int nAniTypeCount = 0;
	int nParts = 0;

	char line[512]{};
	bool loaded[MAX_BONE_ANIMATION_LIST]{};
	for (;;)
	{
		int nFullTickCount = 0;
		const auto result = asset_load::ReadLine(fpList, line);
		if (result == asset_load::LineResult::End) break;
		if (result == asset_load::LineResult::Blank) continue;
		std::string path;
		if (result == asset_load::LineResult::Invalid ||
			!asset_load::ParseList(line, nCount, path, nAniTypeCount, nParts, false, true,
				MAX_BONE_ANIMATION_LIST, 116) || loaded[nCount]) return fail();
		loaded[nCount] = true;

		MeshManager::m_BoneAnimationList[nCount].numAniTypeCount = nAniTypeCount;
		MeshManager::m_BoneAnimationList[nCount].numParts = nParts;

		strcpy_s(MeshManager::m_BoneAnimationList[nCount].szAniName, path.c_str());

		char szTemp[128];
		sprintf_s(szTemp, "%s.bon", MeshManager::m_BoneAnimationList[nCount].szAniName);

		int handle = _open(szTemp, _O_BINARY);
		if (handle < 0) return fail();
		int sz = _filelength(handle);
		if (sz <= 0) { _close(handle); return fail(); }

		MeshManager::m_BoneAnimationList[nCount].pBone = (unsigned int*)malloc(sz);

		if (!MeshManager::m_BoneAnimationList[nCount].pBone ||
			_read(handle, MeshManager::m_BoneAnimationList[nCount].pBone, sz) != sz)
		{ _close(handle); return fail(); }
		_close(handle);

		MeshManager::m_BoneAnimationList[nCount].numBone = sz / 8;
		MeshManager::m_BoneAnimationList[nCount].numBoneBytes = static_cast<unsigned int>(sz);
		unsigned int dwFileIndex = 0;
		std::size_t allocationBytes = 0;
		unsigned int ticks[186]{}, frames[186]{};
		for (size_t nFI = 0; nFI < MeshManager::m_BoneAnimationList[nCount].numAniTypeCount; ++nFI)
		{
			if (m_stValidAniList[nCount][nFI].nI < 0 || m_stValidAniList[nCount][nFI].nI == INT_MAX) return fail();
			const int pathLength = snprintf(szTemp, sizeof(szTemp), "%s%04d.ani", MeshManager::m_BoneAnimationList[nCount].szAniName, m_stValidAniList[nCount][nFI].nI + 1);
			if (pathLength < 0 || pathLength >= sizeof(szTemp)) return fail();
			handle = _open(szTemp, _O_BINARY, 0);

			if (handle >= 0)
			{
				std::size_t bytes = 0;
				// Alternate resources remain loadable even when their legacy character alias is invalid.
				const bool valid = (nCount == 3 || nCount == 5 || asset_load::AnimationMapping(nCount, m_stValidAniList[nCount][nFI].nI)) &&
					_read(handle, &ticks[nFI], 4) == 4 && _read(handle, &frames[nFI], 4) == 4 &&
					asset_load::AnimationBytes(ticks[nFI], frames[nFI], MeshManager::m_BoneAnimationList[nCount].numAniFrame, bytes) &&
					_filelength(handle) >= 8 && static_cast<std::size_t>(_filelength(handle) - 8) == bytes &&
					allocationBytes <= static_cast<std::size_t>(INT_MAX) - bytes;
				_close(handle);
				if (!valid) return fail();
				MeshManager::m_BoneAnimationList[nCount].numAniFrame = frames[nFI];
				allocationBytes += bytes;
				nFullTickCount += ticks[nFI];
				++dwFileIndex;
			}
		}

		MeshManager::m_nFullCount = nFullTickCount;
		MeshManager::m_BoneAnimationList[nCount].matAnimation = allocationBytes ? (LPD3DXMATRIX)malloc(allocationBytes) : nullptr;
		if (allocationBytes && !MeshManager::m_BoneAnimationList[nCount].matAnimation) return fail();

		// This is used on characters animation/bone read. (ch01/02)
		if (nCount == 0 || nCount == 1)
		{
			MeshManager::m_BoneAnimationList[nCount].matQuaternion = allocationBytes ? (LPD3DXQUATERNION)malloc(allocationBytes / sizeof(D3DXMATRIX) * sizeof(D3DXQUATERNION)) : nullptr;
			if (allocationBytes && !MeshManager::m_BoneAnimationList[nCount].matQuaternion) return fail();
		}		
		
		dwFileIndex = 0;
		int nOffset = 0;

		// This loop is related with weapon animations
		for (size_t nFI = 0; nFI < MeshManager::m_BoneAnimationList[nCount].numAniTypeCount; ++nFI)
		{
			if (!ticks[nFI]) continue; // Missing cuts remain sparse; never read newly appeared files into the allocation.
			const int pathLength = snprintf(szTemp, sizeof(szTemp), "%s%04d.ani", MeshManager::m_BoneAnimationList[nCount].szAniName, m_stValidAniList[nCount][nFI].nI + 1);
			if (pathLength < 0 || pathLength >= sizeof(szTemp)) return fail();
			handle = _open(szTemp, _O_BINARY, 0);

			if (handle < 0)
				return fail();

			unsigned int buffer = 0, frameCount = 0;
			std::size_t bytes = 0;
			const bool valid = _read(handle, &buffer, 4) == 4 && _read(handle, &frameCount, 4) == 4 &&
				buffer == ticks[nFI] && frameCount == frames[nFI] &&
				asset_load::AnimationBytes(buffer, frameCount, frames[nFI], bytes) &&
				bytes <= allocationBytes && static_cast<std::size_t>(nOffset) * sizeof(D3DXMATRIX) <= allocationBytes - bytes;
			if (!valid) { _close(handle); return fail(); }
			MeshManager::m_BoneAnimationList[nCount].numAniCut[dwFileIndex] = buffer;
			if (_read(handle, &MeshManager::m_BoneAnimationList[nCount].matAnimation[nOffset], static_cast<unsigned int>(bytes)) != static_cast<int>(bytes))
			{ _close(handle); return fail(); }

			if (nCount == 0 || nCount == 1)
			{
				for (size_t j = 0; j < MeshManager::m_BoneAnimationList[nCount].numAniFrame * buffer; ++j)
				{
					D3DXQuaternionRotationMatrix(
						&MeshManager::m_BoneAnimationList[nCount].matQuaternion[j + nOffset],
						&MeshManager::m_BoneAnimationList[nCount].matAnimation[j + nOffset]);
				}				
			}

			nOffset += MeshManager::m_BoneAnimationList[nCount].numAniFrame * buffer;
			_close(handle);

			if (nCount == 0 || nCount == 1)
			{
				int nArrayIndex = m_stValidAniList[nCount][nFI].nI + 1;
				int nWeapon = nArrayIndex / 100 - 1;
				int nAnimation = nArrayIndex % 100 - 1;
				MeshManager::m_sAnimationArray[nCount][nWeapon][nAnimation] = dwFileIndex;

				// TK-BM
				if (nCount == 0)
				{
					if (nAnimation >= 4 && nAnimation < 9)
					{
						for (int nRemainIndex = nAnimation + 1; nRemainIndex < 10; ++nRemainIndex)
							MeshManager::m_sAnimationArray[nCount][nWeapon][nRemainIndex] = MeshManager::m_sAnimationArray[nCount][nWeapon][nAnimation];
					}
					if (nAnimation >= 25 && nAnimation < 29)
					{
						for (int nRemainIndex = nAnimation + 1; nRemainIndex < 30; ++nRemainIndex)
							MeshManager::m_sAnimationArray[nCount][nWeapon][nRemainIndex] = MeshManager::m_sAnimationArray[nCount][nWeapon][nAnimation];
					}
					if (nWeapon >= 12)
					{
						MeshManager::m_sAnimationArray[0][nWeapon][11] = MeshManager::m_sAnimationArray[nCount][11][11];
						MeshManager::m_sAnimationArray[0][nWeapon][12] = MeshManager::m_sAnimationArray[nCount][11][12];
						MeshManager::m_sAnimationArray[0][nWeapon][14] = MeshManager::m_sAnimationArray[nCount][11][14];
					}
					if (nArrayIndex == 138)
					{
						for (int nAniTypeNum = 1; nAniTypeNum < 60; ++nAniTypeNum)
						{
							for (int nAniMotionNum = 0; nAniMotionNum < 56; ++nAniMotionNum)
								MeshManager::m_sAnimationArray[nCount][nAniTypeNum][nAniMotionNum] = MeshManager::m_sAnimationArray[nCount][0][nAniMotionNum];
						}
					}
				}
				else if (nCount == 1)
				{
					if (nAnimation >= 4 && nAnimation < 9)
					{
						for (int nRemainIndex = nAnimation + 1; nRemainIndex < 10; ++nRemainIndex)
							MeshManager::m_sAnimationArray[nCount][nWeapon][nRemainIndex] = MeshManager::m_sAnimationArray[nCount][nWeapon][nAnimation];
					}
					if (nAnimation >= 25 && nAnimation < 29)
					{
						for (int nRemainIndex = nAnimation + 1; nRemainIndex < 30; ++nRemainIndex)
							MeshManager::m_sAnimationArray[nCount][nWeapon][nRemainIndex] = MeshManager::m_sAnimationArray[nCount][nWeapon][nAnimation];
					}
					if (nWeapon == 2 && nAnimation == 4)
					{
						for (int nRemainIndex = 0; nRemainIndex < 4; ++nRemainIndex)
							MeshManager::m_sAnimationArray[nCount][nWeapon][nRemainIndex] = MeshManager::m_sAnimationArray[nCount][1][nRemainIndex];
					}
					if (nArrayIndex == 137)
					{
						for (int nAniTypeNum = 1; nAniTypeNum < MAX_ANI_TYPE; ++nAniTypeNum)
						{
							for (int nAniMotionNum = 0; nAniMotionNum < MAX_ANI_MOTION; ++nAniMotionNum)
								MeshManager::m_sAnimationArray[nCount][nAniTypeNum][nAniMotionNum] = MeshManager::m_sAnimationArray[nCount][0][nAniMotionNum];
						}
					}
				}
			}
			else if(nCount == 2)
			{
				int nArrayIndex = m_stValidAniList[2][nFI].nI + 1;
				int nWeapon = nArrayIndex / 100 - 1;
				int nAnimation = nArrayIndex % 100 - 1;
				MeshManager::m_sAnimationArray[2][nWeapon][nAnimation] = dwFileIndex;

				if (nAnimation == 3)
				{
					for (int i = 4; i < 5; ++i)
						MeshManager::m_sAnimationArray[nCount][nWeapon][i] = MeshManager::m_sAnimationArray[nCount][nWeapon][nAnimation];
				}
				if (nArrayIndex == 307)
				{
					for (int nAniMotionNum = 0; nAniMotionNum < MAX_ANI_MOTION; ++nAniMotionNum)
						MeshManager::m_sAnimationArray[nCount][3][nAniMotionNum] = MeshManager::m_sAnimationArray[nCount][2][nAniMotionNum];
				}
				if (nArrayIndex == 107)
				{
					for (int nAniTypeNum = 1; nAniTypeNum < MAX_ANI_TYPE; ++nAniTypeNum)
					{
						for (int nAniMotionNum = 0; nAniMotionNum < MAX_ANI_MOTION; ++nAniMotionNum)
							MeshManager::m_sAnimationArray[nCount][nAniTypeNum][nAniMotionNum] = MeshManager::m_sAnimationArray[nCount][0][nAniMotionNum];
					}
				}
			}
			else if (nCount == 5)
			{
				int nArrayIndex = m_stValidAniList[5][nFI].nI + 1;
				if (asset_load::AnimationMapping(nCount, m_stValidAniList[nCount][nFI].nI))
					MeshManager::m_sAnimationArray[4][nArrayIndex / 100 + 59][nArrayIndex % 100 - 1] = dwFileIndex;
				if (nArrayIndex == 111)
				{
					for (int nAniTypeNum = 1; nAniTypeNum < MAX_ANI_TYPE; ++nAniTypeNum)
					{
						for (int nAniMotionNum = 0; nAniMotionNum < MAX_ANI_MOTION; ++nAniMotionNum)
							MeshManager::m_sAnimationArray[nCount][nAniTypeNum][nAniMotionNum] = MeshManager::m_sAnimationArray[nCount][0][nAniMotionNum];
					}
				}
			}
			else if (nCount == 3)
			{
				int nArrayIndex = m_stValidAniList[3][nFI].nI + 1;
				if (asset_load::AnimationMapping(nCount, m_stValidAniList[nCount][nFI].nI))
					MeshManager::m_sAnimationArray[2][nArrayIndex / 100 + 59][nArrayIndex % 100 - 1] = dwFileIndex;
				if (nArrayIndex == 108)
				{
					for (int nAniTypeNum = 1; nAniTypeNum < MAX_ANI_TYPE; ++nAniTypeNum)
					{
						for (int nAniMotionNum = 0; nAniMotionNum < MAX_ANI_MOTION; ++nAniMotionNum)
							MeshManager::m_sAnimationArray[nCount][nAniTypeNum][nAniMotionNum] = MeshManager::m_sAnimationArray[nCount][0][nAniMotionNum];
					}
				}
			}
			else if (nCount == 4)
			{
				int nArrayIndex = m_stValidAniList[4][nFI].nI + 1;
				int nWeapon = nArrayIndex / 100 - 1;
				int nAnimation = nArrayIndex % 100 - 1;
				MeshManager::m_sAnimationArray[4][nWeapon][nAnimation] = dwFileIndex;

				if (nAnimation == 3)
				{
					for (int ll = 4; ll < 5; ++ll)
						MeshManager::m_sAnimationArray[nCount][nWeapon][ll] = MeshManager::m_sAnimationArray[nCount][nWeapon][nAnimation];
				}
				if (nArrayIndex == 108)
				{
					for (int nAniTypeNum = 1; nAniTypeNum < MAX_ANI_TYPE; ++nAniTypeNum)
					{
						for (int nAniMotionNum = 0; nAniMotionNum < MAX_ANI_MOTION; ++nAniMotionNum)
							MeshManager::m_sAnimationArray[nCount][nAniTypeNum][nAniMotionNum] = MeshManager::m_sAnimationArray[nCount][0][nAniMotionNum];
					}
				}
			}
			// Resource IDs 7 and 28 are not character classes in the six-class mapping.
			// Their former alias branch indexed outside m_sAnimationArray.

			if (++dwFileIndex >= MeshManager::m_BoneAnimationList[nCount].numAniTypeCount)
			{
				g_nMaxSizeOffset[nCount] = nOffset * sizeof(D3DXMATRIX);
				break;
			}
		}
		g_nMaxSizeOffset[nCount] = static_cast<int>(allocationBytes);
	}

	fclose(fpList);
	return 1;
}

HRESULT MeshManager::RestoreDeviceObjects()
{
	return S_OK;
}

void MeshManager::ReleaseMesh()
{
	for (int nIndex = 0; nIndex < MAX_COMMON_MESH; ++nIndex)
	{
		SAFE_DELETE(m_stCommonMesh[nIndex].pMesh);
	}

	for (int nIndex = 0; nIndex < MAX_SKIN_MESH_LIST; ++nIndex)
	{
		memset(m_stSkinMeshList[nIndex].szFileName, 0, sizeof(m_stSkinMeshList[nIndex].szFileName));
		
		SAFE_DELETE(m_stSkinMeshList[nIndex].pMesh);

		if (m_stSkinMeshList[nIndex].m_pBoneMatrix != nullptr)
		{
			free(m_stSkinMeshList[nIndex].m_pBoneMatrix);
			m_stSkinMeshList[nIndex].m_pBoneMatrix = nullptr;
		}
		if (m_stSkinMeshList[nIndex].m_dwNames != nullptr)
		{
			free(m_stSkinMeshList[nIndex].m_dwNames);
			m_stSkinMeshList[nIndex].m_dwNames = nullptr;
		}
	}
}

void MeshManager::ReleaseNotUsingMesh()
{
	unsigned int dwCurrentTime = timeGetTime();
	if (m_LastLoadTime + 1200 <= dwCurrentTime)
	{
		m_LastLoadTime = dwCurrentTime;

		for (int nIndex = 0; nIndex < MAX_COMMON_MESH; ++nIndex)
		{
			if (m_stCommonMesh[nIndex].pMesh != nullptr)
			{
				if (dwCurrentTime - m_stCommonMesh[nIndex].pMesh->m_dwLastUsedTime > m_stCommonMesh[nIndex].pMesh->m_dwShowTime)
				{
					SAFE_DELETE(m_stCommonMesh[nIndex].pMesh);
				}
			}
		}
	}
}

int MeshManager::ReadObjectMask()
{
	int nCheckSum = 0;

	FILE* fp = nullptr;
	fopen_s(&fp, ObjectMask_Path, "rb");

	if (fp == nullptr)
	{
		MessageBox(0, "Can't read object.bin", "ERROR", MB_OK);
		return 0;
	}

	for (int MaskNum = 0; MaskNum < MAX_OBJECT_MASK; ++MaskNum)
	{
		for (int MaskY = 0; MaskY < 16; ++MaskY)
		{
			fread(MeshManager::m_aObjectMask[MaskNum][MaskY], 16, 1, fp);

			for (int i = 0; i < 16; ++i)
			{
				nCheckSum += MeshManager::m_aObjectMask[MaskNum][MaskY][i] % 4 + 2 * MaskY + 5 * MaskNum;
			}
		}
	}

	char* pTemp = (char*)MeshManager::m_aObjectMask;

	int nLen = strlen((char*)ObjectMaskEncKeys);
	for (int j = 0; j < sizeof(MeshManager::m_aObjectMask); ++j)
		pTemp[j] = pTemp[j] - ObjectMaskEncKeys[j % nLen] - j;

	int nRCheckSum = 0;
	fread(&nRCheckSum, 4u, 1u, fp);
	fclose(fp);

	if (nCheckSum != nRCheckSum)
	{
		MessageBoxA(0, "Object.bin File Crashed", "CheckSum Error", MB_OK);
		return 0;
	}

	return 1;
}

int MeshManager::LoadCommonMesh(int nIndex, int bEffect)
{
	Sleep(0);

	if (m_stCommonMesh[nIndex].pMesh != nullptr)
		return 0;

	m_stCommonMesh[nIndex].pMesh = new TMMesh();

	if (m_stCommonMesh[nIndex].pMesh == nullptr)
		return 0;

	if (bEffect == 1)
		m_stCommonMesh[nIndex].pMesh->m_bEffect = 1;

	if (!m_stCommonMesh[nIndex].pMesh->LoadMsa(m_stCommonMesh[nIndex].szFileName))
		m_stCommonMesh[nIndex].pMesh->m_bEffect = 0;

	return 1;
}

TMMesh* MeshManager::GetCommonMesh(int nIndex, int bEffect, DWORD showTime)
{
	if (nIndex < 0 || nIndex >= MAX_COMMON_MESH)
		return 0;

	if (m_stCommonMesh[nIndex].pMesh == nullptr)
		LoadCommonMesh(nIndex, bEffect);

	if (m_stCommonMesh[nIndex].pMesh == nullptr)
		return 0;

	if (m_stCommonMesh[nIndex].pMesh->m_bEffect != bEffect)
		m_stCommonMesh[nIndex].pMesh->m_bEffect = bEffect;

	m_stCommonMesh[nIndex].pMesh->m_dwLastUsedTime = timeGetTime();
	m_stCommonMesh[nIndex].pMesh->m_dwShowTime = showTime;
	return m_stCommonMesh[nIndex].pMesh;
}

int MeshManager::GetSkinMeshIndex(char* szSkinMeshName)
{
	for (int nIndex = 0; nIndex < MAX_SKIN_MESH_LIST; ++nIndex)
	{
		if (m_stSkinMeshList[nIndex].pMesh != nullptr)
		{
			if (!strcmp(m_stSkinMeshList[nIndex].szFileName, szSkinMeshName))
				return nIndex;
		}
	}

	return -1;
}

int MeshManager::GetAvailableSkinMeshIndex()
{
	for (int nIndex = 0; nIndex < MAX_SKIN_MESH_LIST; ++nIndex)
	{
		if (m_stSkinMeshList[nIndex].pMesh == nullptr)
			return nIndex;
	}

	return -1;
}
