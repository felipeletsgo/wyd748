#include "pch.h"
#include "TMSkillSnow.h"

// Stub deliberadamente inerte. Uma adaptação futura deve provar textura,
// geometria, temporização, entrada no mundo e teardown antes de ativá-lo.
TMSkillSnow::TMSkillSnow(TMVector3 vecTarget)
{
}

TMSkillSnow::~TMSkillSnow()
{
}

int TMSkillSnow::Render()
{
	// Não submete partículas ao renderer no estado atual.
	return 0;
}

int TMSkillSnow::IsVisible()
{
	// O efeito permanece invisível enquanto não houver lifecycle implementado.
	return 0;
}

int TMSkillSnow::FrameMove(unsigned int dwServerTime)
{
	// Não consome tempo nem altera os buffers ainda não inicializados.
	return 0;
}
