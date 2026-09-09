# Instruções do clientgo748

## Escopo

Este subtree é uma reimplementação nova do client WYD 7.48 em Go. O TMProject
serve apenas como referência estrutural. Não copiar código, ABI, ownership,
estado global ou workaround do TMProject sem uma decisão documentada e uma
validação própria.

## Fontes

Para comportamento legado, usar nesta ordem: código e testes atuais do projeto,
assets 7.48, binário/descompilação estudada no Ghidra, e somente depois o
TMProject como comparação secundária. W2PP, Secrets e Micronics não são fontes
aceitas.

## Regras de implementação

- manter o servidor autoritativo;
- validar limites, tamanhos, versões e estados antes de mutar dados;
- tornar ownership, nulabilidade, cancelamento e teardown explícitos;
- não introduzir opcode, asset ou regra de gameplay sem contrato;
- documentar a procedência e o modo (`PARIDADE_NATIVA`,
  `MODERNIZACAO_COMPATIVEL` ou `EXTENSAO_COORDENADA`);
- escrever mensagens visíveis ao usuário em inglês;
- adicionar teste focado antes de declarar uma fronteira implementada.

## Gate mínimo

```powershell
pwsh -NoProfile -File .\clientgo748\Build-ClientGo.ps1 -Configuration Debug
git diff --check
```

O executável em `clientgo748/bin/` é artefato local e não deve ser versionado.
