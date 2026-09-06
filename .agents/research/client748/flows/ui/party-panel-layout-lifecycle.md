---
id: ui-party-panel-layout-lifecycle
title: Layout e lifecycle do painel Party 7.48
subsystem: ui-layout-party
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Layout e lifecycle do painel Party 7.48

## Pergunta

Como o client 7.48 materializa, posiciona, abre, alimenta, usa e encerra o
painel Party, e quais partes da source moderna podem permanecer sem substituir
os controles nativos?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256
  `8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado
  `exports/party-panel-layout-lifecycle-flow.tsv`.
- Assets: `UI\FieldScene.txt`/`UI\FieldScene2.txt`; root Party `1857`, lista
  `1863` e botão inferior `5742`.
- Source atual: `TMFieldScene::InitializeCompatFieldScene`,
  `PositionCompatPartyPanel`, `SetVisibleParty`, `OnControlEvent` e handlers
  Party em `client-source/tmproject/Projects/TMProject/TMFieldScene.cpp`.
- Servidor: `internal/wire/opcodes.go`, `internal/wire/codec.go` e
  `internal/game/party.go`.

## Fluxo nativo 7.48

### Entrada observável

- Evento: click no botão `5742`, tecla `P`, convite `0x37F`, inclusão
  `0x37D`, remoção `0x37E` ou uma UI concorrente que fecha Party.
- Precondições e estado inicial: FieldScene materializada, controles `1857`,
  `1863` e `5742` vinculados ao receptor da cena; packets exigem payload e
  lista válidos.
- Saída observável: painel na borda inferior esquerda, visibilidade e seleção
  do botão sincronizadas; lista atualizada com líder/membros; clicks na lista
  aceitam convite, selecionam expulsão ou miram uma skill de suporte.

### Callers

- `FUN_00435B13` é alcançada pelo slot `+0x4C` da vtable FieldScene e faz a
  inicialização/binding dos controles.
- `FUN_004662C5`, slot `+0x58`, trata os callbacks dos controles `5742` e
  `1863` e chama `FUN_0044DA6F` no branch Party.
- `FUN_00453B35` chama `FUN_0044DA6F` para a tecla `P`.
- `FUN_0044DF53` chama `FUN_0044DA6F` no fechamento por `Esc`.
- `FUN_0047FCD7` chama `FUN_0044DA6F` ao abrir UI concorrente.
- `FUN_004883BF` e `FUN_00488879` abrem Party por meio de
  `FUN_0044DA6F` após convite/inclusão.
- `FUN_00492E7D` despacha `0x37F`, `0x37D` e `0x37E` para
  `FUN_004883BF`, `FUN_00488879` e `FUN_00488B52`.

### Função principal

`FUN_00435B13` resolve o root `1857`, a lista `1863` e o botão `5742`. O root
e o botão recebem o mesmo anchor:

```text
x = 0
y = viewportHeight - partyPanel.height - 165
```

`FUN_0044DA6F` é o toggle central. Ao alternar o painel, mantém o botão com
seleção inversa à visibilidade: painel visível implica botão não selecionado;
painel oculto implica botão selecionado. A posição é função do viewport e da
altura real do root, não apenas da coordenada serializada no recurso.

### Callees

- `FUN_0044DA6F` usa `FUN_004091AB`, `FUN_0040981B`, `FUN_00429A6D` e
  `FUN_0042AD2B` para consultar/alterar estado dos controles e reproduzir o
  efeito do toggle.
- `FUN_004883BF` materializa o item de convite, atualiza chat/minimapa e chama
  `FUN_0044DA6F` quando precisa expor a lista.
- `FUN_00488879` substitui item duplicado, materializa o membro confirmado,
  atualiza o estado da entidade/minimapa e também pode chamar
  `FUN_0044DA6F`.
- `FUN_00488B52` remove um membro ou esvazia a lista e restaura os estados das
  entidades/minimapa.
- No callback da lista em `FUN_004662C5`, click esquerdo sobre convite envia a
  confirmação `0x3AB`; `Ctrl`+click sobre membro selecionável prepara a
  confirmação de remoção; click direito usa o membro como alvo de skill,
  respeitando mana, delay, alcance, terreno e pertença ao grupo.

### Saídas e erros

- Convite inválido ou lista ausente não deve criar item parcial nem
  desreferenciar controle ausente.
- Inclusão substitui uma entrada anterior do mesmo ID antes de adicionar a
  nova e só abre automaticamente o painel para IDs locais válidos.
- `0x37E` com `Parm == 0` encerra o grupo e esvazia a lista; `Parm != 0`
  remove apenas o membro indicado. Se resta somente uma pessoa, a lista é
  esvaziada e o estado local de Party é desligado.
- Fechar o painel altera somente UI. A composição do grupo só muda por ação ou
  packet próprio; esconder o root não apaga membros.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| bootstrap FieldScene | container e recursos carregados | `FUN_00435B13` | root/lista/botão vinculados; painel oculto | aplica anchor e seleção inversa | controle ausente fica sem binding útil |
| click `5742` ou tecla `P` | root Party válido | `FUN_004662C5`/`FUN_00453B35` → `FUN_0044DA6F` | visibilidade alternada | sincroniza botão | sem root, não há transição |
| convite `0x37F` | payload/lista válidos | `FUN_00492E7D` → `FUN_004883BF` | lista contém convite e painel visível | chat, som e minimapa | falha parcial não deve tocar AutoParty ausente |
| click esquerdo no convite | item em estado de convite | `FUN_004662C5` | confirmação `0x3AB` enviada | marca entidade relacionada | item inválido: nenhuma ação |
| inclusão `0x37D` | payload/lista válidos | `FUN_00492E7D` → `FUN_00488879` | membro inserido/atualizado | Party/minimapa atualizados | duplicata anterior é removida primeiro |
| `Ctrl`+click em membro | membro selecionável | `FUN_004662C5` | modal de remoção preparado | guarda ID do membro | ID fora da faixa: nenhuma ação |
| click direito em membro | skill de suporte válida | `FUN_004662C5` | ataque/skill enviado ao alvo | cooldown e feedback local | bloqueia por mana, delay, alcance ou terreno |
| remoção `0x37E` | payload/lista válidos | `FUN_00492E7D` → `FUN_00488B52` | membro removido ou grupo limpo | flags e minimapa restaurados | lista ausente: nenhuma mutação |
| `Esc` ou UI concorrente | Party visível | `FUN_0044DF53`/`FUN_0047FCD7` → `FUN_0044DA6F` | painel oculto | botão volta a selecionado | membros permanecem |
| teardown da cena | FieldScene deixa de ser ativa | deleting destructor → `FUN_004358DA` | controles e itens deixam a cena | container encerra ownership | nenhum ponteiro Party pode sobreviver à cena |

### Vtables, vptrs e receptores

A vtable FieldScene é `PTR_FUN_005A4294`. O initializer em `+0x4C` aponta para
`FUN_00435B13`, o receptor de controles em `+0x58` para `FUN_004662C5` e o
receptor de packets em `+0x04` converge no dispatcher `FUN_00492E7D`. Os três
fluxos operam sobre o mesmo objeto FieldScene e seus membros Party; não são
singletons independentes.

### Ownership

O `SControlContainer` é dono dos controles carregados do recurso. A FieldScene
mantém ponteiros emprestados para root, lista e botão. A lista é dona dos
`SListBoxPartyItem` nela inseridos e `DeleteItem`/`Empty` encerram esses itens.
Nenhum dos helpers de visibilidade adquire ownership.

### Falha parcial

O layout 7.48 não materializa os controles AutoParty modernos. Na source
compatível, root, lista, botão, chat e payload são protegidos individualmente;
`m_pPartyAutoButton`, `m_pPartyAutoText` e o root moderno `7602196` só são
acessados fora do modo 7.48. A falta desses controles não pode impedir o
convite, a inclusão, a remoção ou o fechamento nativos.

### Cleanup e teardown

`FUN_00488B52` executa o cleanup observável de membros e marcadores quando o
servidor remove alguém ou encerra o grupo. O fechamento visual centralizado
em `FUN_0044DA6F` não faz cleanup de membership. No fim da cena, o deleting
destructor converge em `FUN_004358DA`, que encerra o grafo de controles sob o
owner da FieldScene.

### Shutdown

O shutdown global destrói a cena pelo lifecycle comum; Party não possui thread,
timer ou owner global separado. Itens e ponteiros da lista não devem ser
reutilizados depois que a FieldScene entra em teardown.

### Logout e relogin

Logout/troca de personagem destrói a FieldScene atual. No retorno ao mundo,
`FUN_00435B13` refaz bindings, posição, visibilidade inicial e seleção do
botão; a lista é reconstruída pelos packets do servidor. Não há restauração de
ponteiros ou itens da instância anterior. O fluxo ainda precisa ser exercitado
no `project.exe` antes de receber `CLIENT-TESTED`.

## Wire, ABI e recursos

Esta adaptação não altera wire, packing, structs nem opcodes. O dispatcher
nativo usa `0x37F` para convite, `0x37D` para inclusão e `0x37E` para
remoção; a confirmação aceita pelo callback usa `0x3AB`. Os mesmos opcodes já
existem em `internal/wire/opcodes.go` e são tratados pelo servidor. Os IDs
ativos do layout 7.48 são `1857/1863/5742`; os controles modernos
`475136/475138/65799` não substituem esses bindings no modo compatível.

Para o corte de wire, `FUN_0055890A` aceita `0x37D` somente com 40 bytes. O
payload é `PARTY` de 28 bytes em `+12`, com `ID` em `+20`, `Name` em `+22` e
WORD reservado em `+38`. `PartyAddPacket.h` fixa esses offsets e o gate local
valida o frame antes de `OnPacketAddParty`. O mesmo gate exige 16 bytes para
`0x37E`; `PartyRemovePacket.h` fixa `Parm` em `+12`, onde zero limpa o grupo e
valor diferente de zero identifica o membro. `PartyRequestPacket.h` agora fixa
`0x37F/44`, reutilizando
`PARTY` em `+12` e posicionando `TargetID` int32 em `+40`; o gate precede
`OnPacketREQParty` sem alterar a criação do convite. `0x3AB` permanece em
contrato próprio.

## Mapeamento atual

### Source recompilável

`InitializeCompatFieldScene` vincula `1857/1863/5742` e posiciona o painel.
`PositionCompatPartyPanel` reaplica o anchor nativo no bootstrap e antes de
abrir. `SetVisibleParty` centraliza a visibilidade e a seleção inversa do botão;
botão moderno `65799`, botão nativo `5742`, tecla `P`, packets, `Esc` e UI
concorrente convergem nesse helper. O callback aceita tanto a lista moderna
`475138` quanto a nativa `1863`, e os handlers protegem os controles opcionais.

### WYD-Go

`internal/wire/opcodes.go` declara PartyAdd `0x37D`, PartyRemove `0x37E`,
PartyRequest `0x37F` e PartyAccept `0x3AB`. `internal/wire/codec.go` decodifica
o contrato e `internal/game/party.go` mantém a autoridade sobre convite,
aceite, composição e remoção. O client apenas apresenta o snapshot e envia
intenções.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| controles Party | `1857/1863/5742` | vinculados no compat | usava também IDs 7.59 | N/A | `PARIDADE_NATIVA` |
| posição | `x=0`, `y=H-h-165` | reaplicada no bootstrap/open | dependia de root moderno/posição residual | N/A | `PARIDADE_NATIVA` |
| toggle e fechamento | `FUN_0044DA6F` central | `SetVisibleParty` central | caminhos diretos divergiam | N/A | `PARIDADE_NATIVA` |
| callbacks da lista | lista `1863` | aceita `1863` e `475138` | somente `475138` | revalida intenções | `PARIDADE_NATIVA` |
| packet Party Add | `0x37D/40B` | `PartyAddPacket.h` | compatível | implementado | `PARIDADE_NATIVA/CONTRACT` |
| packet Party Remove | `0x37E/16B` | `PartyRemovePacket.h` | compatível | implementado | `PARIDADE_NATIVA/CONTRACT` |
| packet Party Request | `0x37F/44B` | `PartyRequestPacket.h` | compatível | implementado | `PARIDADE_NATIVA/CONTRACT` |
| confirmação Party | `0x3AB` | preservada | compatível | implementada | corte separado |
| AutoParty moderno | fora deste claim nativo | opcional e isolado fora do compat | controles posteriores | política server-side existente | preservar como extensão separada |

## Decisões

- Usar os três controles efetivamente materializados no 7.48.
- Reaplicar o anchor no open para eliminar posição residual sem alterar o
  recurso nem o wire.
- Centralizar toda alteração de visibilidade e estado do botão.
- Preservar AutoParty moderno somente onde seus controles existem; não criar
  widgets artificiais para esconder ponteiros nulos.
- Manter a autoridade de Party no servidor.

## Lacunas

- Testar em jogo botão `5742`, botão moderno quando aplicável, tecla `P`,
  convite/aceite/remoção e callbacks esquerdo, `Ctrl`+click e direito.
- Testar fechamento por `Esc`, UI concorrente, troca de cena e logout/relogin.
- Comparar a posição em `800x600`, `1024x768` e `1280x960`.
- Não promover a `CLIENT-TESTED` antes desses fluxos.

## Validação

- Pesquisa: callers, callees, dispatcher, vtable, ownership, falha parcial,
  cleanup, teardown, shutdown e relogin foram registrados no export focado e
  nesta ficha; `FUN_0055890A` fixa `0x37D/40`.
- ABI: `PartyAddPacket.h` fixa 40 bytes, `PARTY` em `+12`, ID em `+20`, nome em
  `+22` e reserved em `+38`; `PartyRemovePacket.h` fixa 16 bytes e Parm em
  `+12`; `PartyRequestPacket.h` fixa 44 bytes e TargetID em `+40`. As fixtures
  cobrem framing e preservação do buffer.
- Entrega: `IMPLEMENTED / STATICALLY VERIFIED`; `Build-Client.ps1` concluiu
  Debug e Release com 1892 checks/asserts, XML/caminhos e `git diff --check`
  aprovados. O candidato Release instalado possui SHA-256
  `65CF0A3A2958238963F5FA0750CA75098BC016AA8E07D6289FE16BC92FABDD37`.
- Automação: `validate_research.py`, a fixture C++ e `git diff --check` passaram
  para o lote.
- Client real: não executado; `CLIENT-TESTED` não é alegado.
