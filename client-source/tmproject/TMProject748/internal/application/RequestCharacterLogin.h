#pragma once

#include "ports/ICharacterLoginSender.h"

// Caso de uso sincrono: valida a intencao de entrada e solicita um unico envio.
// A cena continua dona do debounce, existencia do personagem, botoes e timer.
// Nao troca cena nem modifica o estado autoritativo da sessao/personagem.
inline bool RequestCharacterLogin(ICharacterLoginSender& sender, int slot)
{
    // Defesa reutilizavel; a cena viva ja rejeita estes indices antes da chamada.
    if (slot < 0 || slot >= 4)
        return false;

    // Propaga falha sem retry. Codificacao e lifetime do buffer pertencem ao
    // adaptador wire; o caso de uso nao importa detalhes de protocolo/plataforma.
    return sender.SendCharacterLogin(slot);
}
