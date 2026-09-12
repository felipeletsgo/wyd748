#pragma once

#include "TMObject.h"
#include "Structures.h"

class TMEffectBillBoard2;

// Representa objetos visuais que flutuam ou acompanham a superficie da agua.
//
// Conforme o tipo do mapa, a instancia usa uma skin mesh ou billboards e
// atualiza sua altura pela cena. O contrato legado registra os billboards no
// container global, mas conserva e libera seus ponteiros no destrutor; essa
// relacao de lifecycle foi preservada e deve ser considerada antes de qualquer
// alteracao de ownership.
class TMFloat : public TMObject
{
public:
    TMFloat(int nType);
    ~TMFloat();

    // Materializa a skin mesh ou os billboards correspondentes ao tipo.
    int InitObject() override;

    // Desenha a skin mesh quando a variante a utiliza.
    int Render() override;

    // Sincroniza altura, luz, animacao e efeitos com a superficie da agua.
    int FrameMove(unsigned int dwServerTime) override;

    // Recria recursos Direct3D da skin mesh, quando aplicavel.
    void RestoreDeviceObjects() override;

    // Invalida recursos Direct3D da skin mesh antes da perda do dispositivo.
    void InvalidateDeviceObjects() override;

    // Atualiza a posicao base e a posicao dos billboards associados.
    void InitPosition(float fX, float fY, float fZ) override;

public:
    LOOK_INFO m_stLookInfo;                   // Aparencia da variante com skin mesh.
    SANC_INFO m_stSancInfo;                   // Refinamentos visuais da skin mesh.
    TMEffectBillBoard2* m_pBillBoard;         // Billboard principal retido pela instancia.
    TMEffectBillBoard2* m_pWaterEffect[1];    // Efeito de agua retido pela instancia.
    unsigned int m_dwWaterTime;              // Controle temporal da animacao da agua.
};
