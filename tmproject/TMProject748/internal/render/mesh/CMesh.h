#pragma once

class TMMesh;
class TMSkinMesh;

// Renderable skin part with material, bone palette and equipment markers.
// Preserve the public layout for compatibility.
class CMesh
{
public:
	// pParentSkin is borrowed and must outlive this part.
	CMesh(TMSkinMesh* pParentSkin);
	~CMesh();

	// Render with the requested brightness mode and return an integer status.
	virtual int Render(int bBright);

	// Load the named mesh without modifying or owning the caller's path buffer.
	int LoadMesh(char* file);
	// Prepare effect resources after loading the mesh.
	int InitEffect();
	// Update the material with the requested alpha.
	void SetMaterial(char cAlpha);

private:
	int RenderMesh(char cAlpha);

public:
	// All resource pointers are borrowed. MeshManager owns m_pMesh, m_pBoneMatrix
	// and m_dwNames; CFrame owns the matrices referenced by m_pBoneOffset.
	// Consumers must detach before the corresponding owner releases resources.
	TMMesh* m_pMesh;
	DWORD m_dwID;
	DWORD m_numPalette;
	D3DXMATRIX* m_pBoneMatrix;
	D3DXMATRIX* m_pBoneOffset[40];
	DWORD* m_dwNames;
	DWORD m_numFaceInflunce;
	int m_bMeshGenerated;
	TMSkinMesh* m_pParentSkin;
	short m_sMultiType;
	short m_sLegendType;
	int m_nTextureIndex;
	DWORD m_numFaces;
	int m_nSkinMeshIndex;
	bool m_bHead;
	bool m_bMount;
	bool m_god2cos;
	int m_god2Alpha;
	int m_god2Red;
	int m_god2Green;
	int m_god2Brue;
	int m_bSheild;
};
