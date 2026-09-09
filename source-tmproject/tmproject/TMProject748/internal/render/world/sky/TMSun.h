#pragma once

#include "TreeNode.h"

// Parametros de uma imagem do lens flare solar.
// Cada entrada referencia uma textura gerenciada externamente; nao possui
// recurso nem memoria dinamica propria.
struct stFlare
{
    int nTexIndex;       // Indice da textura de efeito.
    float fLoc;          // Posicao relativa no eixo entre flare e tela.
    float fScale;        // Escala calculada para a resolucao corrente.
    unsigned int diffuse; // Cor difusa enviada aos vertices.
};

// Representacao visual do sol e de sua cadeia de lens flares.
//
// O objeto pertence ao lifecycle da cena. Ele consulta camera, dispositivo e
// texturas globais durante Render, sem possuir esses recursos.
class TMSun : public TreeNode
{
public:
    TMSun();
    ~TMSun();

    // Prepara os 12 flares e os quatro vertices reutilizados na renderizacao.
    virtual int InitObject();

    // Projeta a direcao do sol e desenha a cadeia de flares na tela.
    int Render() override;

    // Preserva o ponto virtual de atualizacao; atualmente nao altera estado.
    virtual int FrameMove();

public:
    stFlare m_stFlareArray[12];   // Configuracao fixa da cadeia de flares.
    D3DXVECTOR3 m_vFlareDirection; // Direcao do sol relativa a camera.
    RDTLVERTEX m_vecTLVertex[4];  // Quad temporario reutilizado por flare.
    int bInViewPort;              // Estado historico de visibilidade em tela.
    int m_bHide;                  // Diferente de zero oculta a renderizacao.
    float m_fDefSize;             // Multiplicador global do tamanho dos flares.
};
