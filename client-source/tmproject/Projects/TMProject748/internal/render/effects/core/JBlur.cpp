#include "pch.h"
#include "JBlur.h"

// Esqueleto do pipeline de motion blur existente no TMProject posterior. No
// client 7.48 atual ele permanece inerte: não aloca surfaces, shaders ou
// texturas e, portanto, não exige teardown adicional.
JBlur::JBlur()
{
}

JBlur::~JBlur()
{
}

int JBlur::InitObject()
{
	// Preserva o sucesso legado sem materializar recursos de renderização.
	return 1;
}

int JBlur::MakeVB()
{
	// Pendente: formato dos vértices e ownership do vertex buffer.
	return 0;
}

int JBlur::Begin()
{
	// Pendente: captura/restauração dos estados do dispositivo Direct3D.
	return 0;
}

void JBlur::End()
{
	// Sem estado iniciado por Begin, não há trabalho de restauração.
}

int JBlur::BeginMotionBlur()
{
	// Pendente: contrato dos render targets e shaders usados pelo efeito.
	return 0;
}

void JBlur::EndMotionBlur()
{
	// Sem pipeline ativo, não há recursos temporários a encerrar.
}

void JBlur::Render()
{
	// Stub deliberado: não emite draw calls.
}

void JBlur::MakeBlur()
{
	// Stub deliberado: não compõe amostras nem altera o backbuffer.
}

void JBlur::SwitchSetTexture()
{
	// Pendente: alternância dos render targets do efeito.
}

void JBlur::SetLoginColor()
{
	// Pendente: constantes de cor específicas da cena de login.
}

void JBlur::SetDefaultColor()
{
	// Pendente: constantes padrão do efeito.
}

void JBlur::CopyTexture(int nScale, IDirect3DTexture9* pSrc, IDirect3DSurface9* pDestSurface, float fDepth)
{
	// pSrc e pDestSurface seriam referências não proprietárias. O stub não os
	// acessa, não retém ponteiros e não modifica a surface de destino.
}

void JBlur::SetVSConstant(float fScale, float fWidth, float fHeight, float fWidthPixelOffset, float fHeightPixelOffset, float fDepth)
{
	// Stub deliberado: não escreve constantes no vertex shader.
}
