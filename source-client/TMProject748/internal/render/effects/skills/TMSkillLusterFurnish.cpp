#include "pch.h"
#include "TMSkillLusterFurnish.h"
#include "TMGlobal.h"

// Efeito visual linear entre dois pontos. Os vetores são copiados e não há
// ownership externo; o ObjectManager controla a vida da instância na cena.
TMSKillLusterFurnish::TMSKillLusterFurnish(TMVector3 vecStart, TMVector3 vecEnd)
	: TMEffect()
{
	m_vecStartPos = vecStart;
	m_vecEndPos = vecEnd;

	m_dwStartTime = g_pTimerManager->GetServerTime();
	m_dwLifeTime = 2000;
}

TMSKillLusterFurnish::~TMSKillLusterFurnish()
{
}

int TMSKillLusterFurnish::FrameMove(unsigned int dwServerTime)
{
	// dwServerTime integra a assinatura virtual, mas a implementação preservada
	// consulta o TimerManager global para usar a mesma base da criação.
	if (!IsVisible())
		return 0;

	float fProgress = (static_cast<float>(g_pTimerManager->GetServerTime()) - static_cast<float>(m_dwStartTime)) / static_cast<float>(m_dwLifeTime);

	// Interpolação linear enquanto a vida útil não terminou. Após 2 segundos o
	// objeto solicita sua remoção; nenhum ponteiro deve ser usado depois disso.
	if (fProgress <= 1.0f)
		m_vecPosition = (m_vecStartPos * (1.0f - fProgress)) + (m_vecEndPos * fProgress);
	else
		g_pObjectManager->DeleteObject(this);

	return 1;
}

int TMSKillLusterFurnish::Render()
{
	// Não há geometria própria: efeitos/objetos associados representam o visual.
	return 1;
}
