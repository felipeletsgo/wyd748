---
id: pk-mode-toggle-lifecycle
title: Alternancia e aplicacao autoritativa do PK Mode
subsystem: ui-input-wire-combat
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Alternancia e aplicacao autoritativa do PK Mode

## Pergunta

Como o client nativo 7.48 alterna o PK Mode por teclado, comando e controle de
UI, qual contrato envia ao servidor e em quais caminhos o WYD-Go deve aplicar
esse estado para impedir PvP quando o modo esta desligado?

## Fronteira de evidência

- Executavel: `client748/wyd.exe nativo+patches/WYD.exe`, com o SHA-256 do
  frontmatter.
- Projeto Ghidra: `WYD748Native_20260821.gpr`.
- Raiz nativa: `FUN_0044ECAE`, correspondente a `TMFieldScene::SetPK`.
- Callers diretos: `FUN_004523FA`, `FUN_004541F3` e `FUN_004662C5`.
- Callee de transporte: `FUN_0055F2DD`.
- Source atual: `TMFieldScene::SetPK` em
  `client-source/tmproject/Projects/TMProject/TMFieldScene.cpp`.
- Servidor atual: `internal/game/teleports.go`, `handlers.go`,
  `skill_pvp.go`, `skill_summons.go` e `character_session.go`.
