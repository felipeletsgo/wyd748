---
name: wydgo748-feature
description: Implementar features do servidor WYD-Go 7.48 autoritativo e, quando necessário, o contrato coordenado com o client TMProject.
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

1. Verifique `git status --short`, `HEAD`, diff scoped e o contrato/documento
   atual; preserve mudanças alheias.
2. Localize o fluxo vivo com `rg` e siga a intenção desde o packet até o
   estado, persistência e resposta. Defina rejeições, replay/idempotência,
   concorrência e falha parcial antes de ligar a UI.
3. Para extensão coordenada, documente formato, versão/capability, opcode/IDs,
   validação, fallback e compatibilidade nos dois lados.
4. Faça um patch pequeno, rode o teste focado, depois avance para o próximo
   patch. Não refatore código sem relação.

## Validação

No servidor:

```powershell
Push-Location .\wydgo748
go test -count=1 ./...
Pop-Location
```

Quando houver source client alterada, valide também o build de
`tmproject/Build-Client.ps1` e o fluxo real no client. Relate separadamente
`STATICALLY VERIFIED`, `AUTOMATED TESTED` e `CLIENT-TESTED`.

Alterações ativas do client são source/assets. O conteúdo de
`references/client748/` é evidência histórica e não deve
ser sobrescrito.

Leia as referências em `references/` somente quando o escopo exigir contrato,
UI, Ghidra, auditoria ou continuidade.
