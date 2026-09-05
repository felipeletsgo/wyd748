#pragma once

#include "TMEffect.h"

// Precipitacao localizada composta por dez gotas ao redor de uma coordenada.
// Os vetores e vertices pertencem a propria instancia; respingos secundarios
// sao entregues ao container de efeitos pela implementacao.
class TMDrop : public TMEffect
{
public:
    // Define o centro bidimensional da area de precipitacao.
    TMDrop(TMVector2 vec);
    ~TMDrop();

    // Desenha as gotas correntes e retorna o codigo historico do efeito.
    int Render();

    // Move as gotas e recria as que atingiram o terreno ou a agua.
    int FrameMove(unsigned int dwServerTime);

public:
    TMVector2 m_vecPos;            // Centro da area do efeito.
    unsigned int m_dwOldServerTime;// Ultimo instante processado.
    float m_fSpeed[10];            // Velocidade individual das gotas.
    TMVector3 m_vecDropPosition[10];// Posicao corrente das gotas.
    RDLVERTEX m_vertex[4];         // Quad reutilizado no desenho.
};
