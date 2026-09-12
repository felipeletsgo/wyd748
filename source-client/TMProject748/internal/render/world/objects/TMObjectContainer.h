#pragma once

#include "TreeNode.h"

class TMGround;
class TMLight;

// Limites fixos preservados do contrato legado do container regional.
constexpr auto MAX_OBJECT_LIST = 4096;
constexpr auto MAX_LIGHT_CONTAINER = 6;

/**
 * Agrupa os objetos visuais carregados para uma regiao do mundo.
 *
 * A arvore de cena continua responsavel pelos filhos registrados neste no.
 * `m_pGround` e as entradas de `m_pObjectList` sao referencias nao
 * proprietarias usadas para acesso rapido. O container de efeitos do terreno
 * e anexado como filho, enquanto as luzes mantem o lifecycle legado tratado
 * pela implementacao da classe.
 */
class TMObjectContainer : public TreeNode
{
public:
    TMGround* m_pGround; // Terreno externo associado a esta regiao.
    int m_nLightIndex;
    TMLight* m_pLightContainer[MAX_LIGHT_CONTAINER];
    TreeNode* m_pGroundEffectContainer; // Filho pertencente a arvore de cena.
    int m_nObjectIndex;
    TMObject* m_pObjectList[MAX_OBJECT_LIST]; // Indice nao proprietario dos filhos.

public:
    /** Cria o agregado para `pGround`, que pode ser consultado sem transferir ownership. */
    TMObjectContainer(TMGround* pGround);
    ~TMObjectContainer();

    /** Carrega do arquivo os objetos da regiao; retorna o status legado da operacao. */
    int Load(const char* szFileName);

    /** Persiste os objetos da regiao; retorna o status legado da operacao. */
    int Save(const char* szFileName);

    /** Atualiza os filhos com o tempo autoritativo recebido da cena. */
    int FrameMove(unsigned int dwServerTime);
};
