#include "pch.h"
#include "TMFlail.h"
#include "TMEffectBillBoard.h"

// Stub preservado sem importar comportamento de uma versao posterior. Os
// retornos mantem o contrato atual da source ate o fluxo visual ser rastreado.
TMFlail::TMFlail()
	: TMObject()
{
}

TMFlail::~TMFlail()
{
}

int TMFlail::FrameMove(unsigned int dwServerTime)
{
	// Sucesso sem mutacao: ainda nao existe simulacao ativa para este objeto.
	return 1;
}

int TMFlail::Render()
{
	// Sucesso sem desenho: evita alegar um efeito 7.48 ainda nao comprovado.
	return 1;
}
