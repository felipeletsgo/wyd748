#pragma once

#include "TMObject.h"
#include "Structures.h"

class TMEffectBillBoard;

// Representa o vínculo visual entre uma posição de cabeça e uma de cauda.
// A implementação atual é um stub inerte: não avança nem renderiza o efeito.
class TMFlail : public TMObject
{
public:
	TMFlail();
	~TMFlail();

	// Mantém a assinatura do ciclo de objetos. dwServerTime ainda não é usado.
	int FrameMove(unsigned int dwServerTime) override;
	// Não envia geometria ao renderer enquanto o contrato visual estiver pendente.
	int Render() override;

public:
	// Estado e geometria reservados pela definição posterior do objeto.
	short m_sState;
	TMVector3 m_vecHeadPos;
	TMVector3 m_vecTailPos;
	// Ownership não confirmado; o stub não aloca nem libera este efeito.
	TMEffectBillBoard* m_pHeadEffect;
};
