#pragma once

#include "TMEffect.h"

// Efeito visual que percorre linearmente o segmento entre dois pontos.
//
// A classe nao aplica a habilidade nem altera atributos do personagem. Ela
// apenas apresenta a trajetoria durante sua vida util e solicita ao gerenciador
// de objetos que encerre seu lifecycle ao final. Os vetores sao copiados; nao
// existe ownership externo associado aos parametros.
//
// O nome historico TMSKill (com K maiusculo) foi preservado para nao alterar o
// contrato de compilacao das referencias existentes.
class TMSKillLusterFurnish : public TMEffect
{
public:
    // Define as extremidades copiadas da trajetoria visual.
    TMSKillLusterFurnish(TMVector3 vecStart, TMVector3 vecEnd);
    ~TMSKillLusterFurnish();

    // Atualiza a interpolacao e encerra o efeito depois de m_dwLifeTime.
    // Retorna 0 quando invisivel e 1 quando o frame foi processado.
    int FrameMove(unsigned int dwServerTime) override;

    // Mantem o contrato virtual; a classe nao possui geometria propria.
    int Render() override;

public:
    unsigned int m_dwStartTime; // Instante inicial na base de tempo do servidor.
    unsigned int m_dwLifeTime;  // Duracao total do efeito em milissegundos.
    TMVector3 m_vecStartPos;     // Inicio copiado do segmento.
    TMVector3 m_vecEndPos;       // Destino copiado do segmento.
};
