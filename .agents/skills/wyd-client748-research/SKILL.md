---
name: wyd-client748-research
description: Resolver evidência de comportamento ou contrato legado do client WYD 7.48 para adaptação/paridade, reutilizando fichas válidas; não acionar para servidor interno ou manutenção documental.
---

# Pesquisa do client 7.48

Use esta skill para a fronteira legada afetada ou interceptada por uma extensão.
Não exigir equivalente nativo para um contrato novo. O pipeline de paridade é
uma cadeia de evidências reutilizáveis, não uma sequência de comandos a repetir:

```text
catálogo -> callgraph -> fluxo observável -> adaptação -> validação
```

## Autoridade

Avalie e registre por frente o binário nativo 7.48, o projeto/descompilação
Ghidra, assets 7.48, source atual do TMProject, `wydgo748/`, testes e demais
referências. Marque cada fonte como `UTILIZADA`, `NÃO APLICÁVEL` ou
`CONTRADITÓRIA`, com justificativa. O binário e o Ghidra prevalecem; TMProject
7.69, guias e histórico são referências secundárias. W2PP, Secrets e Micronics
são excluídos e não podem orientar, validar ou contradizer a decisão.

## Classificação e gates

- `PARIDADE_NATIVA`: reproduz comportamento legado; exige ficha `TRACED` e,
  para wire/ABI/loader/recurso, `CONTRACT` antes da adaptação.
- `MODERNIZACAO_COMPATIVEL`: muda a implementação interna preservando o
  contrato; reutiliza a ficha comprovada e restringe a prova às fronteiras
  afetadas.
- `EXTENSAO_COORDENADA`: adiciona contrato ausente no nativo; exige formato,
  versão/capability quando necessário, IDs/opcodes sem colisão, validação,
  rollback/fallback e testes dos dois projetos. Não é paridade.

## O que rastrear

Para abrir uma frente, leia [research-method.md](references/research-method.md).
Para escrever ou atualizar uma ficha, use
[evidence-record.md](references/evidence-record.md). Ao escolher fronteiras,
consulte [subsystem-map.md](references/subsystem-map.md) somente se ainda não
houver fluxo/raiz definido. Acione catálogo apenas quando faltar seed ou o
censo for o objeto da tarefa.

Uma ficha descreve uma transição: entrada/evento, raiz nativa, callers/callees,
estado e mutações, erros, wire/ABI/recursos, source/server, decisão e
validação. Confirme no Ghidra chamadas indiretas, vtables, callbacks,
ownership, falha parcial, cleanup e teardown. Em lifecycle, resolva também
shutdown e logout/relogin ou marque `N/A` com justificativa.

Para packets, registre direção, opcode, tamanho, offsets, packing e signedness.
Para UI/assets, registre recurso materializado, binding, input, ownership,
fechamento e teardown. Ausência de caller textual não prova código morto.

## Continuação e saída

Reutilize a entrada única e a política de invalidação do `AGENTS.md`. Com ficha
suficiente e inputs válidos, avance para a adaptação sem nova exportação/Ghidra.
Se faltar prova, formule uma pergunta sobre a transição e escolha a consulta
que pode resolvê-la. Ao fechar o gate, implementar o delta liberado; não abrir
pesquisa de vizinhos sem dependência demonstrada. Lacuna real bloqueia somente
a edição dependente, e deve apontar a evidência ou acesso que falta.
Registre a ficha em `.agents/research/` e a decisão durável em `DOCS/` sem
duplicar conteúdo. Use `validate_research.py` uma vez por lote de fichas/schema
alterados, antes de aceitar a promoção. Mudança só nas instruções desta skill
não invalida as fichas nem exige reprocessar o corpus.

`STATICALLY VERIFIED`, `AUTOMATED TESTED` e `CLIENT-TESTED` são estados
distintos. Build ou teste estático nunca promove uma frente a `CLIENT_TESTED`.
