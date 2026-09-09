#pragma once

#include "TMItem.h"

enum class EGATE_STATE
{
    // Estados estáveis e transições temporizadas da animação do portão.
    EGATE_OPEN = 1,
    EGATE_CLOSED = 2,
    EGATE_LOCKED = 3,
    EGATE_OPENING = 4,
    EGATE_CLOSING = 5,
    EGATE_LOCKING = 6,
};

class TMEffectBillBoard2;

// Item de mundo especializado em portões compostos por partes esquerda e
// direita. A autorização de abertura continua dependente do estado do servidor.
class TMGate : public TMItem
{
public:
    TMGate();
    ~TMGate();

    // Copia o item-base e deriva índices/dimensões necessários à montagem.
    virtual void InitGate(STRUCT_ITEM stItem);
    int Render() override;
    int FrameMove(unsigned int dwServerTime) override;
    // Inicia ou conclui a transição visual para o estado informado.
    virtual void SetState(EGATE_STATE eState);
    int IsMouseOver() override;

public:
    // O efeito especial acompanha a instância e pode ser nulo. Os índices de
    // mesh não transferem ownership dos recursos carregados globalmente.
    TMEffectBillBoard2* m_pEffectSpecial;
    float m_fOpenAngle;
    EGATE_STATE m_eState;
    unsigned int m_dwLastSetTime;
    short m_sItemIndex;
    int m_nLeftIndex;
    int m_nRightIndex;
    float m_fWidth;
    short m_sAuth;
    short m_sType;
    float m_StdMinus;
    float m_StdPlus;
    float m_RotMinus;
    float m_RotPlus;
};
