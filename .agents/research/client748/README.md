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

O candidato source mais recente fica registrado no handoff do programa.
Recalcular antes de usar: `client748/project.exe` muda a cada build.

## Organização e maturidade

- `flows/TEMPLATE.md`: ponto de partida obrigatório para uma transição
  observável.
- `flows/<subsistema>/`: uma ficha por entrada/saída; não misturar abertura,
  drag, uso, equipamento, venda e rollback em uma ficha genérica.
- `exports/`: relatórios focados que aceleram a busca; passar funções, vtables e
  slots decisivos explicitamente ao `ExportWydFlow.java`. Não versionar exports
  exploratórios amplos sem claim e ficha que os consumam.
- `inventory/scene-transition-evidence-log.md`: ledger reproduzível dos 56
  exports exploratórios da rodada de lifecycle, com a pergunta de cada recorte,
  interpretação atual e lacuna seguinte. É índice de evidência, não ficha, e
  portanto não altera sozinho a maturidade de `scene-transition.md`.
- `.agents/skills/wyd-client748-research/references/evidence-record.md`:
  schema, citações e estados `UNMAPPED`/`LOCATED`/`TRACED`/`CONTRACT`/
  `CLIENT_TESTED`.
- `.agents/handoffs/`: estado operacional curto, próximo comando e riscos; um
  handoff nunca substitui a evidência na ficha nem o projeto Ghidra.

Estado atual do mapa:

| Fluxo | Estado | Ficha |
| --- | --- | --- |
| Gate de tamanho por opcode | `LOCATED` | `flows/transport/packet-size-gate.md` |
| Consulta de byte do array de animação `0x1C1/0x2C2` | `CONTRACT` | `flows/transport/bone-animation-array-probe.md` |
| Foco, IME e lifecycle de controles | `TRACED` | `flows/ui/control-focus-ime-lifecycle.md` |
| Transição e troca de cenas | `LOCATED` | `flows/lifecycle/scene-transition.md` |
| Disconnect TCP e retorno à seleção | `CONTRACT` | `flows/transport/socket-disconnect-return-selectserver.md` |
| Reconstrução Field após migração de servidor | `CONTRACT` | `flows/lifecycle/field-scene-rebuild-after-server-move.md` |
| Logout de personagem, seleção e relogin | `CONTRACT` | `flows/lifecycle/character-logout-selectchar-relogin.md` |
| Fechamento da aplicação e shutdown global | `CONTRACT` | `flows/lifecycle/application-close-global-shutdown.md` |
| Atualização e consumo dos filtros AutoKick `0x2C8` | `TRACED` | `flows/transport/autokick-filter-update.md` |
| Chat local bidirecional `0x333` | `CONTRACT` | `flows/ui/local-chat-message.md` |
| Aviso textual da cena `0x101` | `CONTRACT` | `flows/ui/message-panel-text.md` |
| Notificação de billing `0x194` | `CONTRACT` | `flows/ui/billing-notice.md` |
| Avisos indexados/parametrizados `0x105/0x106` | `UNMAPPED` (extensão coordenada) | `flows/ui/indexed-parameterized-message-extension.md` |
| Atalho F para consumível vinculado ao equipamento | `CONTRACT` | `flows/ui/equipped-item-matched-consumable-shortcut.md` |
| Atalho E para poção especial | `CONTRACT` | `flows/ui/special-potion-shortcut.md` |
| Loader da lista TOTO | `CONTRACT` | `flows/ui/toto-list-loader.md` |
| Seleção, teclado e fechamento TOTO | `TRACED` | `flows/ui/toto-selection-close.md` |
| Compra e materialização do bilhete TOTO | `CONTRACT` | `flows/transport/toto-buy.md` |
| Aposta, rolagem e resultado Gamble/Jackpot | `CONTRACT` | `flows/ui/gamble-jackpot.md` |
| Abertura, render e compra de skill no mestre | `CONTRACT` | `flows/ui/skill-master-purchase.md` |
| Caixa visual e escala de itens em grids | `TRACED` | `flows/ui/grid-item-mesh-scale.md` |
| Composição lado a lado de Shop e Inventory | `TRACED` | `flows/ui/shop-inventory-layout.md` |
| Composição lado a lado de Trade e Inventory | `TRACED` | `flows/ui/trade-inventory-layout.md` |
| Posição compartilhada de AutoTrade, Cargo e Inventory | `TRACED` | `flows/ui/auto-trade-inventory-layout.md` |
| Composição dos seis ItemMix com Inventory | `TRACED` | `flows/ui/native-mix-inventory-layout.md` |
| Posição inicial e toggle do menu inferior direito | `TRACED` | `flows/ui/system-menu-initial-layout.md` |
| Layout e lifecycle do painel Party | `CONTRACT` | `flows/ui/party-panel-layout-lifecycle.md` |
| Envio e aplicação de motion/emote `0x36A` | `CONTRACT` | `flows/transport/motion-emote-roundtrip.md` |
| Recuperação de entidade ausente `0x369` | `CONTRACT` | `flows/transport/missing-entity-request.md` |
| Pedido de renascimento/recall `0x289` | `CONTRACT` | `flows/transport/restart-recall-request.md` |
| Keepalive periódico `0x3A0` | `CONTRACT` | `flows/transport/keepalive-ping.md` |
| Menu de interação Ctrl+clique direito em jogador | `TRACED` | `flows/ui/player-interaction-menu-lifecycle.md` |
| Layout e lifecycle do painel Quest | `TRACED` | `flows/ui/quest-panel-layout-lifecycle.md` |
| Composição lado a lado de Character, Skill e Inventory | `TRACED` | `flows/ui/feature-panel-layout.md` |
| Atualização de Att Speed, C.POINT, HOLD e Kingdom em Character | `TRACED` | `flows/ui/character-stat-fields-update.md` |
| Posição do painel de seleção de personagem | `TRACED` | `flows/ui/select-character-layout.md` |
| Posição e lifecycle da seleção de servidor | `TRACED` | `flows/ui/server-selection-layout-lifecycle.md` |
| Exibição do Premium Firework `0x3CA` | `CONTRACT` | `flows/ui/premium-firework-display.md` |
| Sincronização de estado e controles C.C | `TRACED` | `flows/ui/cc-auto-combat-state-sync.md` |
| Alternância e aplicação autoritativa do PK Mode `0x399` | `CONTRACT` | `flows/ui/pk-mode-toggle-lifecycle.md` |
| Lifecycle da dívida Held EXP por morte PvP | `CONTRACT` | `flows/combat/pvp-death-held-exp-lifecycle.md` |
| `SetMyHumanMagic` nativo sem efeito | `STATICALLY_EVIDENCED` | `inventory/set-my-human-magic-noop.md` |

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
