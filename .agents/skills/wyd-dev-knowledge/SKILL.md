---
name: wyd-dev-knowledge
description: >-
  Consultar conhecimento histórico e técnico de WYD sobre protocolo,
  criptografia, fórmulas, versões, tooling e inventário de funções 7.5x. É uma
  referência secundária: usar depois do código/testes atuais e, para o client
  7.48, somente depois do gate Ghidra de wyd-go-feature.
---

# WYD development knowledge

## Limite de autoridade

Esta skill ajuda a encontrar hipóteses, nomes, algoritmos e pontos de pesquisa.
Ela não promove conteúdo de W2PP, Secrets, Micronics, TMProject ou fórum ao
contrato atual. ABI, offsets, endereços, structs, packets, IDs e comportamento
do client precisam ser revalidados na versão alvo.

Mapa de autoridade para resolver conflitos:

1. código, testes e dados autoritativos atuais do `wyd-go`;
2. Ghidra e binário exato do client 7.48 para client/ABI/UI;
3. W2PP como referência semântica legível;
4. Secrets 7.54 como referência próxima do servidor nativo;
5. Micronics como implementação/hook legado;
6. fórum, catálogo e demais conhecimento histórico.

Uma fonte inferior não corrige uma superior por consenso. Algoritmo pode ser
portável; endereço, layout, opcode, constante e lifecycle permanecem presos à
versão até prova contrária.

Para qualquer tarefa de implementação ou auditoria do WYD-Go, ler primeiro
`../wyd-go-feature/SKILL.md`. Se houver client 7.48, cumprir também o gate
Ghidra antes de consultar esta base.

Para o client 7.48, qualquer executável, patcher ou `.ps1` histórico é somente
evidência read-only. Uma hipótese útil desta base só pode virar produto depois
de ser comprovada no 7.48 e adaptada em `client-source/` ou nos assets
consumidos por `client748/project.exe`. Nunca executar, editar, verificar ou
reaplicar a cadeia binária histórica, nem oferecê-la como fallback.

## Uso econômico

1. Definir uma pergunta concreta e palavras-chave.
2. Pesquisar o índice abaixo e abrir somente as seções correspondentes de
   [`references/knowledge-base.md`](references/knowledge-base.md).
3. Se a seção apontar para um catálogo, abrir somente as entradas relevantes.
4. Voltar ao código atual e confirmar a hipótese com implementação, dados,
   descompilação ou teste; no client, portar o resultado para source/assets 7.48.
5. Para cada fato usado, registrar versão alvo, artefato/fonte, evidência
   observada, grau de portabilidade e decisão tomada.
6. Registrar no resultado o que foi confirmado, descartado ou permaneceu
   histórico.

Não carregar a base inteira por padrão e não copiar grandes trechos para o
contexto da sessão.

## Índice de tópicos

| Pergunta | Procurar na base |
| --- | --- |
| Histórico de versões e compatibilidade | `Version families`, `Compatibility` |
| Login, auth, chaves e criptografia | `Cryptography`, `Login`, `keys` |
| Packets, headers e opcodes | `Protocol`, `packet`, `opcode` |
| Fórmulas de score, combate ou refino | `Formulas`, `damage`, `refine` |
| Mapas, mobs, itens e tabelas | `Data files`, `ItemList`, `Mob` |
| Ferramentas, unpackers e formatos | `Tooling`, `MSH`, `WYS`, `BON`, `ANI` |
| Funções e endereços históricos | `Function inventory`, `addresses` |
| Conteúdo do fórum WebCheats | `WebCheats`, depois o catálogo dedicado |

## Saída mínima

Ao usar esta skill, distinguir explicitamente:

```text
HISTÓRICO       encontrado em fonte secundária, ainda não validado
CORROBORADO     compatível com mais de uma fonte, mas não autoritativo
CONFIRMADO      comprovado no código/dado/binário atual da versão alvo
NÃO APLICÁVEL   pertence a outra versão ou ABI
```

Nunca substituir evidência atual por consenso histórico.

Se a versão do artefato, a proveniência ou a forma de validar estiver ausente,
o fato continua `HISTÓRICO`. Duas fontes derivadas da mesma base não bastam para
`CORROBORADO`; e somente observação no código, dado ou binário exato da versão
alvo permite `CONFIRMADO`.
