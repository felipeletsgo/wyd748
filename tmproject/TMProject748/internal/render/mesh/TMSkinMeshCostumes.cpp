#include "pch.h"
// TMSkinMesh costume and mantle selection; mesh lifecycle and rendering stay in TMSkinMesh.cpp.
#include "CFrame.h"
#include "TMObject.h"
#include "TMEffectSWSwing.h"
#include "TMSkinMesh.h"
#include "FallbackCostumeTable.h"
#include "CostumeSelection.h"
#include "TMGlobal.h"
#include "TMEffectBillBoard.h"
#include "TMHuman.h"
#include "CMesh.h"
#include "TMLog.h"

void TMSkinMesh::SetHardcoreMantle(char* szTexture, char* szName)
{
	sprintf(szName, "mesh\\newmt.msh");
	if (m_Look.Skin0 == 0)
	{
		sprintf(szTexture, "mesh\\newmtB000.wys");
	}
	else if (m_Look.Skin0 == 1)
	{
		sprintf(szTexture, "mesh\\newmtR000.wys");
	}
	else
	{
		sprintf(szTexture, "mesh\\newmtW000.wys");
	}
}

void TMSkinMesh::SetOldCostume(int costype, char* szTexture, char* szName)
{
	switch (costype + 1)
	{
	case 0:
		SetRenewOldCostume(costype, szTexture, szName);
		break;
	case 1:
		return;
	case 2:
		switch (m_Cos)
		{
		case 1:
			strcpy(szTexture, "mesh\\ch020161.wyt");
			strcpy(szName, "mesh\\ch020161.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szTexture, "mesh\\ch020261.wyt");
			strcpy(szName, "mesh\\ch020261.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szTexture, "mesh\\ch020357.wyt");
			strcpy(szName, "mesh\\ch020357.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szTexture, "mesh\\ch020457.wyt");
			strcpy(szName, "mesh\\ch020457.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szTexture, "mesh\\ch020557.wyt");
			strcpy(szName, "mesh\\ch020557.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szTexture, "mesh\\ch020657.wyt");
			strcpy(szName, "mesh\\ch020657.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 3:
		if (!strncmp(szTexture, "mesh\\ch0101", 11) || !strncmp(szTexture, "mesh\\ch0201", 11))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '0';
		}
		else if (!strncmp(szTexture, "mesh\\ch01", 9) || !strncmp(szTexture, "mesh\\ch02", 9))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '1';
		}
		if (!strncmp(szName, "mesh\\ch0101", 11) || !strncmp(szName, "mesh\\ch0201", 11))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '0';
		}
		else if (!strncmp(szName, "mesh\\ch01", 9) || !strncmp(szName, "mesh\\ch02", 9))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '1';
		}
		break;
	case 4:
		strcpy(szTexture, "mesh\\SpiderCos.wyt");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch020190.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch020290.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch020390.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch020490.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch020590.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch020690.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 5:
		if (!strncmp(szTexture, "mesh\\ch0101", 11) || !strncmp(szTexture, "mesh\\ch0201", 11))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '7';
		}
		else if (!strncmp(szTexture, "mesh\\ch0102", 11) || !strncmp(szTexture, "mesh\\ch0201", 11))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 7] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '7';
		}
		else if (!strncmp(szTexture, "mesh\\ch01", 9) || !strncmp(szTexture, "mesh\\ch02", 9))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '7';
		}

		if (!strncmp(szName, "mesh\\ch0101", 11) || !strncmp(szName, "mesh\\ch0201", 11))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '7';
		}
		else if (!strncmp(szName, "mesh\\ch01", 9) || !strncmp(szName, "mesh\\ch02", 9u))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '7';
		}
		break;
	case 6:
		switch (m_Cos)
		{
		case 1:
			strcpy(szTexture, "mesh\\ch020117.wyt");
			strcpy(szName, "mesh\\ch020117.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szTexture, "mesh\\ch020117.wyt");
			strcpy(szName, "mesh\\ch020217.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szTexture, "mesh\\ch020317.wyt");
			strcpy(szName, "mesh\\ch020317.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szTexture, "mesh\\ch020417.wyt");
			strcpy(szName, "mesh\\ch020417.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szTexture, "mesh\\ch020517.wyt");
			strcpy(szName, "mesh\\ch020517.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szTexture, "mesh\\ch020617.wyt");
			strcpy(szName, "mesh\\ch020617.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 7:
		strcpy(szTexture, "mesh\\ch010195.wyt");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch010195.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch010295.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch010395.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch010495.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch010595.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch010695.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 8:
		switch (m_Cos)
		{
		case 1:
			strcpy(szTexture, "mesh\\ch020197.wyt");
			strcpy(szName, "mesh\\ch020197.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szTexture, "mesh\\ch020297.wyt");
			strcpy(szName, "mesh\\ch020297.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szTexture, "mesh\\ch020397.wyt");
			strcpy(szName, "mesh\\ch020397.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szTexture, "mesh\\ch020497.wyt");
			strcpy(szName, "mesh\\ch020497.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szTexture, "mesh\\ch020597.wyt");
			strcpy(szName, "mesh\\ch020597.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szTexture, "mesh\\ch020697.wyt");
			strcpy(szName, "mesh\\ch020697.msh");
			m_Cos = 1;
			break;
		}
		break;
	}
}

void TMSkinMesh::SetRenewOldCostume(int costype, char* szTexture, char* szName)
{
	strcpy(szTexture, "mesh\\ch0101115.wys");
	switch (m_Cos)
	{
	case 1:
		strcpy(szName, "mesh\\ch0101115.msh");
		m_Cos = 2;
		break;
	case 2:
		strcpy(szName, "mesh\\ch0102115.msh");
		m_Cos = 3;
		break;
	case 3:
		strcpy(szName, "mesh\\ch0103115.msh");
		m_Cos = 4;
		break;
	case 4:
		strcpy(szName, "mesh\\ch0104115.msh");
		m_Cos = 5;
		break;
	case 5:
		strcpy(szName, "mesh\\ch0105115.msh");
		m_Cos = 6;
		break;
	case 6:
		strcpy(szName, "mesh\\ch0106115.msh");
		m_Cos = 1;
		break;
	}
}

void TMSkinMesh::SetCostume(int Costype, char* szTexture, char* szName)
{
	// Types without a native renderer are data in FallbackCostumeTable.h;
	// any other type keeps the older selection.
	if (const auto* costume = fallback_costume::Find(Costype))
		fallback_costume::Apply(*costume, m_Cos, szTexture, szName);
	else
		SetOldCostume(Costype, szTexture, szName);
}

namespace
{
// Mantle textures matched by the original MantleException strcmp chain.
constexpr const char* kMantleExceptionTextures[] = {
	"mesh\\mt0101170.wyt",
	"mesh\\mt0101171.wyt",
	"mesh\\mt0101172.wyt",
	"mesh\\mt0101173.wyt",
	"mesh\\mt0101174.wyt",
	"mesh\\mt0101175.wyt",
	"mesh\\mt0101176.wyt",
	"mesh\\mt0101177.wyt",
	"mesh\\mt0101178.wyt",
	"mesh\\mt0101179.wyt",
	"mesh\\mt0101180.wyt",
	"mesh\\mt0101181.wyt",
	"mesh\\mt0101182.wyt",
	"mesh\\mt0101183.wyt",
	"mesh\\mt0101184.wyt",
	"mesh\\mt0101185.wyt",
	"mesh\\mt0101186.wyt",
	"mesh\\mt0101187.wyt",
	"mesh\\mt0101188.wyt",
	"mesh\\mt0101189.wyt",
	"mesh\\mt0101190.wyt",
	"mesh\\mt0101191.wyt",
	"mesh\\mt0101192.wyt",
	"mesh\\mt0101193.wyt",
	"mesh\\mt0101195.wyt",
	"mesh\\mt0101196.wyt",
	"mesh\\mt0101197.wyt",
	"mesh\\mt0101198.wyt",
	"mesh\\mt0101199.wyt",
	"mesh\\mt0101200.wyt",
};
}

int TMSkinMesh::MantleException(char* texture)
{
	for (const char* name : kMantleExceptionTextures)
		if (!strcmp(texture, name))
			return 1;
	return 0;
}

BOOL TMSkinMesh::God2Exception(int i)
{
	return MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'g'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'o'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'd'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[8] == '2'
		&& i == 1
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'd'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[8] == '1'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'd'
		&& i == 1
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'e'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'o'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'm'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'h'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'y'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 's'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'p'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'c'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'w'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'b'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'w'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'f'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'e'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'c'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'b'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'm'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'i'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'm'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'o'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 't'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'w'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 't'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'h'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 's'
		&& i == 1
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'e'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 't'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'n'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'r'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'c'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'f'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'n'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'l'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 't'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'g';
}
