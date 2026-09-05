#pragma once

#include "TMObject.h"

// Representa a entidade movel usada para a bicicleta no mundo do jogo.
// O container da cena e responsavel por criar, atualizar e destruir a instancia;
// a classe apenas mantem a posicao inicial necessaria ao movimento por frame.
class TMBike : public TMObject
{
public:
	TMBike();
	~TMBike();

	// Define a posicao inicial da entidade nas coordenadas do mundo.
	void InitPosition(float fX, float fY, float fZ) override;

	// Atualiza a entidade para o instante autoritativo recebido do servidor.
	// Retorna o codigo de continuidade adotado pelo lifecycle de TMObject.
	int FrameMove(unsigned int dwServerTime) override;

public:
	// Origem bidimensional preservada durante a animacao do objeto.
	TMVector2 m_vecStartPosition;
};
