#pragma once

#include "TMEffect.h"

class TMEffectBillBoard2;

// Compoe as variantes visuais historicamente agrupadas como Judgement.
//
// O tipo recebido escolhe texturas, cores, escalas, rotacao e efeitos de mesh.
// A classe nao calcula acerto, dano ou qualquer outro resultado autoritativo da
// habilidade. Os efeitos filhos entram no container da cena e sao encerrados
// por meio do ObjectManager.
class TMSkillJudgement : public TMEffect
{
public:
    // Copia a posicao, escolhe a variante nType e aplica fSetScale aos tipos
    // que aceitam escala configuravel.
    TMSkillJudgement(TMVector3 vecPosition, int nType, float fSetScale);
    ~TMSkillJudgement();

    // Atualiza a vida visual usando o relogio do servidor.
    // Retorna 0 quando invisivel e 1 quando o frame foi processado.
    int FrameMove(unsigned int dwServerTime) override;

public:
    int m_nType;                    // Variante visual selecionada.
    TMEffectBillBoard2* m_pEffect1; // Referencia nulavel ao primeiro billboard.
    TMEffectBillBoard2* m_pEffect2; // Referencia nulavel ao billboard acompanhado.
    unsigned int m_dwStartTime;     // Instante inicial na base do servidor.
    unsigned int m_dwLifeTime;      // Vida do controlador em milissegundos.
};
