#include "pch.h"
#include "TMVideoWnd.h"

// Adaptador DirectShow ainda inativo no client 7.48. As assinaturas e os
// retornos neutros permanecem disponíveis para NewApp, sem fingir que existe
// reprodução, janela de vídeo ou lifecycle COM implementado.
TMVideoWnd::TMVideoWnd(int bFull)
{
}

TMVideoWnd::~TMVideoWnd()
{
}

HRESULT TMVideoWnd::PlayMovieInWindow(char* szFile)
{
	return E_NOTIMPL;
}

HRESULT TMVideoWnd::InitVideoWindow()
{
	return E_NOTIMPL;
}

void TMVideoWnd::MoveVideoWindow()
{
}

void TMVideoWnd::CheckVisibility()
{
}

int TMVideoWnd::OpenClip(const char* szFilename)
{
	return 0;
}

void TMVideoWnd::CloseClip()
{
}

void TMVideoWnd::CloseInterfaces()
{
}

HRESULT TMVideoWnd::ToggleFullScreen()
{
	return E_NOTIMPL;
}

int TMVideoWnd::HandleGraphEvent()
{
	return 0;
}
