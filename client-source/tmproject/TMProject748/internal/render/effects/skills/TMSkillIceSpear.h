#pragma once

#include "TMEffect.h"

class TMShade;
class TMObject;

// Controla a apresentacao visual do projetil Ice Spear.
//
// A trajetoria parte de uma posicao fixa e pode acompanhar temporariamente um
// TMObject externo. Esse owner e apenas observado: a classe abandona a
// referencia durante a animacao e nunca o libera. O light map, ao contrario,
// e criado no construtor e destruido junto com o efeito.
//
// Acerto, dano e validacao de alvo nao pertencem a este componente visual.
class TMSkillIceSpear : public TMEffect
{
public:
    TMSkillIceSpear(TMVector3 vecStart, TMVector3 vecTarget, int nLevel, TMObject* pOwner);
    ~TMSkillIceSpear();

    // Desenha a malha do projetil e sua sombra luminosa.
    int Render() override;

    // Atualiza o raio visual usado pelo teste de visibilidade.
    int IsVisible() override;

    // Interpola a trajetoria e encerra o acompanhamento do owner.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio da animacao no relogio do servidor.
    TMVector3 m_vecStartPos;    // Origem imutavel da interpolacao.
    TMVector3 m_vecTargetPos;   // Destino atual, atualizado enquanto ha owner.
    int m_nLevel;               // Variante visual recebida pelo efeito.
    float m_fAngle;             // Orientacao calculada para a malha.
    float m_fProgress;          // Progresso normalizado da trajetoria.
    unsigned int m_dwLifeTime;  // Duracao calculada pela distancia, em ms.
    TMShade* m_pLightMap;       // Recurso visual pertencente a esta instancia.
};
