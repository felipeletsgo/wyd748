#include "pch.h"
#include "TMEffectGold.h"

// Stub deliberadamente inerte. Não copiar temporização, mesh ou trajetória de
// outro TMProject sem confirmar recursos e lifecycle compatíveis com o 7.48.
TMEffectGold::TMEffectGold(TMVector3 vecStart, float vecLiveTime)
{
}

TMEffectGold::~TMEffectGold()
{
}

int TMEffectGold::FrameMove(unsigned int dwServerTime)
{
	// Zero mantém o efeito fora do ciclo ativo da implementação atual.
	return 0;
}

int TMEffectGold::Render()
{
	// Nenhum recurso visual é submetido enquanto o contrato estiver pendente.
	return 0;
}
