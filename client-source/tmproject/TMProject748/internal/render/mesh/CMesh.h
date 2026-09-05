#pragma once

class TMMesh;
class TMSkinMesh;

// Parte renderizável de uma skin mesh, incluindo material, paleta de ossos e
// marcadores de equipamento. O layout público é preservado por compatibilidade.
class CMesh
{
public:
	// pParentSkin é referência não proprietária e deve permanecer válida durante
	// o uso desta peça.
	CMesh(TMSkinMesh* pParentSkin);
	~CMesh();

	// Renderiza a peça com o modo de brilho indicado e devolve status inteiro.
	virtual int Render(int bBright);

	// Carrega a mesh identificada por file. A string pertence ao chamador e é
	// apenas consultada; LoadMesh não assume ownership do buffer.
	int LoadMesh(char* file);
	// Prepara recursos de efeito após o carregamento da malha.
	int InitEffect();
	// Atualiza o material usando o nível de alpha informado.
	void SetMaterial(char cAlpha);

private:
	int RenderMesh(char cAlpha);

public:
	// m_pMesh e m_pParentSkin referenciam objetos externos. m_pBoneMatrix e
	// m_dwNames são buffers do carregamento cujo ownership deve ser mantido pela
	// implementação de CMesh; consumidores não devem liberá-los diretamente.
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
