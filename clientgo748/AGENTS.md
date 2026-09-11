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

## Ciclo incremental

Para continuidade ou implementação em lote no `clientgo748`, usar por padrão:

```text
status + HEAD + diff scoped -> patch pequeno -> teste focado -> próximo patch
```

O handoff é contexto transitório; a árvore atual prevalece. Não repetir busca
global, releitura de referência ou reconstrução de diff antigo quando `HEAD` e
a worktree já determinam o estado real. Executar `Verify-Fast.ps1`, gates
contratuais e build completo somente no fechamento proporcional do lote ou
quando a alteração atual exigir explicitamente esse gate.

## Gate mínimo

```powershell
pwsh -NoProfile -File .\clientgo748\Verify-Fast.ps1
git diff --check
```

`Build-ClientGo.ps1` é reservado para gerar o executável e para o smoke test
Windows. Ele executa os testes por padrão; quando o `Verify-Fast.ps1` acabou de
passar sem mudanças nos inputs, pode ser chamado com `-SkipTests` para não
repetir a mesma suíte:

```powershell
pwsh -NoProfile -File .\clientgo748\Build-ClientGo.ps1 -Configuration Debug -SkipTests
```

`-SkipTests` nunca substitui o gate rápido; só é válido depois de uma execução
bem-sucedida dele no mesmo estado da árvore.

O executável em `clientgo748/bin/` é artefato local e não deve ser versionado.