- Referencia funcional secundaria: `DOCS/GUIA/GUIAS_JOGABILIDADE.md`.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0044ECAE` e alcancada por tres entradas diretas:

| Entrada | Caller | Condicao |
| --- | --- | --- |
| teclado | `FUN_004523FA` | `k`, `K`, crase ou `~` |
| comando traduzido | `FUN_004541F3` | token `0x9C9C` |
| clique | `FUN_004662C5` | controle legado `0x132`/`306` |

A rotina inverte o estado global entre `0` e `1`, atualiza o estado visual do
controle associado e envia a nova intencao ao servidor. O modo nao calcula
dano nem transforma o client em autoridade de combate.

### Callers

- `FUN_004523FA` recebe o input de teclado e encaminha `k`, `K`, crase e `~`.
- `FUN_004541F3` traduz o comando `0x9C9C` para a mesma alternancia.
- `FUN_004662C5` recebe o clique do controle legado `0x132`/`306`.

Os tres callers entregam a mesma `TMFieldScene` receptora a
`FUN_0044ECAE`; nao existem caminhos paralelos com contratos de PK distintos.

### Callees

`FUN_0044ECAE` chama o setter virtual do estado global, o helper adjacente
`FUN_0044ECA3`, resolve o controle legado `306`, usa os setters virtuais do
controle quando ele existe e envia o pacote por `FUN_0055F2DD`.

O recurso 7.48 pode nao materializar o botao moderno `65786`. Na source
recompilavel, tanto esse ponteiro opcional quanto o controle legado `306` sao
protegidos. A ausencia visual nao interrompe a alternancia nem o envio de
`0x399`; isso elimina o crash observado ao pressionar `K`.

## Aplicacao autoritativa

Receber e publicar `Player.PKMode` nao basta: o servidor precisa consultar o
estado antes de iniciar qualquer dano jogador contra jogador. A regra comum
`World.canInitiatePvP` e aplicada antes de efeitos consumiveis em tres
fronteiras:

1. ataque fisico contra jogador em `World.onAttack`, antes de confirmar o
   relogio anti-speed, cancelar trade, remover Hide ou causar dano;
2. selecao de alvos jogadores em `World.skillPlayerTargets`, antes de mana,
   cooldown, remocao de Hide e execucao da skill;
3. alvo jogador de summon em `World.summonTarget`, sem alterar o ramo de mobs.

O gate exige atacante e alvo validos e `attacker.PKMode == true`. Excecoes de
Kingdom, Castle ou Guild War deverao ser autorizadas explicitamente quando o
estado desses eventos existir; nao fazem fallback silencioso para PvP comum.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondicao | Funcao/call | Resultado | Efeitos recusados |
| --- | --- | --- | --- | --- |
| tecla/comando/clique | FieldScene ativa | callers -> `FUN_0044ECAE` | estado local alternado e `0x399` enviado | controle opcional ausente nao causa crash |
| `0x399 = 1` valido | personagem no mundo | `World.onPKMode` | `Player.PKMode=true`, `0x166` publicado | trade atual e cancelado |
| `0x399 = 0` valido | personagem no mundo | `World.onPKMode` | `Player.PKMode=false`, `0x166` publicado | novos ataques PvP sao bloqueados |
| ataque fisico PvP desligado | alvo jogador valido | `World.onAttack` | sem dano | relogio, trade, Hide e alvo nao mudam |
| skill PvP desligada | alvo jogador valido | `World.skillPlayerTargets` | nenhum alvo jogador | MP, cooldown, Hide e HP nao mudam |
| summon contra jogador desligado | ordem do dono | `World.summonTarget` | alvo recusado | summon nao persegue nem ataca jogador |
| ataque contra mob desligado | mob valido | caminhos PvE | ataque permitido | nenhum gate PK e aplicado ao mob |
| logout/desconexao | personagem deixa o mundo | `removePlayerFromWorld` | estado efemero descartado | PK nao persiste na conta |
| relogin | nova entrada no mundo | reset de sessao do personagem | `Player.PKMode=false` | exige nova ativacao explicita |

### Vtables, vptrs e receptores

`FUN_0044ECAE` opera sobre a instancia receptora de `TMFieldScene` entregue
pelos tres callers. O setter visual do controle e resolvido pela vtable do
controle somente quando o root legado `306` existe; a transicao principal nao
depende de importar vptr, slot ou layout de objeto de outra versao. O envio
converge no mesmo transporte global por `FUN_0055F2DD`.

### Ownership

No client, a FieldScene possui os controles; o estado visual global existe
somente durante o fluxo da cena e o construtor recompilavel o reinicializa em
zero. No servidor, cada `Player` possui seu `PKMode` efemero; o client somente
envia a intencao de alternancia.

### Falha parcial

Controle moderno ou legado ausente nao interrompe a alternancia nem o envio.
Packet com tamanho ou valor invalido e rejeitado antes de mutar `Player.PKMode`.
Alvo invalido ou PvP desligado nao consome relogio, MP, cooldown ou Hide e nao
deixa dano, perseguição de summon ou trade parcialmente alterado.

### Cleanup e teardown

O teardown da FieldScene descarta seus controles opcionais sem manter ponteiro
para o proximo root. No servidor, `removePlayerFromWorld` desfaz combate,
summons e indices; a reinicializacao da sessao limpa alvo, relogios, cooldowns
e `PKMode`.

### Shutdown

Fechamento global e desconexao convergem em `removePlayerFromWorld`; nenhum
estado de PK e persistido depois da remocao do runtime do personagem.

### Logout e relogin

Logout descarta `Player.PKMode`. O novo runtime criado no relogin inicia com
`PKMode=false`, portanto o jogador precisa reativar o modo explicitamente. A
source client tambem reinicializa `m_bPK` no construtor da nova FieldScene.

## Wire, ABI e recursos

O buffer e zerado antes do preenchimento e enviado por `FUN_0055F2DD`:

| Campo | Valor |
| --- | --- |
| direcao | client para servidor |
| opcode | `0x399` |
| tamanho total | 16 bytes |
| identidade | header legado, copiado do personagem local |
| estado | `uint32` little-endian no offset `+12`, somente `0` ou `1` |

O WYD-Go valida tamanho e dominio, grava `Player.PKMode`, publica `PKInfo`
`0x166` e devolve uma mensagem de estado. Valores acima de `1` sao rejeitados
como violacao de contrato. O controle nativo e o root `0x132`/`306`; o controle
moderno `65786` permanece opcional e nao altera o wire quando ausente.

## Mapeamento atual

### Source recompilável

- `TMFieldScene::SetPK` preserva o toggle e o packet nativos.
- `m_PkButton` moderno `65786` e o controle legado `306` sao opcionais e
  protegidos, sem impedir o envio de `0x399`.
- O construtor reinicializa `m_bPK`, preservando o reset entre cenas/sessoes.

### WYD-Go

- `World.onPKMode` valida e publica o estado recebido.
- `World.canInitiatePvP` centraliza a autorizacao server-authoritative.
- `World.onAttack`, `World.skillPlayerTargets` e `World.summonTarget` consultam
  o gate antes dos efeitos consumiveis, sem bloquear PvE.

### Relação com Chaos/C.Point

O guia coreano confirma que C.Point e penalidade associada a morte PvP e que
morte por mob nao o aumenta. Isso explica a proximidade funcional entre o
toggle PK e o estado de Chaos, mas nao transforma `MSG_UpdateEtc::Hold` em CP:
os contratos de Character e C.Point continuam separados. Kingdom War tambem
e descrita como excecao futura ao PK Mode tradicional, nao como permissao
geral para ignorar o toggle.

## Matriz de delta

| Claim | Nativo 7.48 | Source/client atual | WYD-Go anterior | Decisao |
| --- | --- | --- | --- | --- |
| alternancia | `FUN_0044ECAE`, estado `0/1` | preservada | validava e armazenava | manter paridade |
| wire | C->S `0x399`, 16 bytes, estado `+12` | preservado | contrato ja validado | manter contrato |
| controle | legado `306`; moderno nao exigido | ambos opcionais/protegidos | n/a | impedir crash sem inventar asset |
| ataque fisico PvP | depende do modo | intencao enviada | ignorava `PKMode` | aplicar gate autoritativo |
| skill PvP | depende do modo | intencao enviada | ignorava `PKMode` | aplicar antes de custo/efeitos |
| summon contra jogador | depende da ordem PvP do dono | n/a | ignorava `PKMode` | aplicar somente ao alvo jogador |
| PvE | independente do modo | inalterado | funcionava | preservar |

## Decisões

- Classificar o lote como `PARIDADE_NATIVA`.
- Manter o client como emissor de intencao e o WYD-Go como autoridade.
- Usar uma unica regra central nos tres caminhos de iniciacao PvP.
- Nao implementar Kingdom/Castle/Guild War sem estado e contrato proprios.
- Nao usar Hold como transporte de Chaos/C.Point.

## Lacunas

- Executar o fluxo no `client748/project.exe`: `K` sem crash, PvP bloqueado
  quando desligado, liberado quando ligado e PvE preservado.
- Confirmar em runtime o reset para desligado depois de logout/relogin.
- Kingdom, Castle e Guild War permanecem fora deste contrato ate possuirem
  estado e autorizacao server-side explicitos.

## Validação

- Ghidra: raiz, tres callers, callee de transporte, controle legado e layout
  completo do `0x399` confirmados.
- Pesquisa: `validate_research.py --repo .` aprovado; ficha reconhecida como
  `CONTRACT` e censo atual em `CONTRACT=15`, `TRACED=18`, `LOCATED=4` e
  `UNMAPPED=2`.
- Servidor: `go test -count=1 ./...` aprovado. Os testes cobrem
  bloqueio/liberacao de ataque fisico, skill e summon, nao consumo de
  relogio/MP/cooldown/Hide na rejeicao e preservacao de PvE.
- Client source: `Build-Client.ps1` aprovado em `Release|Win32`, com zero erros
  e 13 warnings `C4018`; candidato instalado em `client748/project.exe`,
  SHA-256 `6884415003707C8C8A0EE1BDF02BE296D70F65D268E4995C3E701D3B73F7457C`.
- Client real: pendente confirmar `K` sem crash, bloqueio desligado, permissao
  ligada, PvE desligado e reset depois de logout/relogin. Ate isso, nao alegar
  `CLIENT_TESTED`.
