# Pesquisa versionada do client WYD 7.48

Este diretório é a memória técnica reproduzível do programa de paridade do
client 7.48. O objetivo é recuperar o fluxo real do executável nativo e
compará-lo com `client-source/tmproject` e o WYD-Go antes de qualquer edição
comportamental. A conversa, um comentário da source ou o TMProject 7.69+ podem
orientar a busca, mas não são contrato.

## Como retomar

1. Ler `AGENTS.md`, o `AGENTS.md` da subtree afetada e
   `.agents/skills/wyd-client748-research/SKILL.md`.
2. Ler apenas o handoff do programa em
   `.agents/handoffs/client748-research-program.md` e, se o escopo for a
   paridade visual/funcional, também `.agents/handoffs/client748-parity.md`.
3. Comparar os fatos do handoff com `git status --short`, os arquivos atuais e
   os hashes; hashes de `project.exe`, logs, dumps e linhas são voláteis.
4. Ler o guia Ghidra e a referência de UI somente quando o domínio exigir.
5. Escolher ou criar uma ficha por transição, consultar o projeto Ghidra e
   executar `validate_research.py` antes de promover a maturidade.

O hash canônico da referência Ghidra usada no estado atual é:

```text
client748/wyd.exe nativo+patches/WYD.exe
8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
```

O candidato source mais recente registrado no handoff do programa é
`client748/project.exe`, SHA-256
`F8251714775601720307940598522E6D2924E5C61DAB300728F949FE0C8A380B`.
Recalcular antes de usar: o candidato muda a cada build.

## Organização e maturidade

- `flows/TEMPLATE.md`: ponto de partida obrigatório para uma transição
  observável.
- `flows/<subsistema>/`: uma ficha por entrada/saída; não misturar abertura,
  drag, uso, equipamento, venda e rollback em uma ficha genérica.
- `exports/`: relatórios focados que aceleram a busca; passar funções, vtables e
  slots decisivos explicitamente ao `ExportWydFlow.java`. Não versionar exports
  exploratórios amplos sem claim e ficha que os consumam.
- `.agents/skills/wyd-client748-research/references/evidence-record.md`:
  schema, citações e estados `UNMAPPED`/`LOCATED`/`TRACED`/`CONTRACT`/
  `CLIENT_TESTED`.
- `.agents/handoffs/`: estado operacional curto, próximo comando e riscos; um
  handoff nunca substitui a evidência na ficha nem o projeto Ghidra.

Estado atual do mapa:

| Fluxo | Estado | Ficha |
| --- | --- | --- |
| Gate de tamanho por opcode | `LOCATED` | `flows/transport/packet-size-gate.md` |
| Foco, IME e lifecycle de controles | `LOCATED` | `flows/ui/control-focus-ime-lifecycle.md` |

`LOCATED` permite investigação e documentação, não edição comportamental.
`TRACED` exige callers/callees e estado/erros fechados; `CONTRACT` acrescenta
wire/ABI/recursos testáveis; `CLIENT_TESTED` exige o fluxo real no
`client748/project.exe`.

## Limites do corpus

O corpus textual auxiliar está em
`%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled` e corresponde ao
projeto `%USERPROFILE%\Tools\GhidraProjects\WYD748Native_20260821.gpr`.
Ele possui 4.146 funções e atualmente indexa 108 referências nativas citadas no
repositório;
`FUN_00452733` e `FUN_0047E4D6` não aparecem no índice. Essa ausência textual
nunca prova ausência no binário: resolver diretamente no projeto Ghidra, com
xrefs e chamadas indiretas, antes de concluir.

Comandos úteis a partir da raiz do repositório:

```powershell
python .agents/skills/wyd-client748-research/scripts/query_corpus.py stats --repo .
python .agents/skills/wyd-client748-research/scripts/query_corpus.py flow 0055890a
python .agents/skills/wyd-client748-research/scripts/query_corpus.py search "FieldScene2.bin"
python .agents/skills/wyd-client748-research/scripts/validate_research.py --repo .
```

Não armazenar binários, export completo do Ghidra, varredura ampla de xrefs,
dumps, credenciais ou pseudocódigo em massa neste diretório. Binários e scripts
em `client748/wyd.exe nativo+patches/` são somente evidência histórica; o único
candidato executável é `client748/project.exe`, produzido pelo build da source.
