#pragma once

#include "TMObject.h"

// Efeito visual de poeira renderizado como objeto temporario da cena.
// A classe possui seus proprios parametros de escala e tempo; nao representa
// item, dano ou qualquer outra regra autoritativa do servidor.
class TMDust : public TMObject
{
public:
	// Cria o efeito com escala visual e variante informadas pelo chamador.
	TMDust(float fScale, int nType);
	~TMDust();

	// Desenha o estado visual atual. Retorna o codigo esperado por TMObject.
	int Render() override;

	// Avanca animacao e lifetime usando o relogio do servidor.
	int FrameMove(unsigned int dwServerTime) override;

public:
	int m_nType;               // Variante visual selecionada na construcao.
	unsigned int m_dwLastTime; // Ultimo instante processado pelo efeito.
	float m_fScaleH;           // Escala horizontal corrente.
	float m_fScaleV;           // Escala vertical corrente.
};
