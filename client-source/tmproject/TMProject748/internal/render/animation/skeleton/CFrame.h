#pragma once

class TMSkinMesh;
class CMesh;

// Representa um no da hierarquia de frames (ossos) de uma malha animada.
//
// A classe calcula as matrizes combinadas recursivamente e associa cada frame
// aos bones usados por TMSkinMesh. Ela tambem fornece ao renderer matrizes de
// encaixe para armas e efeitos; nao controla regras de animacao ou de combate.
class CFrame
{
public:
	CFrame(DWORD id);
	~CFrame();

	CFrame* FindFrame(DWORD dwID);
	HRESULT InvalidateDeviceObjects();
	int Render();
	void AddFrame(CFrame* pframe);
	void LinkBones(CFrame* root);
	void UpdateFrames(D3DXMATRIX* matCur);

public:
	DWORD m_dwID;
	DWORD m_dwParentID;
	D3DXMATRIX m_matRot;
	D3DXMATRIX m_matCombined;

	// Ligacoes nao proprietarias da arvore. O destrutor deste no nao libera
	// irmaos nem filhos; o dono da hierarquia administra esse lifecycle.
	CFrame* m_pSibling;
	CFrame* m_pFirstChild;

	// Recurso pertencente ao frame. O destrutor e InvalidateDeviceObjects
	// liberam a malha quando ela estiver presente.
	CMesh* m_pMesh;

	// Referencia nao proprietaria ao skin que recebe bones e pontos de encaixe.
	TMSkinMesh* m_pParentSkin;
};
