#include "pch.h"
#include "TMFlail.h"
#include "TMEffectBillBoard.h"

// Stub preservado sem importar comportamento de uma versão posterior. Os
// retornos mantêm o contrato atual da source até o fluxo visual ser rastreado.
TMFlail::TMFlail()
	: TMObject()
{
}

TMFlail::~TMFlail()
{
}

int TMFlail::FrameMove(unsigned int dwServerTime)
{
	// Sucesso sem mutação: ainda não existe simulação ativa para este objeto.
	return 1;
}

int TMFlail::Render()
{
	// Sucesso sem desenho: evita alegar um efeito 7.48 ainda não comprovado.
	return 1;
}
