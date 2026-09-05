#pragma once

#include "TMObject.h"

// Representa uma construcao renderizavel do mapa.
//
// Apesar do nome historico, TMHouse tambem apresenta moinhos e construcoes com
// agua animada. A classe consulta camera, terreno, malhas e o objeto focado,
// mas nao possui esses recursos: todos continuam sob responsabilidade dos
// gerenciadores globais e da cena. As particulas criadas durante FrameMove sao
// entregues ao container global de objetos.
//
// Este componente pertence ao mundo visual do client. Estado autoritativo de
// mapa, colisao ou gameplay permanece fora desta classe.
class TMHouse : public TMObject
{
public:
	// Registra a variante visual usada por InitObject, Render e FrameMove.
	TMHouse(char cHouseType);
	~TMHouse();

	// Resolve a malha auxiliar de agua e inicializa o TMObject base.
	int InitObject() override;

	// Atualiza a orientacao e os pontos auxiliares das pas quando aplicavel.
	void InitAngle(float fYaw, float fPitch, float fRoll) override;

	// Desenha a construcao e seus componentes conforme a variante visual.
	int Render() override;

	// Atualiza abertura, rotacao, visibilidade e emissores visuais temporarios.
	int FrameMove(unsigned int dwServerTime) override;

	// Aplica o teste de visibilidade especializado da construcao.
	int IsVisible() override;

public:
	char m_cOpenHouse;              // Indica se a construcao deve aparecer aberta.
	char m_cHouseType;              // Seleciona o comportamento visual da variante.
	short m_sWaterIndex;            // Indice da malha auxiliar de agua.
	float m_fGroundHeight;          // Altura do terreno amostrada para os efeitos.
	float m_fWindMillAngle;         // Angulo corrente das partes rotativas.
	unsigned int m_dwLastParticle; // Instante do ultimo emissor temporario.
	TMVector2 m_vecPos1;            // Deslocamento local da primeira pa/parte.
	TMVector2 m_vecPos2;            // Deslocamento local da segunda pa/parte.

	// Limita globalmente a atualizacao visivel dos efeitos de cascata.
	static unsigned int m_dwVisibleWaterFall;
};
