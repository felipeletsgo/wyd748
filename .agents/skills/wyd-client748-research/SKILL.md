---
name: wyd-client748-research
description: Investigar e documentar a fronteira nativa do client WYD 7.48, correlacionando-a com a source atual; use para paridade, compatibilidade e integração segura de extensões client/server.
---

# Pesquisa do client WYD 7.48

Esta skill produz evidência reproduzível sem transformar toda melhoria do
TMProject em uma reconstrução arqueológica. Use-a antes de `wyd-go-feature`
quando uma mudança tocar comportamento, protocolo, ABI, UI, input, render,
assets ou lifecycle.

Em cada frente, avalie e registre as fontes confiáveis disponíveis: binário
nativo 7.48, projeto Ghidra e descompilação estudada, assets 7.48, source atual,
WYD-Go e testes, TMProject, guias e demais fontes do projeto. Marque cada uma
como `UTILIZADA`, `NÃO APLICÁVEL` ou `CONTRADITÓRIA`, com justificativa. W2PP,
Secrets e Micronics estão excluídos: são fontes bugadas, sem força para validar,
contradizer ou orientar código e contratos. O estudo
da descompilação no Ghidra e o binário nativo 7.48 são obrigatórios antes de
adaptar qualquer fronteira listada acima.

## Classifique antes de pesquisar

| Modo | Objetivo | Gate |
| --- | --- | --- |
| `PARIDADE_NATIVA` | reproduzir ou corrigir comportamento legado | ficha `TRACED`; `CONTRACT` para wire/ABI/loader/recurso |
| `MODERNIZACAO_COMPATIVEL` | manter contrato externo com estrutura interna superior | reutilizar a ficha do contrato e provar somente as fronteiras afetadas |
| `EXTENSAO_COORDENADA` | adicionar feature/asset/contrato ausente no nativo | contrato client/server explícito e testes dos dois lados; Ghidra apenas nas integrações legadas |

`LOCATED` bloqueia edição dependente de um claim nativo incompleto. Não bloqueia
uma extensão que declara corretamente não possuir equivalente nativo. Não crie
uma ficha falsa nem promova maturidade para justificar feature nova.

Código e assets já existentes na source são presumidos intencionais. Antes de
removê-los, demonstre incompatibilidade em wire/ABI, recurso, lifecycle,
server-side ou runtime.

## Fast path de continuidade

1. Ler esta skill, o `AGENTS.md` scoped e somente o handoff do escopo.
2. Conferir `git status`, diff scoped e o fingerprint dos inputs registrados.
3. Se binário/corpus/source relevante não mudou, reutilizar hash, triagem,
   exports, seeds e claims confirmados; não refazer leitura ampla.
4. Partir da função/feature viva da source, localizar a candidata nativa e
   seguir apenas callers/callees/vtables que decidem o delta.
5. Registrar cada descoberta no mesmo ciclo e implementar lotes pequenos assim
   que a fronteira estiver decidida.

Execute o triador do catálogo somente para nova raiz sem fila válida ou quando
seus inputs mudarem. Execute `validate_research.py` somente quando fichas ou seu
schema mudarem.

## Evidência nativa

Uma unidade de paridade é uma transição observável:

```text
entrada/evento
-> raiz nativa e callers/callees relevantes
-> estado, mutações, erros e teardown
-> wire/ABI/recursos aplicáveis
-> source atual e servidor
-> delta e decisão
-> validação
```

No projeto Ghidra, confirme chamadas indiretas, vtables, callbacks, ownership e
lifecycle que sustentam a decisão. O corpus e os fingerprints escolhem
candidatos; ausência textual e `EXACT_MATCH` não provam semântica.

Para wire/ABI, provar direção, opcode, tamanho, offsets, packing e signedness no
packet final. Para UI/assets, provar recurso materializado, binding, input,
ownership, fechamento e teardown. Lifecycle `TRACED` cobre entrada observável,
transições, vptr/receptor, falha parcial, cleanup, shutdown e logout/relogin, ou
marca `N/A` com justificativa.

Detalhes dos modos `pointers`, `relatives`, `bodyrefs`, `rawrelatives`,
`virtualslot`, headless e fingerprints ficam em
[references/research-method.md](references/research-method.md). Leia essa
referência ao executar ou interpretar essas operações, não em toda retomada.

## Modernização e extensão

Antes de escolher estrutura 7.48 ou posterior, compare por claim:

- wire/ABI e compatibilidade com a fronteira existente;
- assets/IDs realmente materializados;
- lifecycle, ownership, teardown e relogin;
- suporte e autoridade server-side;
- manutenção, clareza e possibilidade de evolução coordenada.

Prefira a estrutura tecnicamente superior quando esses limites forem
compatíveis. Se um contrato novo for necessário e ambos os lados estiverem no
escopo, documente formato, versionamento/capability quando aplicável,
IDs/opcodes sem colisão, validação de entrada, rollback e fallback. Depois teste
client e servidor. Não chame isso de paridade nativa.

TMProject e guias podem fornecer nomes, arquitetura e algoritmos candidatos.
Não fornecem por semelhança os fatos do executável 7.48; W2PP, Secrets e
Micronics não fazem parte da pesquisa.

## Maturidade nativa

| Estado | Significado |
| --- | --- |
| `UNMAPPED` | entrada nativa não localizada |
| `LOCATED` | entrada localizada, fluxo incompleto |
| `TRACED` | transição, estado, efeitos, erros e lifecycle rastreados |
| `CONTRACT` | `TRACED` mais wire/ABI/recurso decidido e testável |
| `CLIENT_TESTED` | fluxo real executado no `project.exe` registrado |

A maturidade é por claim/transição e mede conhecimento nativo, não qualidade de
uma extensão. `IMPLEMENTED`, `STATICALLY VERIFIED` e `AUTOMATED TESTED` são
estados de entrega separados.

## Entrega

Atualize a ficha apenas com evidência nova e classifique claims como
`CONFIRMED`, `PROBABLE`, `HYPOTHESIS` ou `REJECTED`. A decisão deve ser uma de:
`portar`, `manter`, `modernizar`, `estender`, `remover`, `proteger como
opcional` ou `não implementar`.

Documente imediatamente função/endereço, relação source, motivo da decisão e
lacuna restante. Handoff guarda só estado operacional e próximo comando; não
duplica pseudocódigo, logs ou fatos estáveis da ficha.

Ao preparar uma adaptação, registre também a procedência de cada unidade
relevante (`7.48`, `TMProject 7.69+` ou implementação local) e se os comentários
descrevem evidência confirmada ou apenas uma lacuna. A documentação didática em
português não pode promover por redação uma hipótese para fato nativo. Um stub
sem contrato suficiente permanece explicitamente pendente.

Ferramentas usuais:

```powershell
python .agents/skills/wyd-client748-research/scripts/query_corpus.py stats --repo .
python .agents/skills/wyd-client748-research/scripts/query_corpus.py flow 0055890a
python .agents/skills/wyd-client748-research/scripts/validate_research.py --repo .
```

Leia [references/evidence-record.md](references/evidence-record.md) ao criar ou
promover ficha e somente a linha relevante de
[references/subsystem-map.md](references/subsystem-map.md) ao abrir nova frente.
Para seleção de modelo, preferir `gpt-5.6-sol`/`xhigh` apenas quando o ambiente
confirmar que essa opção está disponível.
