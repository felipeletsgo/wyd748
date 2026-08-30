# Estratégia de catálogo e cobertura 7.48

## Objetivo

Manter um censo completo das 4.146 funções nativas do client 7.48 e convertê-lo
gradualmente em conhecimento funcional reproduzível. O censo mede existência e
localização; a compreensão é medida por transições observáveis e evidência.

## Artefatos

| Artefato | Papel | Pode decidir comportamento? |
| --- | --- | --- |
| `inventory/functions.tsv` | índice de decompilação, source refs e relações preliminares | não |
| `inventory/ghidra-functions.tsv` | catálogo estrutural exportado do Ghidra | não, sozinho |
| `inventory/research-queue.tsv` | fila histórica e motivos já levantados | não, sem revalidação |
| `flows/<subsystem>/<transition>.md` | evidência por transição | sim, no estado exigido |
| `handoffs/client748-research-program.md` | estado operacional e próximo ponto de entrada | não |

Antes de uma campanha nova, validar a identidade do binário e a correspondência
das 4.146 entradas. Em continuação, reutilizar o resultado se caminho, tamanho,
mtime e corpus registrado não mudaram. Se hash ou cardinalidade mudar,
interromper a interpretação de endereços até regenerar o corpus.

## Unidade de trabalho

Uma unidade contém uma entrada observável, uma função viva da source ou raiz
nativa e somente o caminho que decide o delta:

```text
função/feature source ou evento/input/packet
  -> candidatos e caller/callback
  -> função nativa 7.48
  -> callees e dados
  -> estado/global/objeto mutado
  -> saída, erro, fechamento e relogin
  -> equivalente source/TMProject
  -> decisão de adaptação e validação
```

Funções auxiliares são agrupadas pela transição. Preferir busca source→native,
seeds confirmados e propagação pelo callgraph. Não criar 4.146 fichas isoladas
nem esperar o censo inteiro para adaptar um fluxo crítico já decidido.

## Lanes e ranking

O triador usa somente campos existentes em `functions.tsv` e gera uma ordem
determinística. A pontuação é heurística e não é evidência:

- evidência já documentada: grande bônus para fechar primeiro a lacuna;
- referência direta na source: bônus alto, pois há risco de divergência ativa;
- muitos callers ou referências de entrada: bônus de raiz/dispatcher;
- muitos callees: bônus menor para delimitar o componente;
- ausência de caller direto com referências de dados/fluxo: lane de callback;
- thunk: pequena penalidade para evitar gastar a primeira rodada em wrappers.

O motivo e a próxima ação devem acompanhar cada linha. Empates são resolvidos
pelo endereço hexadecimal, mantendo a fila reproduzível entre sessões.

## Cobertura por fase

1. **Censo** — todas as entradas presentes, únicas, com hash e schema válido.
2. **Triagem** — lane, pontuação e raiz escolhida com motivo.
3. **Rastreamento** — ficha `TRACED`: entrada, callers/callees, estado, efeitos,
   erros e teardown/relogin aplicáveis comprovados no Ghidra.
4. **Contrato** — ficha `CONTRACT` para ABI, packet, offsets, packing,
   signedness, loaders ou recursos.
5. **Adaptação** — para paridade, alterar somente o claim autorizado pela ficha;
   para modernização ou extensão coordenada, preservar o claim nativo separado
   e validar o contrato externo ou o novo contrato client/server.
6. **Validação** — testes estáticos/automatizados e, quando alegado,
   `CLIENT_TESTED` no `client748/project.exe` com hash registrado.

Uma função pode continuar `LOCATED` mesmo estando presente no catálogo. Isso
bloqueia claims dependentes de paridade, não uma extensão deliberadamente nova.
Uma função sem caller pode ser válida, mas precisa de xref indireto, vtable,
callback ou evidência de runtime antes de se tornar contrato nativo.

## Critério de encerramento da campanha

Não basta contar linhas. O client pode avançar por fluxos críticos sem concluir
o censo inteiro. A campanha só pode ser apresentada como cobertura nativa
completa quando cada fluxo crítico tiver ficha e decisão; o restante deve estar
explicitamente classificado como wrapper, runtime, helper, código não alcançado
ou lacuna. Nenhuma categoria desconhecida pode ser escondida em “compreendida”.

Em cada sessão, registrar apenas:

- o que foi confirmado;
- o que foi promovido de estado;
- o que foi adaptado e validado;
- o bloqueio exato;
- o próximo entry point ou comando executável.
