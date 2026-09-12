#pragma once

// Porta semantica de selecao de personagem: nao expoe structs nem opcode.
// O implementador codifica a intencao e nao altera UI ou estado da sessao.
class ICharacterLoginSender
{
public:
    virtual ~ICharacterLoginSender() = default;
    // slot deve estar entre 0 e 3. Chamada sincrona, sem ownership transferido;
    // true indica aceite local do envio, nunca confirmacao de login remoto.
    virtual bool SendCharacterLogin(int slot) = 0;
};
