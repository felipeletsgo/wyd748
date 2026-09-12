---
name: wydgo748-feature
description: Implementar ou revisar comportamento do servidor WYD-Go 7.48 e contratos integrados com o client TMProject; não acionar para documentação ou organização sem mudança funcional.
---

# Feature do WYD-Go 7.48

Use esta skill para mudanças no servidor Go ou em uma feature que precise
conversar entre `wydgo748/` e `tmproject/`.

## Modelo de autoridade

O client envia intenções e apresenta estado. O servidor valida identidade,
permissões, regras, recursos, cooldowns, colisões e transições; persiste o
estado autoritativo e envia snapshots/deltas. Nunca aceitar do client uma
posição, item, dano, skill ou saldo como verdade sem validação server-side.

Antes de editar, classifique o delta como `PARIDADE_NATIVA`,
`MODERNIZACAO_COMPATIVEL` ou `EXTENSAO_COORDENADA`. Se tocar comportamento,
packet, ABI, UI, input, render, asset ou lifecycle legado, use
`wyd-client748-research` antes desta skill. Não portar o TMProject 7.69 às
cegas para o 7.48.

## Procedimento

1. Reutilize a entrada única do `AGENTS.md` e o contrato atual. Uma mudança
   interna do servidor sem fronteira legada não precisa de pesquisa do client.
2. Localize o fluxo vivo com `rg` e siga a intenção desde o packet até o
   estado, persistência e resposta. Defina rejeições, replay/idempotência,
   concorrência e falha parcial relevantes ao fluxo antes de ligar a UI; não
   auditar domínios que a alteração não alcança.
3. Para extensão coordenada, documente formato, versão/capability, opcode/IDs,
   validação, fallback e compatibilidade nos dois lados.
4. Faça um patch pequeno, rode o teste focado, depois avance para o próximo
   patch. Não refatore código sem relação.

## Validação

Aplicar a matriz do `AGENTS.md`. Dentro de `wydgo748/`, selecionar o pacote e
os testes que exercitam a alteração (`go test ./internal/game -run <Teste>`,
por exemplo; substituir pelo alvo real). O filtro deve executar testes, não
apenas retornar sucesso sem casos. Incluir casos de rejeição e consumidores
afetados; reservar suíte ampla/vet para o lote com impacto transversal.

Se alterar os dois projetos, testar ambos. Um teste server-side não valida o
parser/UI do client. Compilar o alvo C++ afetado e registrar execução real
pendente quando não disponível; nunca instalar o candidato como efeito oculto
de uma checagem. Os estados de validação são os definidos no `AGENTS.md`.

Alterações ativas do client são source/assets. O conteúdo de
`references/client748/` é evidência histórica e não deve
ser sobrescrito.

## Referências por assunto

- Contratos do servidor: [emulator-contracts.md](references/emulator-contracts.md).
- Contratos transversais: buscar o subsistema em
  [repository-contracts.md](references/repository-contracts.md) e ler a seção pertinente.
- UI legada: [client-ui-748.md](references/client-ui-748.md).
- Acesso à evidência nativa: [ghidra-client748.md](references/ghidra-client748.md).
- Auditoria solicitada: [audit.md](references/audit.md).
- Retomada: [session-continuity.md](references/session-continuity.md).
