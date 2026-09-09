#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Emissor generico de particulas visuais do client.
//
// O construtor materializa billboards conforme tipo, quantidade, textura e
// direcao e os transfere para o container de efeitos da cena. A instancia do
// emissor conserva apenas o tempo de vida e os parametros necessarios para seu
// proprio lifecycle; ela nao possui os billboards depois de adiciona-los.
class TMEffectParticle : public TMEffect
{
public:
    TMEffectParticle(TMVector3 vecPosition, int nType, int nCount, float fSize, unsigned int dwColor,
        int bRand, int nTextureIndex, float fRadius, int bBright, TMVector3 vecDir, unsigned int dwLifeTime);
    ~TMEffectParticle();

    // O emissor nao desenha geometria propria; os filhos cuidam do desenho.
    int Render() override;

    // Define o volume usado pelo culling do emissor.
    int IsVisible() override;

    // Encerra a instancia quando seu tempo de vida expira.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do lifecycle no relogio do servidor.
    unsigned int m_dwLifeTime;  // Duracao total configurada, em milissegundos.
    float m_fAngle;             // Orientacao preservada para variantes legadas.
    int m_nType;                // Seleciona o padrao de emissao das particulas.
};
