#pragma once

#include "TMEffect.h"

class TMHuman;

// Efeito visual de abertura usado por diferentes animacoes de habilidade.
//
// A variante escolhe textura, escala e comportamento visual. Quando pOwner nao
// e nulo, a posicao acompanha esse personagem por uma referencia nao
// proprietaria; o personagem precisa permanecer valido durante o efeito.
class TMEffectStart : public TMEffect
{
public:
    // Copia vecPosition e aceita pOwner nulavel. As variantes temporarias
    // encerram sozinhas; a variante 4 e ciclica e depende de remocao externa.
    TMEffectStart(TMVector3 vecPosition, int nType, TMHuman* pOwner);
    ~TMEffectStart();

    // Desenha a malha com a textura e a transformacao da variante selecionada.
    int Render() override;

    // Atualiza o raio da malha antes do teste de visibilidade herdado.
    int IsVisible() override;

    // Avanca progresso, escala, rotacao e acompanhamento do owner.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Instante inicial na base do servidor.
    unsigned int m_dwLifeTime;  // Duracao; zero identifica o modo ciclico.
    float m_fAngle;             // Rotacao corrente da animacao.
    float m_fProgress;          // Progresso normalizado ou fase do ciclo.
    int m_nType;                // Variante visual solicitada.
};
