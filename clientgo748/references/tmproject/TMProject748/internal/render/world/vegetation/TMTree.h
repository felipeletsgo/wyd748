#pragma once

#include "TMObject.h"
#include "Structures.h"

// Representa uma arvore ou objeto de vegetacao animada do mapa.
//
// A classe converte o tipo do objeto em aparencia e skin mesh, atualiza luz e
// animacao conforme a cena e pode emitir particulas sazonais. A skin mesh usa o
// campo herdado de TMObject e segue o lifecycle dessa entidade; as particulas
// criadas sao transferidas ao container de efeitos da cena.
class TMTree : public TMObject
{
public:
    // Inicializa a aparencia correspondente ao tipo de objeto do mapa.
    TMTree(int nType);
    ~TMTree();

    // Materializa a skin mesh quando ela ainda nao existe.
    int InitObject() override;

    // Desenha a vegetacao e ajusta variantes dependentes do estado da cena.
    int Render() override;

    // Atualiza culling, iluminacao, animacao e particulas temporarias.
    int FrameMove(unsigned int dwServerTime) override;

    // Recria a skin mesh e seus recursos dependentes do dispositivo.
    void RestoreDeviceObjects() override;

    // Invalida os recursos Direct3D mantidos pela skin mesh.
    void InvalidateDeviceObjects() override;

    // Encaminha a posicao para o lifecycle comum de TMObject.
    void InitPosition(float fX, float fY, float fZ) override;

    // Seleciona uma animacao da skin mesh, quando materializada.
    virtual void SetAnimation(int nAniIndex);

    // Converte o tipo do mapa em look, sanc e indice de skin mesh.
    void InitLook(int nType);

public:
    LOOK_INFO m_stLookInfo;      // Aparencia usada para construir a skin mesh.
    SANC_INFO m_stSancInfo;      // Refinamentos visuais associados ao look.
    unsigned int m_dwLastTime;  // Controle temporal da ultima particula emitida.
};
