#pragma once

#include "TMEffect.h"

// Apresenta uma lamina de gelo ou um segmento de sua sequencia visual.
//
// As tres variantes recebidas pelo construtor selecionam malha, textura,
// escala e rotacao. Alguns tipos criam o proximo segmento da sequencia e
// transferem-no ao container de efeitos; a classe nao conserva ponteiros para
// esses filhos. Nenhuma regra de dano ou congelamento e aplicada aqui.
class TMSkillFreezeBlade : public TMEffect
{
public:
    TMSkillFreezeBlade(TMVector3 vecPosition, int nType, int nType2, int nType3);
    ~TMSkillFreezeBlade();

    // Desenha a malha selecionada com escala e orientacao da variante.
    int Render() override;

    // Atualiza o raio de culling a partir da malha compartilhada.
    int IsVisible() override;

    // Anima o segmento e pode enfileirar a continuacao visual da sequencia.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio da animacao no relogio do servidor.
    unsigned int m_dwLifeTime;  // Duracao total do segmento atual.
    unsigned int m_dwLastTime;  // Controle temporal da emissao de particulas.
    TMVector2 m_vecNextD;        // Deslocamento aplicado ao proximo segmento.
    float m_fProgress;           // Progresso normalizado da animacao.
    float m_fAngle;              // Orientacao visual preservada pelo contrato.
    int m_nType;                 // Variante principal e indice da sequencia.
    int m_nType2;                // Variante de malha/textura e temporizacao.
    int m_nType3;                // Variante de rotacao e escala.
    int m_nMeshIndex;            // Malha compartilhada escolhida no construtor.
    int m_bNext;                 // Impede criar o proximo segmento mais de uma vez.
};
