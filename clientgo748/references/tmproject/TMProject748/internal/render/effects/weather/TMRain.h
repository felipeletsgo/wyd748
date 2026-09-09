#pragma once

#include "TMEffect.h"

// Controla o efeito visual de chuva ativo na cena.
// A instancia possui um conjunto fixo de 50 gotas e nao transfere ownership
// desses buffers: posicoes, velocidades e vertices vivem dentro do objeto.
class TMRain : public TMEffect
{
public:
	TMRain();
	~TMRain();

	// Envia as gotas correntes ao renderer e retorna o codigo de TMEffect.
	int Render() override;

	// Atualiza posicoes e velocidades conforme o tempo do servidor.
	int FrameMove(unsigned int dwServerTime) override;

public:
	unsigned int m_dwOldServerTime;   // Instante usado para calcular o delta.
	float m_fSpeed[50];               // Velocidade individual de cada gota.
	TMVector3 m_vecRainPosition[50];  // Posicao corrente de cada gota.
	RDLVERTEX m_vertex[4];            // Quad reutilizado durante o desenho.
};
