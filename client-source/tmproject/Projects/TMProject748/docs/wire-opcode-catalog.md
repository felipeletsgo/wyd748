# Catálogo inicial de opcodes

Fonte: `internal/core/Basedef.h`, única fonte atual dos identificadores
nomeados. Este catálogo é inventário, não autorização para alterar protocolo.

## Famílias identificadas

| Família | Exemplos |
|---|---|
| bootstrap/login | `MSG_CNFAccountLogin_Opcode`, `MSG_CNFCharacterLogin` |
| movimento | `MSG_Action_Opcode`, `MSG_Action_Stop_Opcode`, `MSG_Motion_Opcode`, `MSG_AirMove_Start_Opcode` |
| inventário | `MSG_DeleteItem_Opcode`, `MSG_SplitItem_Opcode`, `MSG_UseItem_Opcode`, `MSG_UpdateEquip` |
| trade/banco | `MSG_Trade_Opcode`, `MSG_CloseTrade_Opcode`, `MSG_Withdraw_Opcode`, `MSG_Deposit_Opcode` |
| combinação | `MSG_CombineItem_Opcode`, `MSG_CombineItemAylin_Opcode`, `MSG_CombineItemAgatha_Opcode`, `MSG_CombineItemTiny_Opcode` |
| quest/missão | `MSG_Quest_Opcode`, `MSG_Mission` |
| eventos | `MSG_Ping_Opcode`, `MSG_DelayStart_Opcode`, `MSG_SysQuit_Opcode` |

## Dispatch atual

Entrada de rede: `platform/windows/CPSock.cpp`. O pacote é enquadrado como
`MSG_STANDARD` e encaminhado ao `ObjectManager`/cena. O próximo passo da Fase
2 é extrair uma tabela de validação e tradução sem duplicar os structs.

## Regras de migração

- manter o opcode no header legado enquanto houver consumidores;
- validar tamanho antes de reinterpretar o buffer;
- não inferir semântica apenas pelo nome do opcode;
- registrar caller, consumidor e fallback para cada entrada migrada;
- adicionar teste de pacote truncado antes de mover o handler.
