#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Controla a precipitacao visual de neve ao redor da camera ou personagem.
// A instancia possui 200 flocos e seus buffers; nenhum deles e compartilhado
// ou transferido para outro componente.
class TMSnow : public TMEffect
{
public:
    // fScale determina o tamanho visual dos flocos.
    TMSnow(float fScale);
    ~TMSnow();

    // Desenha os flocos visiveis e administra o audio ambiente associado.
    int Render() override;

    // Atualiza e recicla flocos com base no tempo e na camera corrente.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwOldServerTime;    // Ultimo instante processado.
    float m_fSpeed[200];               // Velocidade individual dos flocos.
    TMVector3 m_vecSnowPosition[200];  // Posicao corrente dos flocos.
    RDLVERTEX m_vertex[4];             // Quad reutilizado no desenho.
    float m_fScale;                    // Escala efetiva do efeito.
};
