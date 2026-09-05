#pragma once

#include "TMObject.h"
#include "Structures.h"

// Representacao visual de uma folha ou elemento leve de vegetacao do mundo.
//
// O TMObjectContainer cria esta classe a partir do tipo do objeto de mapa. A
// instancia mantem os dados de aparencia por valor e delega ao TMObject o
// ownership da skin mesh. Nao ha regra autoritativa de jogo neste componente.
class TMLeaf : public TMObject
{
public:
    // Registra o tipo do objeto; a malha e materializada por InitObject.
    explicit TMLeaf(int nType);
    ~TMLeaf();

    // Cria e restaura a skin mesh quando ela ainda nao existe.
    int InitObject() override;

    // Desenha a vegetacao conforme distancia, hardware e chaves visuais.
    int Render() override;

    // Atualiza visibilidade, altura, iluminacao e animacao da skin mesh.
    int FrameMove(unsigned int dwServerTime) override;

    // Recria ou restaura recursos dependentes do dispositivo Direct3D.
    void RestoreDeviceObjects() override;

    // Invalida os recursos da skin mesh antes da perda do dispositivo.
    void InvalidateDeviceObjects() override;

    // Encaminha a posicao recebida ao lifecycle comum de TMObject.
    void InitPosition(float fX, float fY, float fZ) override;

public:
    LOOK_INFO m_stLookInfo; // Aparencia usada para construir a skin mesh.
    SANC_INFO m_stSancInfo; // Refinamentos visuais usados pela skin mesh.
};
