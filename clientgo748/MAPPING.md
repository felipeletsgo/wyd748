# Mapeamento do client WYD 7.48

Este diretório é o pacote autocontido de pesquisa e preparação do novo client
Go. Ele reúne o executável nativo usado como evidência, o cliente oficial e
seus assets, a descompilação exportada, o censo das funções, as fichas de
fluxo, os contratos do servidor e a documentação secundária necessária para
implementar cada unidade sem depender de caminhos externos.

## Resultado atual

O corpus nativo contém **4.146 funções**. O inventário local correlaciona:

- 4.146 entradas no censo (`references/catalog/functions.tsv`);
- 331 funções citadas pelas pesquisas existentes;
- 143 entradas `LOCATED`, 37 com evidência estática adicional e 5 com
  evidência de paridade/fluxo;
- 76 arquivos de fluxo (75 fichas + 1 template) em
  `references/research/flows/` e 47 exportações TSV;
- 3.961 entradas ainda `UNMAPPED` semanticamente.

Portanto, o pacote já possui o **mapeamento completo do corpus e das fontes**,
mas não declara que 4.146 funções foram semanticamente rastreadas. A maturidade
de cada entrada permanece no TSV e não é promovida por compilação ou por
semelhança com o TMProject.

O validador das fichas reproduz, no snapshot atual, `CONTRACT=46`, `TRACED=19`,
`LOCATED=8` e `UNMAPPED=2`. Esses números são das fichas de transição e não
devem ser somados aos estados por função do censo.

## Ordem de autoridade

1. código/testes atuais do WYD-Go e dados autoritativos;
2. binário nativo 7.48 e descompilação estudada no Ghidra;
3. assets oficiais 7.48 e comportamento observável;
4. fichas e exportações de pesquisa deste pacote;
5. TMProject e guias, apenas como comparação estrutural secundária.

W2PP, Secrets e Micronics foram explicitamente excluídos por serem fontes
bugadas. Não orientam, validam nem contradizem contratos.

## Artefatos principais

| Caminho | Conteúdo | Uso |
| --- | --- | --- |
| `CLIENT OFICIAL 7.48/` | executável e assets oficiais | baseline materializada e somente leitura |
| `DECOMP/` | exports de descompilação/disassembly | busca rápida e correlação |
| `references/catalog/functions.tsv` | 4.146 funções com estado e callers/callees | censo e fila de cobertura |
| `references/catalog/ghidra-functions.tsv` | catálogo Ghidra | xrefs, vtables e callgraph |
| `references/catalog/research-queue.tsv` | fila priorizada | próxima raiz de investigação |
| `references/research/flows/` | fichas por transição observável | evidência `TRACED`/`CONTRACT` |
| `references/research/exports/` | tabelas reproduzíveis de xrefs/fluxos | conferência sem reabrir toda a análise |
| `references/tmproject/` | snapshot de referência estrutural | comparação; não copiar código para o client Go |
| `references/server-source/` | wire/game/model relevantes | conferência do contrato server-authoritative |
| `references/server-data/` | skills, itens e NPCs autoritativos | dados consumidos pelos contratos |
| `references/skills/` | instruções e critérios de pesquisa | reprodução do método e dos gates |

## Mapa por subsistema

O mapa detalhado está em [`references/skills/subsystem-map.md`](references/skills/subsystem-map.md).
Ele separa bootstrap, transporte, login, cenas, movimento, entidades, combate,
score, inventário, NPC/trade, UI, render/assets e shutdown. Cada linha aponta
para fichas existentes ou registra explicitamente a lacuna como `UNMAPPED`.

O caminho crítico atual é:

```text
bootstrap -> transporte/login -> cenas -> dispatcher -> mundo
          -> logout/relogin -> shutdown/reconexão
```

As fichas existentes fecham apenas transições estreitas. Movimento completo,
combate/skills, score/equipamento, inventário geral, render e vários lifecycles
continuam como frentes de pesquisa, mesmo quando há código candidato no
TMProject.

## Procedência e fingerprint

O binário analisado é `client748/wyd.exe nativo+patches/WYD.exe`:

```text
SHA-256 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
Tamanho 2,060,288 bytes
```

O executável stock histórico é `WYDoriginal.exe`, SHA-256
`B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C`.
`client748/project.exe` é somente o candidato de validação da source ativa e
seu hash muda a cada build. A cópia observada neste snapshot tem SHA-256
`5082EA8CAA9CC649974F54A24A1FF8061D0BA0C27035A532DD989E9CA3E19155`.

## Estados e claims

- `UNMAPPED`: função presente no censo, sem fluxo nativo completo;
- `LOCATED`: entrada ou relação encontrada, mas callers/callees/lifecycle ainda
  incompletos;
- `TRACED`: transição observável, estado, erros e lifecycle rastreados;
- `CONTRACT`: além de `TRACED`, wire/ABI/recurso testável;
- `CLIENT_TESTED`: fluxo executado no executável candidato.

`IMPLEMENTED`, `STATICALLY VERIFIED`, `AUTOMATED TESTED` e `CLIENT-TESTED` são
estados de entrega separados. Nenhum build verde substitui o teste real do
client.

## Como continuar

1. Escolher a próxima raiz em `references/catalog/research-queue.tsv`.
2. Partir da função/feature viva da source, localizar a candidata nativa e
   abrir callers, callees, vtables, callbacks e teardown no Ghidra.
3. Atualizar uma ficha em `references/research/flows/` com a matriz de fontes,
   decisão (`portar`, `manter`, `modernizar`, `estender` etc.) e lacunas.
4. Só então implementar a unidade Go e validar parser, lifecycle e contrato.
5. Executar os comandos reproduzíveis de `references/REPRODUCE.md`.

O snapshot externo continua preservado para auditoria, mas a implementação do
client Go deve depender apenas deste diretório e das APIs do módulo raiz. Os
arquivos sob `references/tmproject/` são leitura de comparação, não código
ativo.
